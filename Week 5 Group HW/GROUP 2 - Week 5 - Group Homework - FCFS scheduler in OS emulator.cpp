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
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <array>
#include <memory>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"

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
    cout << BLUE
        << "Hello, Welcome to CSOPESY commandline!\n";
    cout << YELLOW
        << "Developers:Ronin Zerna, Luis gutierrez, Kyle Tiu, John Go\n";
    cout << MAGENTA
        << "Type command ""screen -ls"" to view processes or ""exit"" to close the program.\n\n";
    cout << RESET;
}

string currentTimestamp() {
    time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    ostringstream ss;
    ss << put_time(&tm, "(%m/%d/%Y %I:%M:%S%p)");
    return ss.str();
}

class Process
{
    private:
        string name;
        int id;
        int totalInstructions;
        atomic<int> remainingInstructions;
        atomic<int> currCore{-1};
        string startTime;   
        ofstream logFile;

    public:
        Process(const string& processName, int processId, int numInstructions)
        : name(processName), id(processId), totalInstructions(numInstructions),
        remainingInstructions(numInstructions) {
                logFile.open(name + ".txt");
                logFile << "Process name: " << name << "\n";
                logFile << "Logs:\n\n";
            }

    void executeInstruction(){
        if(remainingInstructions > 0){
                logFile << currentTimestamp() << " Core:" << currCore.load()
                        << " \"Hello world from " << name << "!\"\n";
            remainingInstructions--;
        }
    }

    bool hasFinished() const { return remainingInstructions == 0; }
    void setCore(int core) { currCore = core; }
    void setStartTime(const string &t) { startTime = t; }

    void closeLog(){ if (logFile.is_open()) logFile.close(); }

    int getTotalInstructions() const { return totalInstructions; }
    int getDone() const { return totalInstructions - remainingInstructions; }
    int getId() const { return id; }
    int getCore() const { return currCore; }
    const string& getName() const { return name; }
    const string& getStartTime() const { return startTime; }
};

class Scheduler
{
    private:
        mutex mtx;
        condition_variable schedCv;                           
        array<condition_variable, NUMBER_OF_CORES> coreCv; 

        queue<shared_ptr<Process>> readyQueue;           
        array<shared_ptr<Process>, NUMBER_OF_CORES> coreSlot; 
        vector<shared_ptr<Process>> finished;            

        atomic<bool> stopping{false};
        thread schedThread;
        vector<thread> workers;

        void schedulerLoop() {
            unique_lock<mutex> lk(mtx);
            while (true) {
                int idle = -1;
                schedCv.wait(lk, [&]{
                    if (stopping) return true;
                    if (readyQueue.empty()) return false;
                    for (int c = 0; c < NUMBER_OF_CORES; c++)
                        if (!coreSlot[c]) { idle = c; return true; }
                    return false;
                });
                if (stopping) break;

                coreSlot[idle] = readyQueue.front();
                readyQueue.pop();
                coreCv[idle].notify_one();
            }
        }

        void coreLoop(int core) {
            unique_lock<mutex> lk(mtx);
            while (true) {
                coreCv[core].wait(lk, [&]{ return stopping || coreSlot[core]; });
                if (stopping) break;

                auto p = coreSlot[core];
                p->setCore(core);
                p->setStartTime(currentTimestamp());
                lk.unlock();

                while (!p->hasFinished() && !stopping) {
                    p->executeInstruction();
                    this_thread::sleep_for(chrono::milliseconds(INSTRUCTION_DELAY));
                }

                lk.lock();
                p->closeLog();
                if (p->hasFinished()) finished.push_back(p);
                coreSlot[core] = nullptr;
                schedCv.notify_one();
            }
        }

    public:
        void start() {
            schedThread = thread(&Scheduler::schedulerLoop, this);
            for (int c = 0; c < NUMBER_OF_CORES; c++)
                workers.emplace_back(&Scheduler::coreLoop, this, c);
        }

        void addProcess(shared_ptr<Process> p) {
            { lock_guard<mutex> lk(mtx); readyQueue.push(move(p)); }
            schedCv.notify_one();
        }

        void stop() {
            { lock_guard<mutex> lk(mtx); stopping = true; }
            schedCv.notify_all();
            for (auto& cv : coreCv) cv.notify_all();
            if (schedThread.joinable()) schedThread.join();
            for (auto& w : workers) if (w.joinable()) w.join();
            workers.clear();
        }

        void printStatus() {
            lock_guard<mutex> lk(mtx);
            ostringstream out;

            out << "--------------------------------------\n";
            out << "Running processes:\n";
            for (int c = 0; c < NUMBER_OF_CORES; c++) {
                auto& p = coreSlot[c];
                if (!p || p->getCore() != c || p->getStartTime().empty()) continue;
                out << left << setw(12) << p->getName()
                    << RED << setw(26) << p->getStartTime() << RESET
                    << "Core: " << setw(8) << c
                    << RED << p->getDone() << " / " << p->getTotalInstructions() << RESET << "\n";
            }

            out << "\nFinished processes:\n";
            for (auto& p : finished) {
                out << left << setw(12) << p->getName()
                    << GREEN << setw(26) << p->getStartTime() << RESET
                    << setw(14) << "Finished"
                    << GREEN << p->getDone() << " / " << p->getTotalInstructions() << RESET << "\n";
            }
            out << "--------------------------------------\n";
            cout << out.str() << flush;
        }
};

int main(){
    start();

    Scheduler scheduler;
    scheduler.start();

    for (int i = 1; i <= NUMBER_OF_PROCESSES; i++) {
        ostringstream nm;
        nm << "process" << setw(2) << setfill('0') << i;
        scheduler.addProcess(make_shared<Process>(nm.str(), i, PRINTS_PER_PROCESS));
    }

    string inputt;
    while(true){
        cout << "root:\\ " << flush;
        if(!getline(cin, inputt)) break;

        inputt.erase(0, inputt.find_first_not_of(" \t\r\n"));
        inputt.erase(inputt.find_last_not_of(" \t\r\n") + 1);

        if (inputt == "screen -ls"){
            scheduler.printStatus();
        }else if(inputt == "exit"){
            break;
        }else if (!inputt.empty()){
            cout << "Unknown command: " << inputt << "\n";
        }
    }

    scheduler.stop();
    return 0;
}