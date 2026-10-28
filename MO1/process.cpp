#include "process.h"
#include <random>
#include <algorithm>
#include <ctime>

static std::mt19937& rng(){
    static std::mt19937 gen {std::random_device{}()};
    return gen;
}
static std::string timestamp() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", now);
    return buf;
}
static int randomInt(int low, int high){return std::uniform_int_distribution<int>(low,high)(rng());}
static const char* variable_pool[] = {"x", "y", "z", "a", "b", "c"};

static std::string randomVar(){return variable_pool[randomInt(0,5)];}
static Operand randomOperand(){
    Operand op;
    if(randomInt(0,1)){op.isVariable = true; op.name = randomVar();}
    else{op.val = static_cast<uint16_t>(randomInt(0,100));}
    return op;
}
static Instruction randomInstruction(){
    Instruction i;
    switch(randomInt(0,4)){
        case 0:
            i.type = InstructionTypes::PRINT;
            
            break;
        case 1:
            i.type = InstructionTypes::ADD;
            i.var1 = randomVar();
            i.x = randomOperand();
            i.y = randomOperand();
            break;
        case 2:
            i.type = InstructionTypes::DECLARE;
            i.var1 = randomVar();
            i.x.val = static_cast<uint16_t>(randomInt(0,100));
            break;
        case 3:
            i.type = InstructionTypes::SUBTRACT;
            i.var1 = randomVar();
            i.x = randomOperand();
            i.y = randomOperand();
            break;
        default:
            i.type = InstructionTypes::SLEEP;
            i.ticks = static_cast<uint8_t>(randomInt(1,5));
            break;
    }
    return i;
}

static std::vector<Instruction> randomBlock(int depth){
    std::vector<Instruction> out;
    int count = randomInt(1,3);
    for(int i = 0; i < count; ++i){
        if(depth < 3 && randomInt(0,3) == 0){
            std::vector<Instruction> body = randomBlock(depth + 1);
            int repeats = randomInt(2,3);
            for(int r = 0; r < repeats; ++r){
                out.insert(out.end(), body.begin(), body.end());
            }
        }else{
            out.push_back(randomInstruction());
        }
    }
    return out;
}

Process generateProcess(int id, const std::string& name, const Config& cfg){
    Process p;
    p.uniqueId = id;
    p.name = name;
    p.createdAt = timestamp();

    // picks random instruction count for the process
    const uint32_t CAP = 1000000;
    int low = static_cast<int> (std::min(cfg.min_ins, CAP));
    int high = static_cast<int> (std::min(cfg.max_ins, CAP));
    size_t target = static_cast<size_t>(randomInt(low,high));

    while(p.instructions.size() < target){
        if(randomInt(0,5) == 0){
            std::vector<Instruction> block = randomBlock(1);
            int repeats = randomInt(2,3);
            for(int r = 0; r < repeats; ++r){
                for(const auto& in : block){
                    if(p.instructions.size() >= target) break;
                    p.instructions.push_back(in);
                }
            }
        }else{
            p.instructions.push_back(randomInstruction());
        }
    }
    return p;
}

static uint16_t resolve(Process& p, const Operand& op){
    if(!op.isVariable) return op.val;
    return p.variables[op.name];
}
static uint16_t clamp16(int32_t v){
    return static_cast<uint16_t>(std::max<int32_t>(0, std::min<int32_t>(65535,v)));
}

void exeNext(Process& p, int coreId){
    if(p.isFinished() || p.pc >= p.instructions.size()) {p.pState = ProcessStates::FINISHED; return;}

    const Instruction& in = p.instructions[p.pc];
    switch(in.type){
        case InstructionTypes::PRINT:
        {
            std::string msg = "Hello world from " + p.name + "!";
            if(!in.printValue.empty()){
                msg += " Value from: " + in.printValue + "=" + std::to_string(p.variables[in.printValue]); 
            }
            p.logs.push_back("(" + timestamp() + ") Core:" + std::to_string(coreId) + " \"" + msg + "\"");
            break;
        }
        
        case InstructionTypes::DECLARE:
            p.variables[in.var1] = in.x.val;
            break;
        case InstructionTypes::ADD:
            p.variables[in.var1] = clamp16(int32_t(resolve(p, in.x)) + int32_t(resolve(p, in.y)));
            break;
        case InstructionTypes::SUBTRACT:
            p.variables[in.var1] =  clamp16(int32_t(resolve(p, in.x)) - int32_t(resolve(p, in.y)));
            break;
        case InstructionTypes::SLEEP:
            p.sleepRemaining = in.ticks;
            p.pState = ProcessStates::SLEEPING;
            break;
    }  
    ++p.pc;
    if(p.pc >= p.instructions.size() && p.pState != ProcessStates::SLEEPING){
        p.pState = ProcessStates::FINISHED;
    }
}

void tickSleep(Process& p){
    if(p.pState != ProcessStates::SLEEPING) return;
    if(p.sleepRemaining > 0) --p.sleepRemaining;
    if(p.sleepRemaining == 0) p.pState = (p.pc >= p.instructions.size()) ? ProcessStates::FINISHED : ProcessStates::READY;
}

