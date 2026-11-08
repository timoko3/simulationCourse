#include "permissionFaultHandler.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>

namespace simulator {

struct sigaction PermissionFaultHandler::previousSignalAction_{};
volatile sig_atomic_t PermissionFaultHandler::previousSignalActionConsumed_ = 0;
PermissionFaultHandler* PermissionFaultHandler::activeHandler_ = nullptr;
PermissionFaultHandler::WriteContext* PermissionFaultHandler::activeWriteContext_ = nullptr;

PermissionFaultHandler::PermissionFaultHandler(Memory& guestMemory)
    : guestMemory_(&guestMemory)
{
    if (activeHandler_) {
        throw std::logic_error("Permission fault handling is already active");
    }

    if (sigaction(SIGSEGV, nullptr, &previousSignalAction_) != 0) {
        throw std::system_error(errno, std::generic_category(), "Cannot read SIGSEGV handler");
    }
    previousSignalActionConsumed_ = 0;

    struct sigaction signalAction{};
    signalAction.sa_sigaction = &handleSignal;
    signalAction.sa_mask = previousSignalAction_.sa_mask;
    signalAction.sa_flags = SA_SIGINFO | (previousSignalAction_.sa_flags & (SA_ONSTACK | SA_RESTART | SA_NODEFER));
    if (sigaction(SIGSEGV, &signalAction, nullptr) != 0) {
        throw std::system_error(errno, std::generic_category(), "Cannot install SIGSEGV handler");
    }

    activeHandler_ = this;
}

PermissionFaultHandler::~PermissionFaultHandler()
{
    if (previousSignalActionConsumed_) {
        struct sigaction defaultSignalAction{};
        defaultSignalAction.sa_handler = SIG_DFL;
        sigemptyset(&defaultSignalAction.sa_mask);
        sigaction(SIGSEGV, &defaultSignalAction, nullptr);
    }
    else {
        sigaction(SIGSEGV, &previousSignalAction_, nullptr);
    }
    activeHandler_ = nullptr;
}

void PermissionFaultHandler::handleSignal(int signalNumber, siginfo_t* signalInfo, void* hostSignalContext) noexcept
{
    auto* writeContext = activeWriteContext_;
    if (writeContext && signalInfo && signalInfo->si_code == SEGV_ACCERR) {
        const auto hostFaultAddress = reinterpret_cast<std::uintptr_t>(signalInfo->si_addr);
        if (hostFaultAddress >= writeContext->hostWriteBegin &&
            hostFaultAddress - writeContext->hostWriteBegin < writeContext->byteCount) {
            writeContext->faultGuestAddress = static_cast<sig_atomic_t>(hostFaultAddress - writeContext->hostMemoryBase);
            siglongjmp(writeContext->jumpBuffer, 1);
        }
    }

    const auto savedErrno = errno;
    if (previousSignalActionConsumed_ || previousSignalAction_.sa_handler == SIG_DFL ||
        (previousSignalAction_.sa_handler == SIG_IGN && signalInfo && signalInfo->si_code > 0)) {
        struct sigaction defaultSignalAction{};
        defaultSignalAction.sa_handler = SIG_DFL;
        sigemptyset(&defaultSignalAction.sa_mask);
        sigaction(signalNumber, &defaultSignalAction, nullptr);
        raise(signalNumber);
    }
    else if (previousSignalAction_.sa_handler != SIG_IGN) {
        if ((previousSignalAction_.sa_flags & SA_RESETHAND) != 0) {
            previousSignalActionConsumed_ = 1;
        }

        if ((previousSignalAction_.sa_flags & SA_SIGINFO) != 0) {
            previousSignalAction_.sa_sigaction(signalNumber, signalInfo, hostSignalContext);
        }
        else {
            previousSignalAction_.sa_handler(signalNumber);
        }
    }
    errno = savedErrno;
}

}
