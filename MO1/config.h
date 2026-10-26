#pragma once
#include <string>

struct Config{
    uint32_t numOfCpu = 0;
    std::string scheduler;
    uint32_t quantumCycles = 0;
    uint32_t batchProcessFreq = 0;
    uint32_t min_ins = 0;
    uint32_t max_ins = 0;
    uint32_t delaysPerSec = 0;
};

bool loadConfig(const std::string& path, Config& out, std::string& error);