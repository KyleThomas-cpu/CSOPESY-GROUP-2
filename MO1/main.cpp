#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <map>
#include "config.h"
#include "process.h"
#include "report.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"


using namespace std;
void start(){
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
    cout << RESET;
    cout << R"(
   _____ ____   ___  ____  _____ ____ __   __
  / ____/ ___| / _ \|  _ \| ____/ ___|\ \ / /
 | |    \___ \| | | | |_) |  _| \___ \ \ V /
 | |___  ___) | |_| |  __/| |___ ___) | | |
  \_____|____/ \___/|_|   |_____|____/  |_|
)";
    cout << GREEN
        << "Hello, Welcome to CSOPESY Emulator!\n";
    cout << YELLOW
        << "Type 'exit' to quit, 'clear' to clear the screen\n\n** IMPORTANT: Type 'initialize' to load config and start system **\n";
    cout << YELLOW 
         << "Current Date: " << buffer << '\n';
    cout << RESET;
}

void clearScreen() {
    cout << "\033[2J\033[H";   
}

void printProcessInfo(const Process& p){
    cout << "Process name: " << p.name << "\n";
    cout << "ID: " << p.uniqueId << "\n";
    cout << "Logs:\n";
    for(const auto& l : p.logs) cout << l << "\n";
    cout << "\n";
    if(p.isFinished()){
        cout << "Finished!\n";
    }else{
        cout << "Current instruction line: " << p.pc << "\n";
        cout << "Lines of code: " << p.instructions.size() << "\n";
    }
}

void processScreen(Process& p ){
    clearScreen();
    printProcessInfo(p);
    string line;
    
    while(true){
        cout << "\nroot:\\> ";
        if(!getline(cin, line)) return;

        if(line == "exit") return;

        if(line == "process-smi"){
            printProcessInfo(p);
        }else if (line.rfind("run", 0) == 0) {   // DEBUG ONLY: delete once the scheduler exists
            int n = 1;
            try { if (line.size() > 4) n = stoi(line.substr(4)); } catch (...) {}
            for (int i = 0; i < n && !p.isFinished(); ++i) {
                exeNext(p, 0);
                while (p.pState == ProcessStates::SLEEPING) tickSleep(p);
            }
        } else {
            cout << "Unknown command.\n";
        }
    }
}

int main() {
    Config cfg;
    map<string, Process> processes;
    bool initialized = false;
    string line;
    int nextId = 1;
    start();

    while (true) {
        cout << "root:\\> ";
        if (!getline(cin, line)) break;

        istringstream iss(line);
        string cmd;
        if (!(iss >> cmd)) continue;

        if (cmd == "exit") break;

        if (!initialized) {
            if (cmd == "initialize") {
                string error;
                Config temp;
                if (loadConfig("config.txt", temp, error)) {
                    cfg = temp;
                    initialized = true;
                    cout << "Initialized: " << cfg.numOfCpu << " CPU(s), scheduler="
                         << cfg.scheduler << "\n";
                } else {
                    cout << "Initialization failed: " << error << "\n";
                }
            } else {
                cout << "user must run the initialize command first\n";
            }
            continue;
        }

        if (cmd == "initialize") {
            cout << "Already initialized.\n";
        } 

        else if (cmd == "screen") {
            string flag, name;
            iss >> flag >> name;

            if(flag == "-s" && !name.empty()){
                if(processes.count(name)){
                    cout << "Process " << name << " already exists.\n";
                    continue; 
                }
                processes[name] = generateProcess(nextId++, name, cfg);
                processScreen(processes[name]);
                clearScreen();
                start();
            }else if(flag == "-r" && !name.empty()){
                auto it = processes.find(name);
                if(it == processes.end() || it->second.isFinished()){
                    cout << "Process " + name << " not found.\n";
                }else{
                    processScreen(it->second);
                    clearScreen();
                    start();
                }
            }else if(flag == "-ls"){
                cout << buildReport(processes, cfg);
            }else{
                cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
            }

        }
         else if (cmd == "scheduler-start") {
            cout << "scheduler-start command recognized.\n";
        } else if (cmd == "scheduler-stop") {
            cout << "scheduler-stop command recognized.\n";
        } else if (cmd == "report-util") {
            //
        } else if (cmd == "clear") {
            clearScreen();
            start();
        } else {
            cout << "Wrong input\n";
        }
    }
    return 0;
}