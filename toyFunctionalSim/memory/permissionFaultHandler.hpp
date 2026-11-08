#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <setjmp.h>
#include <signal.h>

#include "memory.hpp"

namespace simulator {

struct CodeWriteFault : std::exception {
    Addr writeGuestAddress;
    std::size_t byteCount;
    Addr faultGuestAddress;

    CodeWriteFault(Addr writeGuestAddress, std::size_t byteCount, Addr faultGuestAddress) noexcept
        : writeGuestAddress(writeGuestAddress), byteCount(byteCount), faultGuestAddress(faultGuestAddress) {}

    const char* what() const noexcept override { return "Write to protected guest code"; }
};

class PermissionFaultHandler {
public:
    explicit PermissionFaultHandler(Memory& guestMemory);
    ~PermissionFaultHandler();

    PermissionFaultHandler(const PermissionFaultHandler&) = delete;
    PermissionFaultHandler& operator=(const PermissionFaultHandler&) = delete;
    PermissionFaultHandler(PermissionFaultHandler&&) = delete;
    PermissionFaultHandler& operator=(PermissionFaultHandler&&) = delete;

private:
    friend struct Memory;

    struct WriteContext {
        sigjmp_buf jumpBuffer;
        std::uintptr_t hostWriteBegin;
        std::uintptr_t hostMemoryBase;
        std::size_t byteCount;
        volatile sig_atomic_t faultGuestAddress;
    };

    template<class WriteOperation>
    static void guardWrite(Memory& guestMemory, Addr writeGuestAddress,
                           std::size_t byteCount, WriteOperation writeOperation) {
        if (!activeHandler_ || activeHandler_->guestMemory_ != &guestMemory ||
            !guestMemory.isWriteProtected(writeGuestAddress, byteCount)) {
            writeOperation();
            return;
        }

        WriteContext writeContext{};
        writeContext.hostMemoryBase = reinterpret_cast<std::uintptr_t>(guestMemory.data());
        writeContext.hostWriteBegin = writeContext.hostMemoryBase + writeGuestAddress;
        writeContext.byteCount = byteCount;
        auto* previousWriteContext = activeWriteContext_;

        if (sigsetjmp(writeContext.jumpBuffer, 1) != 0) {
            activeWriteContext_ = previousWriteContext;
            throw CodeWriteFault(writeGuestAddress, byteCount, static_cast<Addr>(writeContext.faultGuestAddress));
        }

        activeWriteContext_ = &writeContext;
        writeOperation();
        activeWriteContext_ = previousWriteContext;
    }

    static void handleSignal(int signalNumber, siginfo_t* signalInfo, void* hostSignalContext) noexcept;

    Memory* guestMemory_;

    static struct sigaction previousSignalAction_;
    static volatile sig_atomic_t previousSignalActionConsumed_;
    static PermissionFaultHandler* activeHandler_;
    static WriteContext* activeWriteContext_;
};

}
