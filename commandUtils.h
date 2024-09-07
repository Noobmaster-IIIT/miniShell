
#ifndef COMMANDUTILS_H
#define COMMANDUTILS_H

#include <vector>
#include <string>
#include<termios.h>
#include <iomanip>


void handleIndirections(std::vector<std::string> &args, int &in_fd, int &out_fd, int &append_fd);
std::string getLocalHome();
std::string getCurrentDirectory();
void enableRawMode(termios &orig_termios);
void disableRawMode(termios &orig_termios);
std::vector<std::string> tokenize(std::string& input);
 

#endif // COMMAND_UTILS_H
