#include "report.h"
#include <sstream>
#include <iomanip>
#include <set>
#include <fstream>

std::string buildReport(const std::map<std::string, Process>& processes, const Config& cfg){
    std::set<int> usedCores;
    for(const auto& [name, p] : processes){
        if(p.pState == ProcessStates::RUNNING && p.core >= 0) usedCores.insert(p.core);
    }
    int total = static_cast<int>(cfg.numOfCpu);
    int used = static_cast<int>(usedCores.size());
    int util = total > 0 ? (used * 100) / total : 0;

    std::ostringstream out;
    out << "CPU utilization: " << util << "%\n";
    out << "Cores used: " << used << "\n";
    out << "Cores available: " << (total - used) << "\n\n";
    out << "----------------------------------------\n";

    out << "Running processes:\n";
    for (const auto& [name, p] : processes){
        if(p.isFinished()) continue;

        out << std::left << std::setw(12) << p.name << " (" << p.createdAt << ")   ";
        if(p.pState == ProcessStates::RUNNING){
            out << "Core: " << p.core;
        }else{
            out << "Core: -";
        }
        out << "   " << p.pc << " / "<< p.instructions.size() << "\n";
    }

    out << "\nFinished processes:\n";
    for(const auto&[name, p] :processes){
        if(!p.isFinished()) continue;

        out << std::left << std::setw(12) << p.name << " (" << p.createdAt << ")   Finished   " 
        << p.instructions.size() << " / " << p.instructions.size() << "\n";
    }
    out << "----------------------------------------\n";
    return out.str();
}
