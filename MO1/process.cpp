#include "process.h"
#include <random>
#include <algorithm>
#include <ctime>

std::string currentTimestamp() {
    time_t t = time(nullptr);
    tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[64];
    strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", &tmv);
    return buf;
}

static std::mt19937_64& rng() {
    static std::mt19937_64 gen{std::random_device{}()};
    return gen;
}
static int64_t randomInt(int64_t low, int64_t high) {
    return std::uniform_int_distribution<int64_t>(low, high)(rng());
}

static const char* variable_pool[] = {"x", "y", "z", "a", "b", "c"};
static std::string randomVar() { return variable_pool[randomInt(0, 5)]; }

static Operand randomOperand() {
    Operand op;
    if (randomInt(0, 1)) { op.isVariable = true; op.name = randomVar(); }
    else                 { op.val = static_cast<uint16_t>(randomInt(0, 100)); }
    return op;
}

static Instruction randomInstruction() {
    Instruction i;
    int r = static_cast<int>(randomInt(0, 9));
    if (r <= 2) {
        i.type = InstructionTypes::PRINT;      
    } else if (r <= 4) {
        i.type = InstructionTypes::ADD;
        i.var1 = randomVar(); i.x = randomOperand(); i.y = randomOperand();
    } else if (r <= 6) {
        i.type = InstructionTypes::DECLARE;
        i.var1 = randomVar(); i.x.val = static_cast<uint16_t>(randomInt(0, 100));
    } else if (r <= 8) {
        i.type = InstructionTypes::SUBTRACT;
        i.var1 = randomVar(); i.x = randomOperand(); i.y = randomOperand();
    } else {
        i.type = InstructionTypes::SLEEP;
        i.ticks = static_cast<uint8_t>(randomInt(1, 5));
    }
    return i;
}

static std::vector<Instruction> genBlock(int64_t budget, int depth) {
    std::vector<Instruction> out;
    while (budget > 0) {
        if (depth < 3 && budget >= 2 && randomInt(0, 2) == 0) {
            int64_t rmax = budget > 1000 ? 1000 : 10;
            int64_t repeats = randomInt(2, std::min<int64_t>(budget, rmax));
            int64_t bodyBudget = randomInt(1, budget / repeats);
            Instruction f;
            f.type = InstructionTypes::FOR;
            f.repeats = static_cast<uint32_t>(repeats);
            f.body = genBlock(bodyBudget, depth + 1);
            out.push_back(std::move(f));
            budget -= repeats * bodyBudget;
        } else {
            out.push_back(randomInstruction());
            --budget;
        }
    }
    return out;
}

static uint64_t countFlat(const std::vector<Instruction>& v) {
    uint64_t n = 0;
    for (const auto& in : v)
        n += (in.type == InstructionTypes::FOR) ? countFlat(in.body) * in.repeats : 1;
    return n;
}

Process generateProcess(int id, const std::string& name, const Config& cfg) {
    Process p;
    p.uniqueId = id;
    p.name = name;
    p.createdAt = currentTimestamp();
}

static uint16_t resolve(Process& p, const Operand& op) {
    if (!op.isVariable) return op.val;
    return p.variables[op.name];            
}
static uint16_t clamp16(int32_t v) {
    return static_cast<uint16_t>(std::max<int32_t>(0, std::min<int32_t>(65535, v)));
}

static const Instruction* nextSimple(Process& p) {
    if (!p.started) {
        p.stack.push_back(Frame{p.program.get(), 0, 1});
        p.started = true;
    }
    while (!p.stack.empty()) {
        Frame& f = p.stack.back();
        if (f.idx >= f.body->size()) {
            if (f.itersLeft > 1) { --f.itersLeft; f.idx = 0; }
            else p.stack.pop_back();
            continue;
        }
        const Instruction& in = (*f.body)[f.idx];
        ++f.idx;                                          
        if (in.type == InstructionTypes::FOR) {
            if (in.repeats > 0 && !in.body.empty())
                p.stack.push_back(Frame{&in.body, 0, in.repeats});
            continue;
        }
        return &in;
    }
    return nullptr;
}

bool exeNext(Process& p, int coreId) {
    if (p.isFinished()) return false;
    const Instruction* in = nextSimple(p);
    if (!in) { p.pState = ProcessStates::FINISHED; return false; }

    switch (in->type) {
        case InstructionTypes::PRINT: {
            std::string msg = in->printValue.empty()
                ? "Hello world from " + p.name + "!"
                : "Value from: " + std::to_string(p.variables[in->printValue]);
            p.logs.push_back("(" + currentTimestamp() + ") Core:" + std::to_string(coreId) +
                             " \"" + msg + "\"");
            break;
        }
        case InstructionTypes::DECLARE:
            p.variables[in->var1] = in->x.val;
            break;
        case InstructionTypes::ADD:
            p.variables[in->var1] = clamp16(int32_t(resolve(p, in->x)) + int32_t(resolve(p, in->y)));
            break;
        case InstructionTypes::SUBTRACT:
            p.variables[in->var1] = clamp16(int32_t(resolve(p, in->x)) - int32_t(resolve(p, in->y)));
            break;
        case InstructionTypes::SLEEP:
            p.sleepRemaining = in->ticks;
            p.pState = ProcessStates::SLEEPING;      
            break;
        case InstructionTypes::FOR:
            break;                                    
    }

    ++p.executed;
    if (p.executed >= p.totalInstructions && p.pState != ProcessStates::SLEEPING)
        p.pState = ProcessStates::FINISHED;
    return true;
}

bool tickSleep(Process& p) {
    if (p.pState != ProcessStates::SLEEPING) return true;
    if (p.sleepRemaining > 0) --p.sleepRemaining;
    if (p.sleepRemaining > 0) return false;
    p.pState = (p.executed >= p.totalInstructions) ? ProcessStates::FINISHED : ProcessStates::READY;
    return true;
}
