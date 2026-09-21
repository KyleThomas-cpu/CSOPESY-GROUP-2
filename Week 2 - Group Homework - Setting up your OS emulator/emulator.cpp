#include <iostream>

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
        << "Type 'exit' to quit, 'clear' to clear the screen\n\n** IMPORTANT: Type 'initialize' to load config and start system **\n";
    cout << RESET;
}

int main() {
    string x;
    start();

    while(true){
        cout << "Enter a command: ";
        cin >> x;
        if(x == "initialize"){
            cout << "initialize command recognized. Doing initialize\n";
        } else if (x == "screen"){
            cout << "screen command recognized. Doing screen\n";
        } else if (x == "scheduler-start"){
            cout << "scheduler-start command recognized. Doing scheduler-start\n";
        } else if (x == "scheduler-stop"){
            cout << "scheduler-stop command recognized. Doing scheduler-stop\n";
        } else if (x == "report-util"){
            cout << "report-util command recognized. Doing report-util\n";
        } else if (x == "clear"){
            system("cls");
            start();
        } else if (x == "exit"){
            break;
        }  else { 
            cout << "Wrong input\n";
        }
    }
    return 0;
}