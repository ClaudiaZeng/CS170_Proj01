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

int checkSymbol(string& cmd, char symbol){ //symbol can be |, <, >, space
    int first = cmd.find(symbol);
    if(first==std::string::npos){ //not found
        return -1;
    } else{
        if(symbol=='<' || symbol=='>'){
            vector<string> parsedCmd = parse(cmd,to_string(symbol));
            if(parsedCmd.size()>2){ //only the first can have <, and the last can have >
                perror("ERROR: ");
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

string removeSpaces(string str) { 
    int i = 0, j = 0; 
    while (str[i]) { 
        if (str[i] != ' ') 
           str[j++] = str[i]; 
           i++; 
    } 
    str[j] = '\0'; 
    return str; 
}

void sigHandler(int sigNum)
{
}

int main(int argc, char *argv[]){
    bool prompt = true;
    if(argc>1 && strcmp(argv[1],"-n")==0){ //cmd contains -n
        prompt = false;
    }
    do{
        if(prompt){
            cout<<"shell: ";
        }
        string cmd;
        getline(cin,cmd);
        bool isBackground = false;
        if(cmd[cmd.length()-1]=='&'){
            isBackground = true;
        }
        pid_t pid = fork();
        if(pid==0){ //child process, execute the cmd
            int containsInput = checkSymbol(cmd,'<'); //check input redirector
            //find the correct substring of cmd to execute i.e. cat (<) 1| sort -> cat (<) 1
            int index = 0;
            char curr = cmd[index];
            while(curr!='|' && curr!='>'&& curr!='&'){
                //TODO!!! pipe or output redirector handling
                index++;
                curr = cmd[index];
            }
            string newCmd = cmd.substr(0,index); //copy until reaching another symbol. i.e. cat (<) 1 & -> cat (<) 1 

            if(containsInput==-1){ //not found <
                vector<string> parsedCmd = parse(newCmd, " "); 
                char** cmdArgvs = convert(parsedCmd); //convert vector<string> to char** ended with 0
                execvp(cmdArgvs[0], cmdArgvs);
                perror("ERROR: ");
                exit(1);
            } else{ //contains <
                vector<string> parsedCmd = parse(newCmd, "<");
                //get cmd before < 
                string part1_cmd = parsedCmd[0];
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
                    perror("ERROR: ");
                    exit(1);
                }
                dup2(fd,STDIN_FILENO);
                close(fd);
                execvp(cmdName, cmdArgvs);
                perror("ERROR: ");
                exit(1);
            }
        } else{ //parent process
            if(!isBackground){ // no &
                while (true) {
                    int status;
                    // waitpid(-1,&status, 0);
                    // exit(status);
                    pid_t done = waitpid(-1,&status, 0);
                    if (done < 0) { //wait for child process
                        if (errno == ECHILD) break; // no more child processes
                    } else {
                        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                            cerr << "pid " << done << " failed" << endl;
                            int exit(1);
                        }
                    }
                }
            } else{ //run in background
                signal(SIGCHLD,sigHandler);
            }
        }

    } while (true);
    return 0;
}