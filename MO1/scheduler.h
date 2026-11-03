#pragma once
#include <deque>
#include <map>
#include <string>
#include "config.h"
#include "process.h"
#include <cstdint>
#ifndef CSOPESY_GROUP_2_SCHEDULER_H
#define CSOPESY_GROUP_2_SCHEDULER_H
#include <vector>
#include <thread>
#include <thread>
#include <atomic>

class Scheduler {
public:
    ~Scheduler();
    bool isRunning() const { return runningScheduling_; }
    void startRunning();
    void stopRunning();


    void tick();
    void init(const Config& cfg) {
        cfg_ = cfg;
        cores_.assign(cfg.numOfCpu, Core{});
    }
    Process* createProcess(const std::string& name);   // nullptr if the name is taken
    Process* findActive(const std::string& name);      // nullptr if missing or finished
    const std::map<std::string, Process>& processes() const { return processes_; }
private:
    bool runningScheduling_ = false;
    std::thread schedulerThread_;
    int nextId_ = 1;
    int numOfProcesses_ = 1;

    struct Core {
        Process* proc = nullptr;
        bool isFree() const { return proc == nullptr; }
    };
    std::vector<Core> cores_;
    Config cfg_;
    std::map<std::string, Process> processes_;   // std::map keeps Process* valid as it grows
    std::deque<Process*> ready_;                 // the line of waiting processes

    uint64_t cycles_ = 0;

};


#endif //CSOPESY_GROUP_2_SCHEDULER_H
