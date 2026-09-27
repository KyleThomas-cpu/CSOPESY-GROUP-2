#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>
#include <atomic>
#include <mutex>

using namespace std::this_thread; 
using namespace std::chrono;

class Marquee{
    public:
        int speed;
        char text[80] = "test";
        char screen[20][80] {};
        std::atomic<bool> running = false;
        int time = 100;
        std::atomic<bool> exit = false;
        std::thread worker;
        std::mutex textMutex;
        std::mutex outputMutex;
    
    void drawEmptyGrid(){
        char blank[20][80];
        for (int i = 0; i < 20; i++)
            for (int j = 0; j < 80; j++)
                blank[i][j] = '.';
        printFrame(blank);
    }
    void animate(){
        
        int x = 0;
        int y = 0;
        int dx = 1;
        int dy = 1;
        for (int i = 0; i < 20; i++) {
            for (int j = 0; j < 80; j++) {
                screen[i][j] = '.';
            }
        }
        printFrame(screen);
        x += dx;
        y += dy;
        while(running){ 
            char localText[80];
            int localTime;
            {
                std::lock_guard<std::mutex> lock(textMutex);
                strcpy(localText, text);
                localTime = time;
            }

            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 80; j++) {
                    screen[i][j] = '.';
                }
            }

            if (x >= 79 - strlen(localText) || x <= 0){
                dx = -dx;
            }
            if (y >= 19 || y <= 0){
                dy = -dy;
            }
            int maxLen = 80 - x;

            if (strlen(localText) < maxLen) {
                maxLen = strlen(localText);
            }

            for (int i = 0; i < maxLen; i++) {
                screen[y][x + i] = localText[i];
            }

            printFrame(screen);

            x += dx;
            y += dy;

            sleep_for(milliseconds(localTime));
        }
    }
    void printFrame(char arr[][80]){
        std::lock_guard<std::mutex> lock(outputMutex);
        std::string frame;
        frame += "\033[H";
        for (int i = 0; i < 20; i++) {
            frame += "\033[2K";
            for (int j = 0; j < 80; j++) {
                frame += arr[i][j];
            }
            if (i < 19) {
                frame += '\n';
            }
        }
        frame += "\033[22;1H";
        frame += "Command>";
        std::cout << frame << std::flush;
    }
    void setText(const char newtext[]){
        std::lock_guard<std::mutex> lock(textMutex);
        strcpy(text, newtext);
    }
    void startMarquee(){
        if(running) return;
        running = true;
        std::cout << "\033[2J";
        worker = std::thread(&Marquee::animate, this);
    }
    void stopMarquee(){
        running = false;
        if(worker.joinable()){
            worker.join();
        }
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "\033[2J";
        std::cout << "\033[H";
        std::cout << std::flush;
    }
    void setTime(int newTime){
        std::lock_guard<std::mutex> lock(textMutex);
        time = newTime;
    }
    void setExit(){
        stopMarquee();
        exit = true;
    }
};

void printHeader(){
    std::cout << "Welcome to CSOPESY!\n\n";
    std::cout << "Group Developers:\n";
    std::cout << "Zerna, Ronin\n";
    std::cout << "Tiu, Kyle Thomas\n";
    std::cout << "Go, John William\n";
    std::cout << "Gutierrez, Michael Luis\n";
    std::cout << "Version Date: <Date of Completion>\n\n";
}

void parseCommand(const std::string& line, Marquee& m){
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    std::string rest;
    std::getline(iss, rest);
    if(!rest.empty() && rest[0] == ' ') rest.erase(0,1);

    if(cmd.empty()){
        return;

    }else if(cmd == "help"){
        std::lock_guard<std::mutex> lock(m.outputMutex);
        std::cout << "\033[22;1H";
        std::cout << "\033[2K";
        std::cout << "Available commands:\n";
        std::cout << "  help            - displays the commands and description\n";
        std::cout << "  start_marquee   - starts marquee animation\n";
        std::cout << "  stop_marquee    - stops marquee animation\n";
        std::cout << "  set_text <text> - sets the text that will display as marquee\n";
        std::cout << "  set_speed <ms>  - sets how fast the marquee animation will refresh in ms\n";
        std::cout << "  exit            - exits the console\n\n";

        std::cout << "Command>" << std::flush;

    }else if(cmd == "start_marquee"){
        m.startMarquee();

    }else if(cmd == "stop_marquee"){
        m.stopMarquee();

        std::lock_guard<std::mutex> lock(m.outputMutex);
        std::cout << "Command>" << std::flush;

    }else if(cmd == "set_text"){
        std::lock_guard<std::mutex> lock(m.outputMutex);

        std::cout << "\033[22;1H";
        std::cout << "\033[2K";

        if(rest.empty()){
            std::cout << "Usage: set_text <text>\n\n";
        }else{
            m.setText(rest.c_str());
            std::cout << "Marquee text set to \"" << rest << "\"\n\n";
        }

        std::cout << "Command>" << std::flush;

    }else if(cmd == "set_speed"){
        std::lock_guard<std::mutex> lock(m.outputMutex);
        std::cout << "\033[22;1H";
        std::cout << "\033[2K";
        if(rest.empty()){
            std::cout << "Usage: set_speed <ms>\n\n";
        }else{
            int ms = std::stoi(rest);
            m.setTime(ms);

            std::cout << "Marquee speed set to " << ms << "ms\n\n";
        }

        std::cout << "Command>" << std::flush;

    }else if(cmd == "exit"){
        m.setExit();

    }else{
        std::lock_guard<std::mutex> lock(m.outputMutex);

        std::cout << "\033[22;1H";
        std::cout << "\033[2K";

        std::cout << "Unknown command '" << cmd
                  << "'. Type 'help' for a list of commands.\n\n";

        std::cout << "Command>" << std::flush;
    }
}

int main(){
    Marquee m;
    std::string command;
    std::cout << "\033[2J";
    std::cout << "\033[H";
    printHeader();
    m.drawEmptyGrid();
    while(!m.exit){
        std::getline(std::cin, command);
        {
            std::lock_guard<std::mutex> lock(m.outputMutex);

            std::cout << "\033[22;1H";

            // Clear rows 22-40
            for(int i = 22; i <= 40; i++){
                std::cout << "\033[2K";
                std::cout << "\033[1B";
            }

            std::cout << "\033[22;1H";
        }

        parseCommand(command, m);
    }
}
