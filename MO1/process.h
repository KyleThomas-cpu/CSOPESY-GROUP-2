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
    std::string var1;                 // target variable (DECLARE / ADD / SUBTRACT)
    Operand x, y;                     // operands (DECLARE uses x.val as its value)
    uint8_t ticks = 0;                // SLEEP duration in CPU ticks
    std::string printValue;           // PRINT: optional variable appended to the message
    std::vector<Instruction> body;    // FOR: the instructions to repeat (may contain nested FORs)
    uint32_t repeats = 0;             // FOR: number of repetitions
};

// One level of FOR nesting currently being executed.
struct Frame {
    const std::vector<Instruction>* body = nullptr;
    size_t idx = 0;
    uint32_t itersLeft = 1;
};

struct Process {
    int uniqueId = 0;
    std::string name;

    std::shared_ptr<const std::vector<Instruction>> program;  // shared so snapshots are cheap
    std::vector<Frame> stack;                                 // FOR execution stack
    bool started = false;

    uint64_t totalInstructions = 0;   // flattened count (FOR bodies counted per repetition)
    uint64_t executed = 0;            // instructions executed so far ("current instruction line")

    ProcessStates pState = ProcessStates::READY;
    std::vector<std::string> logs;
    std::map<std::string, uint16_t> variables;
    int core = -1;
    uint32_t sleepRemaining = 0;      // ticks left while SLEEPING
    uint32_t waitRemaining = 0;       // delay-per-exec busy-wait cycles left
    std::string createdAt;

    bool isFinished() const { return pState == ProcessStates::FINISHED; }
};

std::string currentTimestamp();
Process generateProcess(int id, const std::string& name, const Config& config);

// Executes exactly one instruction on `coreId`. May leave the process SLEEPING or FINISHED.
bool exeNext(Process& p, int coreId);

// Advances a sleeping process by one tick. Returns true once it is no longer sleeping.
bool tickSleep(Process& p);
