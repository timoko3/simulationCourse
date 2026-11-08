#include "smcController.hpp"

#include <stdexcept>

#include "cpu.hpp"
#include "dbtExecutor.hpp"
#include "memory/permissionFaultHandler.hpp"

namespace simulator {

void SmcController::handleSMC(const CodeWriteFault& fault)
{
    auto& state = cpu_.getState();
    auto& pipeline = cpu_.getSimplePipeline();
    const Word raw = pipeline.fetch_.instrFetch(memory_, state.pc);
    const Instruction instruction = pipeline.decode_.instrDecode(raw);
    if (instruction.instrKind != InstrKind::I_ST) {
        throw std::runtime_error("Permission fault outside a guest store");
    }

    const Addr writeAddress = state.getReg(instruction.src1) + instruction.dst;
    if (fault.faultGuestAddress < writeAddress || fault.faultGuestAddress - writeAddress >= sizeof(Word) ||
        !memory_.isWriteProtected(fault.faultGuestAddress, 1)) {
        throw std::runtime_error("Permission fault does not match the guest store");
    }

    if (dbtExecutor_) {
        dbtExecutor_->invalidateCode();
    }
    cpu_.invalidateCode();
    memory_.unprotectCode(writeAddress, sizeof(Word));

    try {
        cpu_.executeInstruction(memory_, instruction);
    }
    catch (...) {
        memory_.protectCode(writeAddress, sizeof(Word));
        throw;
    }

    memory_.protectCode(writeAddress, sizeof(Word));
}

}
