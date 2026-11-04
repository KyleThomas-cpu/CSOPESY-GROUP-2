#pragma once
#include <string>
#include <vector>
#include "process.h"

// `procs` must be ordered by creation. Used by both "screen -ls" and "report-util".
std::string buildReport(const std::vector<const Process*>& procs, int totalCores, int usedCores);
