#pragma once
#include <map>
#include <string>
#include "process.h"
#include "config.h"

std::string buildReport(const std::map<std::string, Process>& processes, const Config& cfg);
