#pragma once
#include "config.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

enum class InstructionTypes { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR };
enum class ProcessStates { READY, RUNNING, SLEEPING, FINISHED };

struct Operand {
    bool isVariable = false;
    std::string name;
    uint16_t val = 0;
};

struct Instruction {
    InstructionTypes type = InstructionTypes::PRINT;
    std::string var1;                 
    Operand x, y;                     
    uint8_t ticks = 0;                
    std::string printValue;           
    std::vector<Instruction> body;    
    uint32_t repeats = 0;             
};

struct Frame {
    const std::vector<Instruction>* body = nullptr;
    size_t idx = 0;
    uint32_t itersLeft = 1;
};

struct Process {
    int uniqueId = 0;
    std::string name;

    std::shared_ptr<const std::vector<Instruction>> program;  
    std::vector<Frame> stack;                                 
    bool started = false;

    uint64_t totalInstructions = 0;   
    uint64_t executed = 0;            

    ProcessStates pState = ProcessStates::READY;
    std::vector<std::string> logs;
    std::map<std::string, uint16_t> variables;
    int core = -1;
    uint32_t sleepRemaining = 0;      
    uint32_t waitRemaining = 0;       
    std::string createdAt;

    bool isFinished() const { return pState == ProcessStates::FINISHED; }
};

std::string currentTimestamp();
Process generateProcess(int id, const std::string& name, const Config& config);

bool exeNext(Process& p, int coreId);

bool tickSleep(Process& p);
