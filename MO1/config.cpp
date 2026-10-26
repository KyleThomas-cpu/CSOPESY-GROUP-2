#include "config.h"
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>

static bool parseInt(const std::string& s, uint64_t& out) {
    if (s.empty() || s.size() > 10 || !std::all_of(s.begin(), s.end(), ::isdigit)) return false;
    out = std::stoull(s);                       // at most 10 digits, can't throw
    return out <= 0xFFFFFFFFULL;
}

static std::string stripQuotes(std::string s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        s = s.substr(1, s.size() - 2);
    return s;
}

bool loadConfig(const std::string& path, Config& config, std::string& error) {
    std::ifstream file(path);
    if (!file) { error = "Cannot open " + path; return false; }

    std::set<std::string> seenParams;
    std::string line;
    int lineNo = 0;

    while (std::getline(file, line)) {
        ++lineNo;
        std::istringstream iss(line);
        std::string key, value;

        if (!(iss >> key)) continue;
        if (!(iss >> value)) {
            error = "Line " + std::to_string(lineNo) + ": missing value for '" + key + "'";
            return false;
        }

        uint64_t n = 0;
        auto num = [&](uint64_t low, uint64_t high, uint32_t& dst) {
            if (!parseInt(value, n) || n < low || n > high) {
                error = "'" + key + "' must be an integer in range [" +
                        std::to_string(low) + "," + std::to_string(high) + "]";
                return false;
            }
            dst = static_cast<uint32_t>(n);
            return true;
        };

        if (key == "num-cpu") {
            if (!num(1, 128, config.numOfCpu)) return false;
        } else if (key == "scheduler") {
            value = stripQuotes(value);
            if (value != "fcfs" && value != "rr") {
                error = "'scheduler' must be \"fcfs\" or \"rr\"";
                return false;
            }
            config.scheduler = value;
        } else if (key == "quantum-cycles") {
            if (!num(1, 0xFFFFFFFFULL, config.quantumCycles)) return false;
        } else if (key == "batch-process-freq") {
            if (!num(1, 0xFFFFFFFFULL, config.batchProcessFreq)) return false;
        } else if (key == "min-ins") {
            if (!num(1, 0xFFFFFFFFULL, config.min_ins)) return false;
        } else if (key == "max-ins") {
            if (!num(1, 0xFFFFFFFFULL, config.max_ins)) return false;
        } else if (key == "delay-per-exec" || key == "delays-per-exec") {
            key = "delay-per-exec";
            if (!num(0, 0xFFFFFFFFULL, config.delaysPerSec)) return false;
        } else {
            error = "Line " + std::to_string(lineNo) + ": unknown parameter '" + key + "'";
            return false;
        }
        seenParams.insert(key);
    }

    const char* requiredKeys[] = {"num-cpu", "scheduler", "quantum-cycles",
                                  "batch-process-freq", "min-ins", "max-ins", "delay-per-exec"};
    for (const char* req : requiredKeys) {
        if (seenParams.count(req) == 0) {
            error = "Missing required parameter '" + std::string(req) + "'";
            return false;
        }
    }

    if (config.min_ins > config.max_ins) { error = "min-ins must be <= max-ins"; return false; }
    return true;
}