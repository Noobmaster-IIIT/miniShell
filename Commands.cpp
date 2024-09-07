#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <cstring> 

#include"commandUtils.h"
#include"Commands.h"
#include"Shell.h"











int cdCommand::execute(std::vector<std::string>& args){
    
    if(args.size()>2){
        std::cerr<<"cd : Too many arguments!";
        return 1;
    }
    if(args.size()==1){chdir(getLocalHome().c_str());return 0;}
    std::string arg=args[1];


    

    std::vector<std::string> components;
    int i=0,j=0;
    
    int found_atleast1=0;
    while((j=arg.find('/',i))!=std::string::npos){
        components.push_back(arg.substr(i,j-i));
        i=j+1;
        found_atleast1++;
     

    }
    if(found_atleast1==0){
        //remove whitespaces if any and push into component
        int i=0;
        while(arg[i]==' ' & i<arg.size())i++;
        components.push_back(arg.substr(i));
        std::cout<<arg.substr(i);
    }
    else components.push_back(arg.substr(i));//remainder

    if(components.size()==1 && components[0]==""){
        
      //get local home                             //   <-----
    }

    std::string currentPath=getCurrentDirectory();

     if (arg[0] == '~') {
        std::string username = arg.substr(1); 

        if (username.empty()) {
            // 
            char* home = getenv("HOME"); //  change to globalLocalhome later!
            if (home) {
                
                currentPath = std::string(home);
            } else {
                std::cerr << "Error: HOME environment variable is not set." << std::endl;
                return 1;
            }
        } else {
            //Find user's home directory
            struct passwd* pw = getpwnam(username.c_str());
            if (pw) {
                
                currentPath = std::string(pw->pw_dir);
            } else {
                std::cerr << "cd: No such user: " << username << std::endl;
                return 1;
            }
        }
    }
    else{
    for(auto& component:components){

        //if(component==".") just ignore update to currentPath
         
        if(component==".."){
            //go one step back
   
                size_t pos=currentPath.find_last_of('/');
                
                currentPath=(pos==std::string::npos)?getCurrentDirectory():currentPath.substr(0,pos);
                }
            else if(component!="."){
                //apppend dir name to current path
 
                currentPath+="/"+component;
                
            }
        }
    }
    
    //perform directory change
    std::cout<<currentPath<<std::endl;
    if(chdir(currentPath.c_str())==0){
        std::cout<<"Directory changed to "<<getCurrentDirectory(); //remove cout later
        return 0;}
    else{
        std::cerr<<"cd: "<<currentPath<<" No such Directory or path";
    }
    return 1;
}




