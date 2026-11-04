#pragma once
#include <string>
#include <vector>
#include "process.h"

std::string buildReport(const std::vector<const Process*>& procs, int totalCores, int usedCores);
