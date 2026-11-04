#include "report.h"
#include <iomanip>
#include <sstream>

std::string buildReport(const std::vector<const Process*>& procs, int totalCores, int usedCores) {
    int util = totalCores > 0 ? (usedCores * 100) / totalCores : 0;

    std::ostringstream out;
    out << "CPU utilization: " << util << "%\n";
    out << "Cores used: " << usedCores << "\n";
    out << "Cores available: " << (totalCores - usedCores) << "\n\n";
    out << "----------------------------------------\n";

    out << "Running processes:\n";
    for (const Process* p : procs) {
        if (p->isFinished() || p->core < 0) continue;
        out << std::left << std::setw(12) << p->name << " (" << p->createdAt << ")   "
            << "Core: " << p->core << "   " << p->executed << " / " << p->totalInstructions << "\n";
    }

    bool header = false;
    for (const Process* p : procs) {
        if (p->isFinished() || p->core >= 0) continue;
        if (!header) { out << "\nWaiting processes (ready/sleeping):\n"; header = true; }
        out << std::left << std::setw(12) << p->name << " (" << p->createdAt << ")   "
            << (p->pState == ProcessStates::SLEEPING ? "Sleeping" : "Ready   ") << "   "
            << p->executed << " / " << p->totalInstructions << "\n";
    }

    out << "\nFinished processes:\n";
    for (const Process* p : procs) {
        if (!p->isFinished()) continue;
        out << std::left << std::setw(12) << p->name << " (" << p->createdAt << ")   Finished   "
            << p->totalInstructions << " / " << p->totalInstructions << "\n";
    }
    out << "----------------------------------------\n";
    return out.str();
}
