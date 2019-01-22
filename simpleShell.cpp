#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cctype>
#include <sys/stat.h> 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fstream>
#include <dirent.h>
#include <algorithm>

using namespace std;

vector<string> parse(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

int checkSymbol(string& cmd, string symbol){ //symbol can be |, <, >, space
    int first = cmd.find(symbol);
    if(first==std::string::npos){ //not found
        return -1;
    } else{
        if(symbol=="<" || symbol==">"){
            string newCmd = cmd.substr(first+1, cmd.length()-first-1);
            int second = newCmd.find(symbol);
            if(second!=std::string::npos){ //found second one
                perror("ERROR");
                exit(1);
            }
        }
        return first;
    }
}

char** convert(vector<string>& s) {//convert string to char*[]
    char** result = new char*[s.size()+1];
    for(int i = 0; i < s.size(); i++){
        char* converted = new char[s[i].length()+1];
        strcpy(converted,s[i].c_str());
        result[i] = converted;
    }
    result[s.size()] = 0 ;
    return result; 
}

void sigHandler(int sigNum){
}

string pwd(){
    char* cwd = getcwd( 0, 0 ) ; // **** microsoft specific ****
    string working_directory(cwd) ;
    free(cwd) ;
    return working_directory ;
}

void execIRArgs(bool containsLessThan, bool containsNestGreaterThan, string newCmd, string outFileName){ //input redirector
    vector<string> parsedCmd;
    if(containsLessThan){
        parsedCmd = parse(newCmd, "<");
        //get cmd before < 
        string part1_cmd = parsedCmd[0];
        cout << "part1_cmd: "<< part1_cmd<<endl;
        vector<string> parsedSpacePart1 = parse(part1_cmd, " ");
        char** cmdArgvs = convert(parsedSpacePart1); //convert vector<string> to char** ended with 0
        char* cmdName = cmdArgvs[0];
        //get filename
        string part2_fileName = parsedCmd[1];
        part2_fileName.erase(remove(part2_fileName.begin(), part2_fileName.end(), ' '), part2_fileName.end()); //eliminate spaces
        char* fileName = new char[part2_fileName.length()+1];
        strcpy(fileName,part2_fileName.c_str());
        //file descriptor handling
        int fd = open(fileName, O_RDONLY);
        if(fd == -1){
            perror("ERROR");
            exit(1);
        }
        dup2(fd,STDIN_FILENO);
        close(fd);
        if(containsNestGreaterThan){
            //get file name
            char* fileName = new char[outFileName.length()+1];
            strcpy(fileName,outFileName.c_str());
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            int fd = open(fileName, O_CREAT|O_WRONLY|O_TRUNC, mode);
            if(fd == -1){
                
                exit(1);
            }
            dup2(fd,STDOUT_FILENO);
            close(fd);
        }
        int exe = execvp(cmdName, cmdArgvs);
        if(exe == -1){
            
            exit(1);
        }
    } else{
        parsedCmd = parse(newCmd, ">");
        //get cmd before >
        string part1_cmd = parsedCmd[0];
        vector<string> parsedSpacePart1 = parse(part1_cmd, " ");
        char** cmdArgvs = convert(parsedSpacePart1); //convert vector<string> to char** ended with 0
        char* cmdName = cmdArgvs[0];
        cout << "part1_cmd: "<< part1_cmd<<endl;
        //get file name
        char* fileName = new char[outFileName.length()+1];
        strcpy(fileName,outFileName.c_str());
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int fd = open(fileName, O_CREAT|O_WRONLY|O_TRUNC, mode);
        if(fd == -1){
            
            exit(1);
        }
        dup2(fd,STDOUT_FILENO);
        close(fd);
        int exe = execvp(cmdName, cmdArgvs);
        if(exe == -1){
            
            exit(1);
        }
        
    }
}

void init(bool prompt, string& cmd, bool& isBackground){
    if(prompt){
        cout<<"shell: ";
    }
    getline(cin,cmd);
    if(cin.eof()){
        cout<<endl;
        exit(1);
    }
    int andCount = 0;

    while(cmd[cmd.length()-1]==' ' || cmd[cmd.length()-1]=='&'){ //remove possible trailing spaces
        if(cmd[cmd.length()-1]=='&'){
            isBackground = true;
            andCount++;
        }
        cmd = cmd.substr(0,cmd.length()-1);
    }
    if(andCount>1){
        
    }
}

void outRedirector(string cmd, string newCmd, int index, bool containsLessThan, bool containsNestGreaterThan){
    int start = index;
    int fileNameLength = 0;
    while(start!=cmd.length()-1){ //starts from >
        start++;
        fileNameLength++;
    }
    string outFileName = cmd.substr(index+1,fileNameLength);
    outFileName.erase(remove(outFileName.begin(), outFileName.end(), ' '), outFileName.end()); //eliminate spaces
    execIRArgs(containsLessThan, containsNestGreaterThan, newCmd, outFileName);
}

void execArgs(string newCmd){
    vector<string> parsedCmd = parse(newCmd, " "); 
    char** cmdArgvs = convert(parsedCmd); //convert vector<string> to char** ended with 0
    int exe = execvp(cmdArgvs[0], cmdArgvs);
    if(exe == -1){
        perror("ERROR");
        exit(1);
    }
}

void execNoPipe(string cmd){
    int containsInput = checkSymbol(cmd,"<"); //check input redirector
    int index = 0;
    char curr = cmd[index];
    while(curr!='>' && index!=cmd.length()-1){
        index++;
        curr = cmd[index];
    }
    string newCmd;
    if (index==cmd.length()-1){ 
        newCmd = cmd.substr(0,index+1);
    } else{ //index = >
        newCmd = cmd.substr(0,index); //(position,length)
    }
    if(containsInput==-1){ //not found <   
        if(index==cmd.length()-1){
            execArgs(newCmd);
        } else{ //contains >, if cmd = ls > 3, then newCmd = ls
            int containsInput = checkSymbol(cmd,">"); //check outpur redirector valid
            outRedirector(cmd, newCmd, index, false, false);
        }
    } else{ //contains <
        if(index==cmd.length()-1){
            execIRArgs(true, false, newCmd, "stub"); //containsLessThan = true, scontainsNestGreaterThan = false, stub for outFileName, cannot use NULL for unknown bug
        } else{ //if(curr=='>')
            outRedirector(cmd, newCmd, index, true, true);
        }
    }
}

void execPipedArgs(vector<string> parsedPipe, int pipeCount){
    const int read = 0;
    const int write = 1;
    int status;
    int pipefds[2*pipeCount]; //2 for each pipe
    for(int i = 0; i < pipeCount; i++){
        if(pipe(pipefds + i*2) < 0) {
            
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0;i<parsedPipe.size();i++){ //# of cmd
        pid_t pid = fork();
        if(pid<0){ //fork failed
            
            exit(EXIT_FAILURE);
        } else if(pid==0){ //child process, check if still contains pipe
            if(i==0){
                if(close(pipefds[read])<0){
                    
                    exit(EXIT_FAILURE);
                }
                if(dup2(pipefds[write], STDOUT_FILENO)<0){
                    
                    exit(EXIT_FAILURE);
                }
            } else if(i==parsedPipe.size()-1){ 
                if(close(pipefds[2*(i-1)+write])<0){
                    
                    exit(EXIT_FAILURE);
                }
                if(dup2(pipefds[2*(i-1)+read], STDIN_FILENO)<0){
                    
                    exit(EXIT_FAILURE);
                }
            } else {
                if(close(pipefds[2*(i-1)+write])<0){
                    
                    exit(EXIT_FAILURE);
                }
                if(close(pipefds[2*i+read])<0){
                    
                    exit(EXIT_FAILURE);
                }
                if(dup2(pipefds[2*(i-1)+read], STDIN_FILENO)<0){ //read from previous pipe
                    
                    exit(EXIT_FAILURE);
                }
                if(dup2(pipefds[2*i+write], STDOUT_FILENO)<0){ //write to next pipe
                    
                    exit(EXIT_FAILURE);
                }
            }
            cout<<"parsedPipe: "<<parsedPipe[i]<<endl;
            execNoPipe(parsedPipe[i]);
        } else{
        }
    }
    for(int i = 0; i < 2*pipeCount; i++){
        if(close(pipefds[i])<0){
            
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < pipeCount + 1; i++){
        wait(&status);
    }
}

int main(int argc, char *argv[]){
    bool prompt = true;
    if(argc>1 && strcmp(argv[1],"-n")==0){ //cmd contains -n
        prompt = false;
    }
    do{  
        string cmd;
        bool isBackground = false;
        init(prompt, cmd, isBackground);
        pid_t pid = fork();
        if(pid<0){
            
            exit(1);
        } else if(pid==0){ //child process, execute the cmd
            vector<string> parsedPipe = parse(cmd,"|"); //Check if contains pipe
            if(parsedPipe.size()>1){
                int pipeCount = parsedPipe.size()-1;
                execPipedArgs(parsedPipe, pipeCount);
            } else {
                execNoPipe(cmd);
            }    
        } else{ //parent process
            if(!isBackground){ // no &
                while (true) {
                    int status;
                    pid_t done = waitpid(-1,&status, 0);
                    if (done < 0) { //wait for child process
                        if (errno == ECHILD) break; // no more child processes
                    } else {
                        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                            // cerr << "pid " << done << " failed" << endl;
                            // 
                            int exit(1);
                        }
                    }
                }
            } else{ //run in background
                wait(NULL);
                signal(SIGCHLD,sigHandler);
            }
        }

    } while (true);
    return 0;
}