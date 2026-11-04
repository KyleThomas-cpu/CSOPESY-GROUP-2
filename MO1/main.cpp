#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>
#include "scheduler.h"
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
        cout << "Current instruction line: " << p.executed << "\n";
        cout << "Lines of code: " << p.totalInstructions << "\n";
    }
}

void processScreen(Scheduler& s, const string& name){
    Process p;
    if(!s.snapshot(name, p)){
        cout << "Process " << name << " not found.\n";
        return;
    }
    clearScreen();
    printProcessInfo(p);
    string line;

    while(true){
        cout << "\nroot:\\> ";
        if(!getline(cin, line)) return;

        if(line == "exit") return;

        if(line == "process-smi"){
            if(s.snapshot(name, p)) printProcessInfo(p);
        } else {
            cout << "Unknown command.\n";
        }
    }
}

int main() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) SetConsoleMode(hOut, mode | 0x0004);  
#endif
    Config cfg;
    Scheduler s;
    bool initialized = false;
    string line;
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
                    s.init(cfg);
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
        } else if (cmd == "screen") {
            string flag, name;
            iss >> flag >> name;

            if (flag == "-s" && !name.empty()) {
                if (!s.createProcess(name)) {
                    cout << "Process " << name << " already exists.\n";
                    continue;
                }
                processScreen(s, name);
                clearScreen();
                start();
            } else if (flag == "-r" && !name.empty()) {
                Process p;
                if (!s.snapshot(name, p) || p.isFinished()) {
                    cout << "Process " << name << " not found.\n";
                } else {
                    processScreen(s, name);
                    clearScreen();
                    start();
                }
            } else if (flag == "-ls") {
                cout << s.report();
            } else {
                cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
            }
        } else if (cmd == "scheduler-start") {
            if (s.isGenerating()) cout << "Scheduler is already generating processes.\n";
            else { s.startGenerating(); cout << "Scheduler started: generating dummy processes.\n"; }
        } else if (cmd == "scheduler-stop") {
            if (!s.isGenerating()) cout << "Scheduler is not generating processes.\n";
            else { s.stopGenerating(); cout << "Scheduler stopped: no longer generating processes.\n"; }
        } else if (cmd == "report-util") {
            ofstream f("csopesy-log.txt");
            if (!f) {
                cout << "Could not write csopesy-log.txt\n";
            } else {
                f << s.report();
                f.close();
                cout << "Report generated at "
                     << std::filesystem::absolute("csopesy-log.txt").string() << "!\n";
            }
        } else if (cmd == "clear") {
            clearScreen();
            start();
        } else {
            cout << "Wrong input\n";
        }
    }
    s.shutdown();
    return 0;
}
