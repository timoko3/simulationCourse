#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <setjmp.h>
#include <signal.h>

#include "memory.hpp"

namespace simulator {

struct CodeWriteFault : std::exception {
    Addr faultGuestAddress;

    explicit CodeWriteFault(Addr faultGuestAddress) noexcept : faultGuestAddress(faultGuestAddress) {}

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

    template<class Execution>
    auto execute(Execution execution) {
        if (executionActive_) {
            throw std::logic_error("Permission fault checkpoint is already active");
        }

        if (sigsetjmp(jumpBuffer_, 0) != 0) {
            executionActive_ = 0;
            throw CodeWriteFault(static_cast<Addr>(faultGuestAddress_));
        }

        executionActive_ = 1;
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<Execution>>) {
                execution();
                executionActive_ = 0;
            }
            else {
                auto result = execution();
                executionActive_ = 0;
                return result;
            }
        }
        catch (...) {
            executionActive_ = 0;
            throw;
        }
    }

private:
    static void handleSignal(int signalNumber, siginfo_t* signalInfo, void* hostSignalContext) noexcept;

    std::uintptr_t hostMemoryBase_;
    std::size_t memorySize_;
    sigjmp_buf jumpBuffer_;
    volatile sig_atomic_t executionActive_ = 0;
    volatile sig_atomic_t faultGuestAddress_ = 0;

    static struct sigaction previousSignalAction_;
    static volatile sig_atomic_t previousSignalActionConsumed_;
    static PermissionFaultHandler* activeHandler_;
};

}
