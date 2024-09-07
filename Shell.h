#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include "Commands.h"  // Include your command headers
#include "commandUtils.h"  // Include your utility functions

class Shell {
public:
    Shell();
    ~Shell();
    void run();
    
private:
    void updatePrompt();
    void loadHistory();
    void saveHistory();
    void historyCommand(std::vector<std::string>& args);
    bool handleBuiltInCommands(std::vector<std::string>& args, Shell &shell);
    int runCommand(std::vector<std::string> commands);
    
    
    
    
   

    std::string currentDirectory;
    std::string localHome;
    std::deque<std::string> commandHistory;
    std::string historyFile = "myshell_history.txt";
    size_t historyLimit = 20;
    size_t historyIndex;
};

#endif // SHELL_H