std::vector<std::string> EchoCommand::parseInput(std::string& inputLine) {
    std::vector<std::string> tokens;
    std::istringstream stream(inputLine);
    std::string token;
    bool insideQuotes = false;
    char quoteChar = '\0';

    while (stream) {
        char c = stream.get();
        if (c == '"' || c == '\'') {
              if (insideQuotes) {
                if(c==quoteChar)insideQuotes = false; //end quotes
                else token+=c;
              
               
            } 
            else if (!insideQuotes) {
                insideQuotes = true; // Start quotes
                quoteChar = c;
               
                
            }
        }
         else if (c==' ' && !insideQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else if (c != EOF) {  //push c into token
            token += c;
        }
    }
    
    if (!token.empty()) {
        tokens.push_back(token); // Add  last token
    }

    return tokens;
}

int EchoCommand::execute(std::vector<std::string> &inputLine) {
   
    std::vector<std::string> output;



    // Loop through the arguments and print each one
    for (int i = 1; i < inputLine.size(); ++i) {
        output=parseInput(inputLine[i]);
        for(auto it:output){
            std::cout<<it;
        }
        std::cout<<" ";
    }
    std::cout<< std::endl; // End with a newline

    // Output the result
    

    return 0; 
}


  int LsCommand::execute( std::vector<std::string>& args){



    try {
        auto option_args = parseArguments(args);

        int option = option_args.first;
        bool all=false,longFormat=false;

        if(option==1)all=true;
        else if(option==2)longFormat=true;
        else if(option==3) {
            all=true;
            longFormat=true;
        }
        else std::cerr<<"Invalid option";
        std::vector<std::string> directories = option_args.second;

        // If no directories are provided, use current directory
        if (directories.empty()) {
            directories.push_back(".");
        }

        // Iterate through each directory and list files
        for ( auto& dir : directories) {
            listDirectory(dir,all,longFormat);
        }

        return 0; // Success
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1; // Error occurred, return non-zero status
    }

}

std::string LsCommand::resolvePath( std::string& path)
{
    if (path == "..") {
        return getParentDirectory();
    } else if (path == ".") {
        return getCurrentDirectory();
    } else if (path == "~") {
        return getHomeDirectory(); //NEED TO CHANGE THIS TO LOCAL HOME
    } else {
        return path; // Return the directory as is
    }
      
}

std::string LsCommand:: getParentDirectory(){

    char cwd[1000];
    if(getcwd(cwd,sizeof(cwd))!=nullptr){
        std::string currentDir(cwd);
        int pos=currentDir.find_last_of('/');
        return currentDir.substr(0,pos); //handle error later
    }
    return ".";

}

std::string LsCommand::getCurrentDirectory(){
    char cwd[1000];
    if(getcwd(cwd,sizeof(cwd))!=nullptr){
        return std::string(cwd);
    }
    return ".";

}


std::string LsCommand::getHomeDirectory(){
  
  char* home=getenv("HOME"); // use localhome()
  if(home){
    return std::string(home);
  }
  return ".";
}

int LsCommand::atoo(std::string& arg){

    if (arg.length() > 3) {
        throw std::invalid_argument("Error: Option length is greater than 3.");
    }
    if (arg == "-a") return 1;
    if (arg == "-l") return 2;
    if (arg == "-al" || arg=="la") return 3;
    throw std::invalid_argument("Error: Invalid option provided.");
}

    
    


std::pair<int,std::vector<std::string>> LsCommand::parseArguments( std::vector<std::string>& args){
  
  int option=0; //default
  args.erase(args.begin());
  std::vector<std::string> dirs;
  for(auto arg:args){
    if(arg[0]=='-')//option
    {
        
        option|=atoo(arg);
        
    }
    
  
  else{
    dirs.push_back(resolvePath(arg));
  }
  }
  std::cout<<option;
  std::cout<<dirs[0];

return {option, dirs};
}

void LsCommand::listDirectory( std::string& path, bool all, bool longFormat){

    

    DIR* dir=opendir(path.c_str());
    if(!dir)std::cerr<<"Error: ls cannot access "<<path<<" No such file or directory";

    struct dirent* entry;

    while((entry=readdir(dir))!=nullptr){
        std::string fileName = entry->d_name;
        
        
        //skip hidden if !-a

        if(!all && fileName[0]=='.'){ //hidden files start with '.'
            continue;
        }

        std::string fullPath = path+"/"+fileName;

        struct stat filestat;

        if(stat(fullPath.c_str(),&filestat)==0){
            listFiles(fullPath,all,longFormat);
        }
    }
    closedir(dir);
}

void LsCommand::listFiles( std::string& path, bool all, bool longFormat)
{
    struct stat fileStat;
    
    if(stat(path.c_str(),&fileStat)!=0){

      std::cerr<<"ls: cannot access "<<path<<" : No such file or directory"<<std::endl;
    }

      if(longFormat){
        
        printFileInfo(path,fileStat);
      }
      else{
        std::cout<<path.substr(path.find_last_of('/')+1)<<"\n";
      }
    }


void LsCommand::printFileInfo( std::string& path, struct stat fileStat){
   
   
   bool type =S_ISDIR(fileStat.st_mode);
   std::cout<<type?'d':'-';
   //permissions
   std::vector<int> permissions={  
        S_IRUSR, S_IWUSR, S_IXUSR,  // User permissions
        S_IRGRP, S_IWGRP, S_IXGRP,  // Group permissions
        S_IROTH, S_IWOTH, S_IXOTH   // Others permissions
    };
    std::vector<char> symbols={'r','w','x'};
    for(int i=0;i<permissions.size();++i){
        std::cout<<((fileStat.st_mode & permissions[i])?symbols[i%3]:'-');
    }
    //links

    std::cout<<' '<<std::setw(2)<<fileStat.st_nlink;
    // Owner and group
    struct passwd* pw = getpwuid(fileStat.st_uid);
    struct group* gr = getgrgid(fileStat.st_gid);
    std::cout << ' ' << (pw ? pw->pw_name : std::to_string(fileStat.st_uid));
    std::cout << ' ' << (gr ? gr->gr_name : std::to_string(fileStat.st_gid));

    // File size
    std::cout << ' ' << std::setw(6) << fileStat.st_size;

    // Last modified time
    struct tm* timeinfo = localtime(&fileStat.st_mtime);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%b %d %H:%M", timeinfo);
    std::cout << ' ' << timeBuf;

    // File name
    std::cout << ' ' << path.substr(path.find_last_of('/') + 1) << "\n";
   
   

}

void pinfo(std::vector<std::string> &args) {
    if(args.size()>2)std::cerr<<"Too many arguments"<<std::endl;
    
     pid_t pid;
    if (args.size() == 1) {
        pid = getpid();  // Default to current process if no PID is provided
    } else {
        try {
            pid = std::stoi(args[1]);
        } catch (const std::invalid_argument&) {
            std::cerr << "Error: Invalid PID. Please provide a valid process ID." << std::endl;
            return;
        } catch (const std::out_of_range&) {
            std::cerr << "Error: PID out of range. Please provide a valid process ID." << std::endl;
            return;
        }
    }

    std::string proc_dir = "/proc/" + std::to_string(pid);
    std::string status_file = proc_dir + "/status";
    std::string stat_file = proc_dir + "/stat";
    std::string exe_link = proc_dir + "/exe";

    // Open and read the status file
    int status_fd = open(status_file.c_str(), O_RDONLY);
    if (status_fd == -1) {
        std::cerr << "Error: Unable to open status file for process " << pid << std::endl;
        return;
    }
    
    char buffer[4096];
    ssize_t bytes_read = read(status_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        std::cerr << "Error: Unable to read status file for process " << pid << std::endl;
        close(status_fd);
        return;
    }
    buffer[bytes_read] = '\0';  //add null at end
    std::string status_content(buffer);
    close(status_fd);

    // extract p status and vm info
    std::istringstream status_stream(status_content);
    std::string line, process_status;
    unsigned long int virtual_memory = 0;
    while (std::getline(status_stream, line)) {
        if (line.find("State:") == 0) {
            process_status = line.substr(7);
        }
        if (line.find("VmSize:") == 0) {
            std::istringstream iss(line);
            std::string vm_label;
            iss >> vm_label >> virtual_memory;
        }
    }

    // Open and read stat file
    int stat_fd = open(stat_file.c_str(), O_RDONLY);
    if (stat_fd == -1) {
        std::cerr << "Error: Unable to open stat file for process " << pid << std::endl;
        return;
    }

    bytes_read = read(stat_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        std::cerr << "Error: Unable to read stat file for process " << pid << std::endl;
        close(stat_fd);
        return;
    }
    buffer[bytes_read] = '\0'; 
    std::string stat_content(buffer);
    close(stat_fd);

    // Parse the foreground/background status from the stat file
    std::istringstream stat_stream(stat_content);
    std::string token;
    for (int i = 0; i < 3; i++) stat_stream >> token; // Skip PID, comm, state
    pid_t sid, pgrp, tpgid;
    for (int i = 0; i < 4; i++) stat_stream >> token; // Skip ppid, pgrp, sid, tpgid
    pgrp = std::stoi(token);
    stat_stream >> sid;
    stat_stream >> tpgid;

    bool is_foreground = (pgrp == sid && tpgid == sid);

    // Get executable path
    char exe_path[1024];
    ssize_t len = readlink(exe_link.c_str(), exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    } else {
        std::cerr << "Error: Unable to retrieve executable path for process " << pid << std::endl;
        return;
    }

    // Output
    std::cout << "pid: " << pid << std::endl;
    std::cout << "Process Status: " << process_status;
    if (is_foreground) {
        std::cout << " + (Foreground)";
    }
    std::cout << std::endl;
    std::cout << "Memory: " << virtual_memory << " kB (Virtual memory)" << std::endl;
    std::cout << "Executable Path: " << exe_path << std::endl;
}


int  pwdCommand::execute(std::vector<std::string> args){

    char cwd[1000];
    if(getcwd(cwd,sizeof(cwd))!=nullptr){
        std::cout<<std::string(cwd)<<"\n";
        
        return 0;
    }
    return 1;
    }



    int SearchCommand::execute(std::vector<std::string> args) {
        if(args.size()>2){
            std::cout<<"Search: Too many arguments";
            return 1;
        }else{
        std::string name=args[1];
        std::string ans=searchRecursive(".", name)?"True":"False";
        std::cout<<ans<<std::endl;; // Start searching from the current directory
        return 0;
        }
    }
    bool SearchCommand::searchRecursive(const std::string& current_dir, const std::string& name) {
        DIR* dir = opendir(current_dir.c_str());
        if (!dir) {
            std::cerr << "Could not open directory: " << current_dir << std::endl;
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string path = current_dir + "/" + entry->d_name;

       
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Check if the entry matches the name we're searching for
            if (name == entry->d_name) {
                std::cout << "Found: " << path << std::endl;
                closedir(dir);
                return true;
            }

            // search recursively within it
            struct stat info;
            if (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
                if (searchRecursive(path, name)) {
                    closedir(dir);
                    return true;
                }
            }
        }

        closedir(dir);
        return false;
    }
