#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <deque>
#include <fstream>
#include <string>
#include <cstring>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include"commandUtils.h"
#include "Commands.h"
#include <sys/wait.h>
#include <unistd.h>
#include "Shell.h"




    Shell::Shell(){localHome=getLocalHome();loadHistory();};          // Constructor declaration
    Shell::~Shell(){saveHistory();};         // Destructor declaration
    // Other member functions



    void Shell:: updatePrompt() {
        currentDirectory = getCurrentDirectory();
        char hostname[1024];
        gethostname(hostname, sizeof(hostname));
        std::string relativePath = (currentDirectory.find(localHome) == 0)
            ? "~" + currentDirectory.substr(localHome.length())
            : currentDirectory;
        std::cout << getenv("USER") << "@" << hostname << ":" << relativePath << "$ "<<std::flush;
    }

    void Shell::loadHistory() {
        std::ifstream infile(historyFile);
        std::string line;
        while (std::getline(infile, line)) {
          
            if (!line.empty()) {
                commandHistory.push_back(line);
            }
        }
        infile.close();
        historyIndex = commandHistory.size();
    }

    void Shell::historyCommand(std::vector<std::string> &args){
        loadHistory();
        for(auto cmd:commandHistory){
            std::cout<<cmd<<std::endl;
        }

    }

    void Shell::saveHistory() {
        std::ofstream outfile(historyFile);
        for (const auto &cmd : commandHistory) {
            outfile << cmd << std::endl;
        }
        outfile.close();
    }


   bool Shell::handleBuiltInCommands(std::vector<std::string>& args,Shell &shell) {
    if (args.empty()) return false;

    const std::string& command = args[0];
    if (command == "cd") {
        std::cerr<<"reached handleBuiltins with args "<<args[1];
        cdCommand cd;
        cd.execute(args);
     } else if (command == "ls") {
         LsCommand ls;
         ls.execute(args);
    } else if (command == "pinfo") {
         pinfo(args);
    } else if (command == "history") {
        shell.historyCommand(args);
    } else if (command == "search") {
       SearchCommand srch;
       srch.execute(args);
    // } else if (command == "echo") {
    //     executeEcho(args);
    } 
    else {
        return false;  // Not a built-in command
    }
    return true;  // Command was handled as a built-in
}
int Shell::runCommand(std::vector<std::string> commands) {
    int pipe_fd[2];
    int in_fd = 0;  // This will track the input for the current command (initially stdin)
    int out_fd = 1; // Standard output default
    int append_fd = 1;
    
    bool isBackgroundP = false;
    std::vector<std::string> command;
    
    if (!commands.empty()) {
        for (auto it : commands) {
            if (it != "|") {
                if (it == "&") {
                    isBackgroundP = true;
                } else {
                    command.push_back(it);
                }
            } else {
                // Create a pipe
                if (pipe(pipe_fd) == -1) {
                    std::cerr << "Error creating pipe" << std::endl;
                    return 1;
                }
                if (isBackgroundP == true) {  // | cannot occur after &
                    std::cerr << "myShell: syntax error near unexpected token `|'" <<std::endl;
                    return 1;
                }

                if (!command.empty()) {
                    int pid = fork();
                    if (pid == -1) {
                        std::cerr << "Error forking" <<std::endl;
                        return 1;
                    } else if (pid == 0) {  // Child process
                        handleIndirections(command, in_fd, out_fd, append_fd); // Handle indirections
                        close(pipe_fd[0]);  // Close unused read end
                        dup2(in_fd, STDIN_FILENO);  // Duplicate the input from the last command
                        dup2(pipe_fd[1], STDOUT_FILENO);  // Redirect output to pipe's write end
                        close(pipe_fd[1]);  // Close original write end

                        // Check if it's a built-in command
                        if (!handleBuiltInCommands(command, *this)) {
                            std::vector<char*> args;
                            for (auto& cmd : command) {
                                args.push_back(&cmd[0]); 
                            }
                            args.push_back(nullptr);

                            execvp(args[0], args.data());  // Execute command
                            perror("execvp failed");
                        }
                        exit(1); // Ensure child exits after executing the command
                    } else {  // Parent process
                        waitpid(pid, NULL, 0);  // Wait for the child process to finish
                        close(pipe_fd[1]);  // Close write end of the pipe
                        in_fd = pipe_fd[0];  // Save the read end for the next command
                        command.clear();  // Clear the command vector for the next command
                    }
                }
            }
        }

        // For background process or any remaining command
        if (!command.empty()) {
            handleIndirections(command, in_fd, out_fd, append_fd);

            int back_pid = 0;
            if (isBackgroundP) {
                back_pid = fork();
            }

            if (back_pid == 0 || !isBackgroundP) { // Child process or foreground process
                dup2(in_fd, STDIN_FILENO);  // Input from the last command
                close(in_fd);

                if (!handleBuiltInCommands(command, *this)) {
                    std::vector<char*> args;
                    for (auto& cmd : command) {
                        args.push_back(&cmd[0]);
                    }
                    args.push_back(nullptr);

                    execvp(args[0], args.data());  // Execute command
                    perror("execvp failed");
                    exit(1); // Ensure child exits after executing the command
                }
                exit(0);  // Ensure the child process exits after execution
            } else if (back_pid > 0) { // Parent process in background mode
                if (isBackgroundP) std::cout << back_pid <<std::endl;
            }
        }
    }

    return 0;
}


    void Shell::run() {
        termios orig_termios;

        // Get original terminal attributes
        tcgetattr(STDIN_FILENO, &orig_termios);

        // Enable raw mode
        enableRawMode(orig_termios);

    while (true) {
        std::string input;

        updatePrompt();
        std::string currentInput;
        historyIndex = commandHistory.size(); // Reset history index
        
        
        std::string buffer;  // Buffer for unfinished commands

        std::cout << "\033[?12h\033[?25h" << std::flush;
        char c;

        while (read(0,&c,1)==1) {
             
            if (c == '\n') {
                std::cout << std::endl;
                if (!currentInput.empty()) {
                    // Add to history and reset index
                    if (commandHistory.size() == historyLimit) {
                        commandHistory.pop_front();
                    }
                    commandHistory.push_back(currentInput);
                    historyIndex = commandHistory.size();
                }
                buffer.clear();  // Clear the buffer as the command is now completed
                break;
            } else if (c == 127) {  // Handle backspace
                if (!currentInput.empty()) {
                    currentInput.pop_back();
                    std::cout << "\b \b" << std::flush;  // Flush after backspace
                }
            } else if (c == 27) {  // Handle escape sequences (arrows)
                char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '[') {
                    if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                        if (seq[1] == 'A') {  // Up arrow
                            if (historyIndex > 0) {
                                if (historyIndex == commandHistory.size()) {
                                    buffer = currentInput;  // Save unfinished command in buffer
                                }
                                historyIndex--;
                                // Clear current line
                                std::cout << "\r\033[K" << std::flush;
                                updatePrompt();
                                currentInput = commandHistory[historyIndex];
                                std::cout << currentInput << std::flush;
                            }
                        } else if (seq[1] == 'B') {  // Down arrow
                            if (historyIndex < commandHistory.size()) {
                                historyIndex++;
                                // Clear current line
                                std::cout << "\r\033[K" << std::flush;
                                updatePrompt();
                                if (historyIndex < commandHistory.size()) {
                                    currentInput = commandHistory[historyIndex];
                                } else {
                                    currentInput = buffer;  // Restore unfinished command from buffer
                                }
                                std::cout << currentInput << std::flush;
                            }
                        }
                    }
                }
            } else {
                currentInput += c;
                std::cout << c << std::flush;  // Flush after every character
            }
        }

        input = currentInput;
  

        if (input.empty()){
           std::cin.ignore();
           continue;
        }   // Skip processing if input is empty
        if(input=="exit"){
            break;
        }
        // Tokenization and further processing of the input
        std::vector<std::string> tokenizedCommands = tokenize(input);
        std::vector<std::string> indiCommand;

        for (auto it : tokenizedCommands) {
            if (it != ";") {
                indiCommand.push_back(it);
            } else {
                if (!indiCommand.empty()) {
                    runCommand(indiCommand);
                    indiCommand.clear();
                }
            }
        }

        if (!indiCommand.empty()) {
            runCommand(indiCommand);
        }

        // Restore original terminal attributes before exit
        
    }
    disableRawMode(orig_termios);
}



int main(){
Shell* sh=new Shell();
   
    sh->run();


    return 0;
} 