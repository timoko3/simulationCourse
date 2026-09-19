#include <vector> 

#include "isa/definitions.hpp"
#include "generalFunctions/file.h"

using namespace simulator;

constexpr char* PROGRAM_FILE_NAME = "program.bin";

int main(){
    std::vector<Word> program = generalFunctions::readBinaryFile<Word>(PROGRAM_FILE_NAME);
    
}