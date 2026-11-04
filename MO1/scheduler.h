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

// Real-time CPU scheduler. Once init() is called a background thread ticks continuously;
// every tick runs one "CPU cycle" on every core. All shared state is guarded by mtx_.
class Scheduler {
public:
    Scheduler() = default;
    ~Scheduler();
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void init(const Config& cfg);     // configures cores and starts the scheduler thread
    void shutdown();                  // stops the scheduler thread

    void startGenerating();           // scheduler-start
    void stopGenerating();            // scheduler-stop
    bool isGenerating() const { return generating_; }

    bool createProcess(const std::string& name);                    // false if name is taken
    bool snapshot(const std::string& name, Process& out) const;     // copy of a process (thread-safe)
    std::string report() const;                                     // screen -ls / report-util text
    uint64_t cycles() const;

private:
    struct Core {
        Process* proc = nullptr;
        uint64_t ran = 0;             // cycles used in the current time slice (RR)
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
    std::map<std::string, Process> processes_;   // std::map keeps Process* stable as it grows
    std::deque<Process*> ready_;
    std::vector<Process*> sleeping_;

    int nextId_ = 1;
    int autoCounter_ = 1;
    uint64_t cycles_ = 0;
    uint64_t sinceLastGen_ = 0;
};
