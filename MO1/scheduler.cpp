#include "scheduler.h"
#include "report.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

static constexpr int TICK_MS = 10;       
static constexpr int GEN_TICKS_PER_FREQ = 10;  

Scheduler::~Scheduler() { shutdown(); }

void Scheduler::init(const Config& cfg) {
    shutdown();
    {
        std::lock_guard<std::mutex> lk(mtx_);
        cfg_ = cfg;
        cores_.assign(cfg.numOfCpu, Core{});
        cycles_ = 0;
        sinceLastGen_ = 0;
    }
    running_ = true;
    thread_ = std::thread(&Scheduler::run, this);
}

void Scheduler::shutdown() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}

void Scheduler::run() {
    while (running_) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
    }
}

void Scheduler::startGenerating() {
    std::lock_guard<std::mutex> lk(mtx_);
    if (!generating_) sinceLastGen_ = 0;
    generating_ = true;
}

void Scheduler::stopGenerating() { generating_ = false; }

uint64_t Scheduler::cycles() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return cycles_;
}

void Scheduler::createLocked(const std::string& name) {
    auto res = processes_.emplace(name, generateProcess(nextId_++, name, cfg_));
    ready_.push_back(&res.first->second);       
}

bool Scheduler::createProcess(const std::string& name) {
    std::lock_guard<std::mutex> lk(mtx_);
    if (processes_.count(name)) return false;
    createLocked(name);
    return true;
}

std::string Scheduler::nextAutoName() {
    for (;;) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "p%02d", autoCounter_++);
        if (!processes_.count(buf)) return buf;
    }
}

bool Scheduler::snapshot(const std::string& name, Process& out) const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto it = processes_.find(name);
    if (it == processes_.end()) return false;
    out = it->second;
    return true;
}

std::string Scheduler::report() const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<const Process*> procs;
    procs.reserve(processes_.size());
    for (const auto& kv : processes_) procs.push_back(&kv.second);
    std::sort(procs.begin(), procs.end(),
              [](const Process* a, const Process* b) { return a->uniqueId < b->uniqueId; });
    int used = 0;
    for (const auto& c : cores_) if (c.proc) ++used;
    return buildReport(procs, static_cast<int>(cores_.size()), used);
}

void Scheduler::tick() {
    std::lock_guard<std::mutex> lk(mtx_);
    ++cycles_;
    const bool rr = (cfg_.scheduler == "rr");

    for (size_t i = 0; i < sleeping_.size();) {
        Process* p = sleeping_[i];
        if (tickSleep(*p)) {
            sleeping_.erase(sleeping_.begin() + static_cast<std::ptrdiff_t>(i));
            if (!p->isFinished()) ready_.push_back(p);
        } else ++i;
    }

    if (generating_) {
        if (++sinceLastGen_ >= static_cast<uint64_t>(cfg_.batchProcessFreq) * GEN_TICKS_PER_FREQ) {
            sinceLastGen_ = 0;
            createLocked(nextAutoName());
        }
    }
    auto dispatch = [&]() {
        for (size_t i = 0; i < cores_.size() && !ready_.empty(); ++i) {
            if (cores_[i].proc) continue;
            Process* p = ready_.front();
            ready_.pop_front();
            p->pState = ProcessStates::RUNNING;
            p->core = static_cast<int>(i);
            cores_[i].proc = p;
            cores_[i].ran = 0;
        }
    };
    dispatch();

    for (size_t i = 0; i < cores_.size(); ++i) {
        Core& c = cores_[i];
        if (!c.proc) continue;
        Process& p = *c.proc;

        if (p.waitRemaining > 0) {
            --p.waitRemaining;                       
        } else {
            exeNext(p, static_cast<int>(i));
            p.waitRemaining = cfg_.delayPerExec;
        }
        ++c.ran;

        if (p.isFinished()) {
            p.core = -1;
            c.proc = nullptr;
        } else if (p.pState == ProcessStates::SLEEPING) {  
            p.core = -1;
            c.proc = nullptr;
            sleeping_.push_back(&p);
        } else if (rr && c.ran >= cfg_.quantumCycles) {     
            if (!ready_.empty()) {
                p.pState = ProcessStates::READY;
                p.core = -1;
                c.proc = nullptr;
                ready_.push_back(&p);
            } else {
                c.ran = 0;                                  
            }
        }
    }
    dispatch();
}