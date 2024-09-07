#include <vector>
#include <string>
#include<termios.h>
#include <iomanip>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include<iostream>
#include"Commands.h"
#include <vector>
#include <string>
#include"Shell.h"
#include"commandUtils.h"


void handleIndirections(std::vector<std::string> &args, int &in_fd, int &out_fd, int &append_fd) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") {
            out_fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("open");
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            i--; // Adjust the loop after erasing
        } else if (args[i] == ">>") {
            append_fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (append_fd < 0) {
                perror("open");
            }
            dup2(append_fd, STDOUT_FILENO);
            close(append_fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            i--; // Adjust the loop after erasing
        } else if (args[i] == "<") {
            in_fd = open(args[i + 1].c_str(), O_RDONLY);
            if (in_fd < 0) {
                perror("open");
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            i--; // Adjust the loop after erasing
        }
    }
}



    std::string getLocalHome() {
        char cwd[1000];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return std::string(cwd);
        }
        return "";
    }
    
        std::string getCurrentDirectory() {
        char cwd[1000];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return std::string(cwd);
        }
        return getLocalHome();
    }
    
    void enableRawMode(termios &orig_termios) {
        termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON | ISIG);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    
    void disableRawMode(termios &orig_termios) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }

    
 std::vector<std::string> tokenize(std::string& input){
    std::vector<std::string> tokens;
    std::string token;
    bool insideQuotes = false;
    char quote = '\0';  // To track which type of quotes are being used

    for (size_t i = 0; i <input.size(); ++i) {
        char c = input[i];

        if (c == '"' || c == '\'') {
            if (insideQuotes && c == quote) {
                insideQuotes = false;
                quote = '\0';
            } else if (!insideQuotes) {
                insideQuotes = true;
                quote = c;
            }
        }

        if (c == '|' || c == ';' || c == '<' || c == '>'||c=='&') {
            if (!insideQuotes) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }

                // Handle the `<<` case
                if (c == '<' && i + 1 < input.size() && input[i + 1] == '<') {
                    tokens.push_back("<<");
                    ++i;  // Skip the next character since it's part of the `<<` token
                }
                // Handle the `>>` case
                else if (c == '>' && i + 1 < input.size() && input[i + 1] == '>') {
                    tokens.push_back(">>");
                    ++i;  // Skip the next character since it's part of the `>>` token
                } else {
                    tokens.push_back(std::string(1, c));
                }
            } else {
                token += c;
            }
        } else if(!insideQuotes && c==' '){
              
              if(!token.empty()){ tokens.push_back(token);token.erase();}
        }
              
        else token += c;
        
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }
    

    return tokens;
}

