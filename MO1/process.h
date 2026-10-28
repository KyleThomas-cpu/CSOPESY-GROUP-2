#pragma once
#include "config.h"
#include <string>
#include <vector>
#include <map>

enum class InstructionTypes {PRINT, DECLARE, ADD, SUBTRACT, SLEEP};
enum class ProcessStates {READY, RUNNING, SLEEPING, FINISHED};

struct Operand {
    bool isVariable = false;
    std::string name;
    uint16_t val = 0;
};

struct Instruction{
    InstructionTypes type;
    std::string var1;
    Operand x, y;
    uint16_t ticks = 0;
    std::string printValue;
};

struct Process{
    int uniqueId = 0;
    std::string name;
    std::vector<Instruction> instructions;
    size_t pc = 0;
    ProcessStates pState = ProcessStates::READY;
    std::vector<std::string> logs;
    std::map<std::string, uint16_t> variables;
    int core = -1;
    uint32_t sleepRemaining = 0;
    std::string createdAt;

    bool isFinished() const {return pState == ProcessStates::FINISHED;}
};

Process generateProcess(int id, const std::string& name, const Config& config);

void exeNext(Process& p, int coreId);

void tickSleep(Process& p);