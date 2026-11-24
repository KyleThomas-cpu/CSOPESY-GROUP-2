#include "process.h"
#include "memory.h"
#include <random>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <cctype>

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

static std::string hhmmss() {
    time_t t = time(nullptr);
    tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", &tmv);
    return buf;
}

static std::mt19937_64& rng() {
    static std::mt19937_64 gen{std::random_device{}()};
    return gen;
}
static int64_t randomInt(int64_t low, int64_t high) {
    return std::uniform_int_distribution<int64_t>(low, high)(rng());
}

static uint32_t randomAddress(uint32_t memSize){
    return static_cast<uint32_t>(randomInt(32, (memSize - 2) / 2)) * 2;
}

static uint32_t randomPower2(uint32_t lo, uint32_t hi){
    int a = 0, b = 0;
    while((1u << a) < lo) ++a;
    while((1u << b) < hi) ++b;
    return 1u << randomInt(a,b);
}
 
// Mem size for processes created by scheduler-start
uint32_t randomMemorySize(const Config& cfg){
    return randomPower2(cfg.minMemPerProc, cfg.maxMemPerProc);
}

static const char* variable_pool[] = {"x", "y", "z", "a", "b", "c"};
static std::string randomVar() { return variable_pool[randomInt(0, 5)]; }

static Operand randomOperand() {
    Operand op;
    if (randomInt(0, 1)) { op.isVariable = true; op.name = randomVar(); }
    else                 { op.val = static_cast<uint16_t>(randomInt(0, 100)); }
    return op;
}

static Instruction randomInstruction(uint32_t memSize) {
    Instruction i;
    bool canUseMem = memSize >= 68;
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
    }else if (r == 9){
        i.type = InstructionTypes::SLEEP;
        i.ticks = static_cast<uint8_t>(randomInt(1,5));
    }else if (r == 10 && canUseMem){
        i.type = InstructionTypes::READ;
        i.var1 = randomVar();
        i.address = randomAddress(memSize);
    }else if (r == 11 && canUseMem){
        i.type = InstructionTypes::WRITE;
        i.address = randomAddress(memSize);
        i.x = randomOperand();
    }
    else {
        i.type = InstructionTypes::PRINT;
    }
    return i;
}

