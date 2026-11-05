#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "config.h"
#include "process.h"

class Scheduler {
public:
    Scheduler() = default;
    ~Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void init(const Config& cfg);     
    void shutdown();                  

    void startGenerating();           
    void stopGenerating();           
    bool isGenerating() const { return generating_; }

    bool createProcess(const std::string& name);                    
    bool snapshot(const std::string& name, Process& out) const;     
    std::string report() const;                                     
    uint64_t cycles() const;

private:
    struct Core {
        Process* proc = nullptr;
        uint64_t ran = 0;             
    };

    void run();
    void tick();
    void createLocked(const std::string& name);
    std::string nextAutoName();

    mutable std::mutex mtx_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> generating_{false};

    Config cfg_;
    std::vector<Core> cores_;
    std::map<std::string, Process> processes_;   
    std::deque<Process*> ready_;
    std::vector<Process*> sleeping_;

    int nextId_ = 1;
    int autoCounter_ = 1;
    uint64_t cycles_ = 0;
    uint64_t sinceLastGen_ = 0;
};
