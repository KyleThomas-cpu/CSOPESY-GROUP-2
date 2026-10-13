#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <atomic>
#include <condition_variable>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

const int NUMBER_OF_CORES = 4;
const int NUMBER_OF_PROCESSES = 10;
const int INSTRUCTION_DELAY = 100;
const int PRINTS_PER_PROCESS = 100;

using namespace std;
void start(){
    cout << RESET;
    cout << R"(
   _____ ____   ___  ____  _____ ____ __   __
  / ____/ ___| / _ \|  _ \| ____/ ___|\ \ / /
 | |    \___ \| | | | |_) |  _| \___ \ \ V /
 | |___  ___) | |_| |  __/| |___ ___) | | |
  \_____|____/ \___/|_|   |_____|____/  |_|
)";
    cout << GREEN
        << "Hello, Welcome to CSOPESY commandline!\n";
    cout << YELLOW
        << "Developers:\nRonin Zerna, <add ur names here>";
    cout << RESET;
}



std::string currentTimestamp() {
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "(%m/%d/%Y %I:%M:%S%p)");
    return ss.str();
}

class Process
{
    private:
        std::string name;
        int id;
        int totalInstructions;
        std::atomic<int> remainingInstructions;
        std::atomic<int> currCore{-1};
        std::string startTime;
        std::ofstream logFile;

    public:
        Process(const std::string& processName, int processId, int numInstructions)
        : name(processName), id(processId), totalInstructions(numInstructions) ,
        remainingInstructions(numInstructions) {
            logFile.open(name + ".txt");
            logFile << "Process name: " << name << "\n";
            logFile << "Logs:\n\n";
        }

    void executeInstruction(){
        if(remainingInstructions > 0){
            logFile << currentTimestamp() << " Core:" << currCore.load() << " \"Hello world from " << name << "!\"\n";
            remainingInstructions--;
        }
    }  

    bool hasFinished() const {return remainingInstructions == 0; }
    void setCore(int core) { currCore = core;}
    // void setStartTime(const std::string &t) {startTime = t;}

    void closeLog(){if (logFile.is_open()) logFile.close();}
    
    int getRemainingInstructions() const{
        return remainingInstructions;
    }
    int getTotalInstructions() const {return totalInstructions;}
    int getDone() const {return totalInstructions - remainingInstructions;}
    int getId() const {return id;}
    int getCore() const {return currCore;}
    const std::string& getName(){return name;}
    const std::string& getStartTime() const { return startTime; }
};


int main(){
    start();

    std::string inputt;
    while(true){
        std::cout << "root:\\ " << std::flush;
        if(!std::getline(std::cin, inputt)) break;

        inputt.erase(0, inputt.find_first_not_of(" \t\r\n"));
        inputt.erase(inputt.find_last_not_of(" \t\r\n") +  1);
        if (inputt == "screen -ls"){
            // scheduler func
        }else if(inputt == "ëxit"){
            break;
        }else if (!inputt.empty()){
            std::cout << "Unknown command: " << inputt << "\n";
        }

    }
}