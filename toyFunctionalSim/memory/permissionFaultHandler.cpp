#include "permissionFaultHandler.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <ucontext.h>

namespace simulator {

struct sigaction PermissionFaultHandler::previousSignalAction_{};
volatile sig_atomic_t PermissionFaultHandler::previousSignalActionConsumed_ = 0;
PermissionFaultHandler* PermissionFaultHandler::activeHandler_ = nullptr;

PermissionFaultHandler::PermissionFaultHandler(Memory& guestMemory)
    : hostMemoryBase_(reinterpret_cast<std::uintptr_t>(guestMemory.data())), memorySize_(guestMemory.size())
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
    const auto savedErrno = errno;
    if (auto* handler = activeHandler_) {
        handler->tryJumpFromGuestFault(signalInfo, hostSignalContext, savedErrno);
    }

    forwardSignal(signalNumber, signalInfo, hostSignalContext);
    errno = savedErrno;
}

void PermissionFaultHandler::tryJumpFromGuestFault(const siginfo_t* signalInfo, const void* hostSignalContext,
                                                   int savedErrno) noexcept
{
    if (!executionActive_ || !signalInfo || !hostSignalContext || signalInfo->si_code != SEGV_ACCERR) {
        return;
    }

    const auto hostFaultAddress = reinterpret_cast<std::uintptr_t>(signalInfo->si_addr);
    if (hostFaultAddress < hostMemoryBase_ || hostFaultAddress - hostMemoryBase_ >= memorySize_) {
        return;
    }

    const auto* signalContext = static_cast<const ucontext_t*>(hostSignalContext);
    if (sigprocmask(SIG_SETMASK, &signalContext->uc_sigmask, nullptr) != 0) {
        return;
    }

    faultGuestAddress_ = static_cast<sig_atomic_t>(hostFaultAddress - hostMemoryBase_);
    errno = savedErrno;
    siglongjmp(jumpBuffer_, 1);
}

void PermissionFaultHandler::forwardSignal(int signalNumber, siginfo_t* signalInfo, void* hostSignalContext) noexcept
{
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
}

}
