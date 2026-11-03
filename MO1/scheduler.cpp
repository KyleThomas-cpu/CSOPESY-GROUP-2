#include "scheduler.h"

#include <cstdio>
#include <thread>
#include <chrono>
static constexpr int TICK_MS = 10;
Scheduler::~Scheduler() {
    stopRunning();
}

void Scheduler::startRunning() {
    if (runningScheduling_) {
        return;
    }
    runningScheduling_ = true;

    schedulerThread_ = std::thread([this]() {
        while (runningScheduling_) {
            tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
        }
    });
}

void Scheduler::stopRunning() {
    runningScheduling_ = false;

    if (schedulerThread_.joinable()) {
        schedulerThread_.join();
    }
}

Process* Scheduler::createProcess(const std::string& name) {
    if (processes_.count(name)) return nullptr;
    auto res = processes_.emplace(name, generateProcess(nextId_++, name, cfg_));
    Process* p = &res.first->second;
    ready_.push_back(p);          // new processes join the back of the line
    return p;
}

Process* Scheduler::findActive(const std::string& name) {
    auto it = processes_.find(name);
    if (it == processes_.end() || it->second.isFinished()) return nullptr;
    return &it->second;
}

void Scheduler::tick() {
    cycles_++;
    for (size_t i = 0; i < cores_.size() && !ready_.empty(); i++) {
        if (!cores_[i].isFree()) {
            continue;
        }
        Process* p = ready_.front();
        ready_.pop_front();
        p->pState = ProcessStates::RUNNING;
        p->core = static_cast<int>(i);
        cores_[i].proc = p;
    }

    for (size_t i = 0; i < cores_.size(); i++) {
        if (cores_[i].isFree()) continue;
        Process& p = *cores_[i].proc;
        exeNext(p, static_cast<int>(i));
        if (p.isFinished()) {
            p.core = -1;
            cores_[i].proc = nullptr;
        }
    }

}


