#include "simplePipeline.hpp"
#include "cpu.hpp"

namespace simulator{

void
SimplePipeline::runSimplePipeline(Cpu& cpu){

    while(true){
        Word rawBytesInstr = fetch_.instrFetch(cpu);

    }
    
}

}