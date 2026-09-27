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
        std::string text = "testing123";
        bool running = false;
        int time = 1000;
        bool exit = false;
    void animate(){
        while(running){
            std::cout << "\0337";
            std::cout << "\033[1A\r";
            std::cout << "\033[2K";
            std::cout << text;
            std::cout << "\0338";
            std::cout.flush();
            std::string temp;
            for(int i = 1; i < text.length(); i++){
                temp += text[i];
            }
            temp += text[0];
            text = temp;
            sleep_for(milliseconds(time));
        }
    }
    void setText(std::string newtext){
        text = newtext;
    }
    void startMarquee(){
        if(running) return; 
        running = true;

        std::cout << "\n";
        std::cout << text << "          ";
        std::cout << "\n";
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
    std::string newText;
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    std::string rest;
    std::getline(iss, rest);
    if(!rest.empty() && rest[0] == ' ') rest.erase(0,1);

    if(cmd.empty()){
        std::cout << "Enter command:" << std::flush;
        return;
    }else if(cmd == "help"){
        helpFunction();
        std::cout << "Enter command:" << std::flush;
    }else if(cmd == "start_marquee"){
        m.startMarquee();
        std::cout << "Enter command:" << std::flush;
    }else if(cmd == "stop_marquee"){
        m.stopMarquee();
        std::cout << "Enter command:" << std::flush;
        // function for stopping marquee
    }else if(cmd == "set_text"){
        if(rest.empty()){
            std::cout << "Usage: set_text <text>\n\n";
            std::cout << "Enter command:" << std::flush;
        }else{
            m.setText(rest);
            std::cout << "Marquee text set to \"" << rest << "\"\n\n";
            std::cout << "Enter command:" << std::flush;
        }
    }else if(cmd == "set_speed"){
        if(rest.empty()){
            std::cout << "Usage: set_speed <ms>\n\n";
            std::cout << "Enter command:" << std::flush;
            return;
        }else{
            int ms = std::stoi(rest);
            m.setTime(ms);
            std::cout << "Marquee speed set to " << ms << "ms\n\n";
            std::cout << "Enter command:" << std::flush;
        }
    }else if(cmd == "exit"){
        m.setExit();
    }else{
        std::cout << "Unknown command '" << cmd << "'. Type 'help' for a list of commands.\n\n";
        std::cout << "Enter command:" << std::flush;
    }
}

int main(){
    Marquee m;
    std::string command;
    printHeader();
    std::cout<< "Enter command:";
    while(!m.exit){
        std::getline(std::cin, command);
        parseCommand(command, m);
    }
}