static std::vector<Instruction> genBlock(int64_t budget, int depth, uint32_t memSize) {
    std::vector<Instruction> out;
    while (budget > 0) {
        if (depth < 3 && budget >= 2 && randomInt(0, 2) == 0) {
            int64_t rmax = budget > 1000 ? 1000 : 10;
            int64_t repeats = randomInt(2, std::min<int64_t>(budget, rmax));
            int64_t bodyBudget = randomInt(1, budget / repeats);
            Instruction f;
            f.type = InstructionTypes::FOR;
            f.repeats = static_cast<uint32_t>(repeats);
            f.body = genBlock(bodyBudget, depth + 1, memSize);
            out.push_back(std::move(f));
            budget -= repeats * bodyBudget;
        } else {
            out.push_back(randomInstruction(memSize));
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

static void setupMemory(Process& p, uint32_t memSize, uint32_t frameSize){
    p.memSize = memSize;
    p.pageTable.assign((memSize + frameSize - 1) / frameSize, -1);
}

Process buildProcess(int id, const std::string& name, uint32_t memSize, std::vector<Instruction> prog, const Config& cfg){
    Process p;
    p.uniqueId = id;
    p.name = name;
    p.createdAt = currentTimestamp();
    setupMemory(p,memSize,cfg.memPerFrame);
    p.totalInstructions = countFlat(prog);
    p.program = std::make_shared<std::vector<Instruction>>(std::move(prog));
    return p;
}

// delegates to buildProcess
Process generateProcess(int id, const std::string& name, uint32_t memSize, const Config& cfg) {
    int64_t target = randomInt(cfg.min_ins, cfg.max_ins);
    return buildProcess(id, name, memSize, genBlock(target, 0, memSize), cfg);
}

// ---- parsing (for screen -c) ----

static std::string trim(const std::string& s){
    size_t a = s.find_first_not_of(" \t\r\n");
    if(a==std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a,b-a+1);
}

static std::string upper(std::string s){
    for(auto& c: s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

static bool parseNum(const std::string& tok, uint32_t& out){
    if(tok.empty()) return false;
    try{
        bool hex = tok.size() > 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X');
        size_t used =0;
        unsigned long v = std::stoul(tok, &used, hex ? 16 : 10);
        if(used != tok.size()) return false;
        out = static_cast<uint32_t>(v);  
        return true;
    }catch(...) {return false;}
}

static bool parseOperand(const std::string& token, Operand& op){
    if(token.empty()) return false;

    if(std::isdigit(static_cast<unsigned char>(token[0]))){
        uint32_t v;
        if(!parseNum(token,v)) return false;
        op.isVariable = false;
        op.val = clamp16(static_cast<int32_t>(std::min<uint32_t>(v,65535)));
    }else{
        op.isVariable = true;
        op.name = token;
    }
    return true;
}

// for PRINT("text" + var)  |  PRINT("text")  |  PRINT(var)
static bool parsePrint(const std::string& s, Instruction& in){
    size_t open = s.find('('), close = s.rfind(')');
    if(open == std::string::npos || close == std::string::npos || close > open) return false;
    std::string inner = trim(s.substr(open + 1, close - open - 1));
    in.type = InstructionTypes::PRINT;
    if(!inner.empty() && inner[0] == '"'){
        size_t q2 = inner.find('"', 1);
        if(q2 == std::string::npos) return false;
        in.printText = inner.substr(1, q2 -1);
        std::string rest = trim(inner.substr(q2 + 1));
        if(rest.empty()) return false;
        if(rest[0] != '+') return false;
        in.printValue = trim(rest.substr(1));
        return !in.printValue.empty();
    }
    in.printValue = inner;
    return !inner.empty();
}

static bool parseOne(const std::string& raw, Instruction& in){
    std::string s = trim(raw);
    if(s.empty()) return false;
    if(upper(s.substr(0,5)) == "PRINT") return parsePrint(s,in);

    std::istringstream iss(s);
    std::string op, a, b, c, extra;
    iss >> op >> a >> b >> c >> extra;
    op = upper(op);
    if (!extra.empty()) return false;

    uint32_t n;
    if(op == "DECLARE"){
        if(a.empty() || !parseNum(b,n) || !c.empty()) return false;
        in.type = InstructionTypes::DECLARE;
        in.var1 = a;
        in.x.val = clamp16(static_cast<int32_t>(std::min<uint32_t>(n,65535)));
        return true;
    }
    if(op == "ADD" || op == "SUBTRACT"){
        if(a.empty() || !parseOperand(b, in.x) || !parseOperand(c, in.y)) return false;
        in.type = op == "ADD" ? InstructionTypes::ADD : InstructionTypes::SUBTRACT;
        in.var1 = a;
        return true;
    }
    if(op == "WRITE"){
        if(!parseNum(a, in.address) || !parseOperand(b, in.x) || !c.empty()) return false;
        in.type = InstructionTypes::WRITE;
        return true;
    }
    if(op == "READ"){
        if(a.empty() || !parseNum(b,in.address) || !c.empty()) return false;
        in.type = InstructionTypes::READ;
        in.var1 = a;
        return true;
    }
    if(op == "SLEEP"){
        if(!parseNum(a,n) || !b.empty()) return false;
        in.type = InstructionTypes::SLEEP;
        in.ticks = static_cast<uint8_t>(std::min<uint32_t>(n,255));
        return true;
    }
    return false;
}

// Splits on ';' (ignoring ';' inside quotes), turns \" into ", parses each instruction.
bool parseInstructions(const std::string& text, std::vector<Instruction>& out) {
    std::string clean;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == '"') continue;
        clean += text[i];
    }
    std::string cur;
    bool inQuote = false;
    auto flush = [&]() {
        if (trim(cur).empty()) { cur.clear(); return true; }
        Instruction in;
        if (!parseOne(cur, in)) return false;
        out.push_back(std::move(in));
        cur.clear();
        return true;
    };
    for (char ch : clean) {
        if (ch == '"') inQuote = !inQuote;
        if (ch == ';' && !inQuote) {
            if (!flush()) return false;
        } else cur += ch;
    }
    return flush();
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

static bool varAddr(Process& p, const std::string& n, bool create, uint32_t& addr){
    auto it = p.symbols.find(n);
    if(it == p.symbols.end()){
        uint32_t slot = static_cast<uint32_t>(p.symbols.size());
        if(!create || slot >= 32) return false;
        it = p.symbols.emplace(n,slot).first;
    }
    addr = it->second * 2;
    return true;
}

static uint16_t getVariable(Process& p, MemoryManager& m, const Operand& op){
    if(!op.isVariable) return op.val;
    uint32_t a;
    return varAddr(p, op.name, false, a) ? m.read16(p,a) : 0;
}

static void setVariable(Process& p, MemoryManager& m, const std::string& n, uint16_t v){
    uint32_t a;
    if(varAddr(p,n,true,a)) m.write16(p,a,v);
}

static void violate(Process& p, MemoryManager& m, uint32_t addr){
    p.violated = true;
    p.violatedAdd = addr;
    p.violationTime = hhmmss();
    p.pState = ProcessStates::FINISHED;
    m.release(p);
}

bool exeNext(Process& p, int coreId, MemoryManager& mem) {
    if (p.isFinished()) return false;
    const Instruction* in = nextSimple(p);
    if (!in) { p.pState = ProcessStates::FINISHED; return false; }

    std::vector<uint32_t> pages{0};
    if(in->type == InstructionTypes::READ || in->type == InstructionTypes::WRITE){
        if(in->address < 64 || uint64_t(in->address) + 2 > p.memSize) {
            violate(p,mem,in->address);
            return false;
        }
        uint32_t frameSize = mem.frameSize();
        pages.push_back(in->address / frameSize);
        if((in->address + 1) / frameSize != in->address / frameSize) pages.push_back((in->address + 1) / frameSize);
    }

    //Page fault handling
    if(!mem.ensureFrames(p,pages)) return false;

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
        case InstructionTypes::READ:
            setVariable(p, mem, in->var1, mem.read16(p,in->address));
            break;
        case InstructionTypes::WRITE:
            mem.write16(p,in->address,getVariable(p,mem,in->x)); 
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
