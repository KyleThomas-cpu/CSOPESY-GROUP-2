#include <iostream>
#include <sstream>

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
    std::cout << "  exit            - exits the console\n";
}


void parseCommand(const std::string& line){
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    std::string rest;
    std::getline(iss, rest);
    if(!rest.empty() && rest[0] == ' ') rest.erase(0,1);

    if(cmd.empty()){
        return;
    }else if(cmd == "help"){
        helpFunction();
    }else if(cmd == "start_marquee"){
        // function for starting marquee animation
    }else if(cmd == "stop_marquee"){
        // function for stopping marquee
    }else if(cmd == "set_text"){
        // function for setting display text
    }else if(cmd == "set_speed"){
        // function for setting marquee refresh rate
    }else if(cmd == "exit"){
        // handled in main
    }else{
        std::cout << "Unknown command '" << cmd << "'. Type 'help' for a list of commands.\n";
    }
}



int main(){
    printHeader();


}