#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>

using namespace std::this_thread; 
using namespace std::chrono;

class Marquee{
    public:
        int speed;
        char text[80] = "test";
        char screen[30][80] {};
        bool running = false;
        int time = 100;
        bool exit = false;
        
    void animate(){
        
        int x = 0;
        int y = 0;
        int dx = 1;
        int dy = 1;
        for (int i = 0; i < 30; i++) {
            for (int j = 0; j < 80; j++) {
                screen[i][j] = '.';
            }
        }
        strcpy(screen[y] + x, text);
        printFrame(screen);
        x += dx;
        y += dy;
        while(running){
            for (int i = 0; i < 30; i++) {
                for (int j = 0; j < 80; j++) {
                    screen[i][j] = '.';
                }
            }
            if (x == 79-strlen(text) || x == 0){
                dx = -dx;
            }
            if (y == 29 || y == 0){
                dy = -dy;
            }
            strcpy(screen[y] + x, text);
            printFrame(screen);
            x += dx;
            y += dy;
            sleep_for(milliseconds(time));
        }
    }
    void printFrame(char arr[][80]){
        std::cout << "\033[s";
        std::cout << "\033[H";
        for (int i = 0; i < 30; i++) {
            for (int j = 0; j < 80; j++) {
                std::cout << arr[i][j];
            }
            std::cout << '\n';
        }
        std::cout << "\033[u";
        std::cout << std::flush;

    }
    void setText(const char newtext[]){
        strcpy(text, newtext);
    }
    void startMarquee(){
        if(running) return; 
        running = true;
        std::cout << "\033[2J"; 
        std::thread t(&Marquee::animate, this);
        t.detach();
    }
    void stopMarquee(){
        running = false;
        
    }
    void setTime(int newTime){
        time = newTime;
    }
    void setExit(){
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

void helpFunction(){
    std::cout << "\nAvailable commands:\n";
    std::cout << "  help            - displays the commands and description\n";
    std::cout << "  start_marquee   - starts marquee animation\n";
    std::cout << "  stop_marquee    - stops marquee animation\n";
    std::cout << "  set_text <text> - sets the text that will display as marquee\n";
    std::cout << "  set_speed <ms>  - sets how fast the marquee animation will refresh in ms\n";
    std::cout << "  exit            - exits the console\n\n";
}


void parseCommand(const std::string& line, Marquee& m){
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    std::string rest;
    std::getline(iss, rest);
    if(!rest.empty() && rest[0] == ' ') rest.erase(0,1);

    if(cmd.empty()){
        std::cout << "Command>" << std::flush;
        return;
    }else if(cmd == "help"){
        helpFunction();
        std::cout << "Command>" << std::flush;
    }else if(cmd == "start_marquee"){
        m.startMarquee();
        std::cout << "Command>" << std::flush;
    }else if(cmd == "stop_marquee"){
        m.stopMarquee();
        std::cout << "Command>" << std::flush;
    }else if(cmd == "set_text"){
        if(rest.empty()){
            std::cout << "Usage: set_text <text>\n\n";
            std::cout << "Command>" << std::flush;
        }else{
            m.setText(rest.c_str());
            std::cout << "Marquee text set to \"" << rest << "\"\n\n";
            std::cout << "Command>" << std::flush;
        }
    }else if(cmd == "set_speed"){
        if(rest.empty()){
            std::cout << "Usage: set_speed <ms>\n\n";
            std::cout << "Command>" << std::flush;
            return;
        }else{
            int ms = std::stoi(rest);
            m.setTime(ms);
            std::cout << "Marquee speed set to " << ms << "ms\n\n";
            std::cout << "Command>" << std::flush;
        }
    }else if(cmd == "exit"){
        m.setExit();
    }else{
        std::cout << "Unknown command '" << cmd << "'. Type 'help' for a list of commands.\n\n";
        std::cout << "Command>" << std::flush;
    }
}

int main(){
    Marquee m;
    std::string command;
    printHeader();
    std::cout<< "Command>";
    while(!m.exit){
        std::getline(std::cin, command);
        parseCommand(command, m);
    }
}