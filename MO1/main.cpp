#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include "config.h"

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

int main() {
    Config cfg;
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
            string args;
            getline(iss, args);
            cout << "screen command recognized. args:" << args << "\n";
        } else if (cmd == "scheduler-start") {
            cout << "scheduler-start command recognized.\n";
        } else if (cmd == "scheduler-stop") {
            cout << "scheduler-stop command recognized.\n";
        } else if (cmd == "report-util") {
            cout << "report-util command recognized.\n";
        } else if (cmd == "clear") {
            clearScreen();
            start();
        } else {
            cout << "Wrong input\n";
        }
    }
    return 0;
}