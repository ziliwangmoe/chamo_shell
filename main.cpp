#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define tempFileName "pipeFile.txt"

using namespace std;

int chamo_execute(string appName, vector<string> args) {
	int i;

	if (appName == "") {
		return 1;
	}
	return 1;
}



int execute(vector<string> args, string inputFile="", string outFile="", bool isBackstage=false){
	pid_t pid;
	int status;
	char **pArgs = new char*[args.size()+1];
	for (int i = 0; i < args.size(); i++) {
		pArgs[i] = new char[args.size()+1];
		for (int j = 0; j < args[i].size(); j++) {
			pArgs[i][j] = args[i][j];
		}
		pArgs[i][args[i].size()]='\0';
	}
	pArgs[args.size()]=NULL;

	if(isBackstage){
		pid = fork();
		if (pid < 0) {
			cout << "something wrong!!" << endl;
			exit(EXIT_FAILURE);
		} else if (pid == 0) {
			if (setsid() < 0)
				exit(EXIT_FAILURE);
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			pid = fork();
			if (pid < 0) {
				cout << "something wrong!!" << endl;
				exit(EXIT_FAILURE);
			} else if (pid == 0) {
				umask(0);
				chdir("/");
				if (execvp(args[0].c_str(), pArgs) == -1) {
					cout << "something wrong!!" << endl;
				}
				exit(EXIT_FAILURE);

			} else {
				exit(EXIT_SUCCESS);
			}
		} else {
			do {
				waitpid(pid, &status, WUNTRACED);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}else{
		pid = fork();
		if (pid == 0) {
			mode_t mode = S_IRWXU;
			if (inputFile != "") {
				int fd0 = open(inputFile.c_str(), O_RDONLY, mode);
				dup2(fd0, STDIN_FILENO);
				close(fd0);
			}

			if (outFile !="" ) {
				int fd1 = creat(outFile.c_str(), mode);
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}
			if (execvp(args[0].c_str(), pArgs) == -1) {
				cout << "something wrong!!" << endl;
			}
			exit(EXIT_FAILURE);
		} else if (pid < 0) {
			cout << "something wrong!!" << endl;
		} else {
			do {
				waitpid(pid, &status, WUNTRACED);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}

	return 1;
}

void findFirstRepalcement(int &start, int &end, string str){
	bool findArrow=false;
	bool findReplaceStarte= false;
	bool findFullReplacement = false;
	for(int i=0; i<str.size();i++){
		if(str[i]=='<'){
			findArrow= true;
		}else{
			if(str[i]=='('){
				if(findArrow == true){
					start = i;
				}else{
					findArrow= false;
				}
			}else if(str[i]==')'){
				end = i;
				findFullReplacement = true;
			}
		}
		if(findFullReplacement){
			break;
		}
	}
}

string checkReplacement(string str){
	int start = 0;
	int end = 0;
	string reStr="";
	findFirstRepalcement(start, end, str);
	if(start != end){
		reStr = str.substr(start, end- start);
	}else{
		reStr= "";
	}
	return reStr;
}

string replaceStringWithFileName(string str, string fileName){
	int start = 0;
	int end = 0;
	findFirstRepalcement(start, end, str);
	if(start != end){
		str.replace(start-2, end- start+1, fileName);
	}
	return str;
}

bool checkIsBackstage(string &commandName){
	if (commandName[commandName.size() - 1] == '&') {
		commandName.erase(commandName.size() - 1, 1);
		return true;
	}else{
		return false;
	}
}

int execute_string_no_plancement(string &str, string outFile, string &nextInputFileName, string inputFile){
	vector<string> args;
	int argCount = 0;
	args.push_back("");
	string _outFile = outFile;
	string _inputFile = inputFile;
	bool readFullName = false;
	bool readInputFileName = false;
	bool readOutFileName = false;
	bool moreCommand= false;
	int commandLength = 0;
	for (int i=0;i<str.size();i++){
		if(str[i]!='>' && str[i]!='<' && str[i]!='|'){
			if(!readFullName){ //is still reading the name
				if(str[i]==' '){ // first space, change state to read args
					readFullName=true;
					argCount++;
					args.push_back("");
				}else{ //still command name
					args[0].push_back(str[i]);
				}
			}else if (readInputFileName){
				_inputFile.push_back(str[i]);
			}else if (readOutFileName){
				_outFile.push_back(str[i]);
			}else{//already in args reading state
				if(str[i]==' '){ //means there are more arg
					argCount++;
					args.push_back("");
				}else{
					args[argCount].push_back(str[i]);
				}
			}
		}else if (str[i]=='>'){ // end of one command
			readFullName = true;
			readInputFileName= false;
			readOutFileName =true;
		}else if (str[i]=='<'){ // end of one command
			readFullName = true;
			readOutFileName = false;
			readInputFileName =true;
		}else if (str[i]=='|'){
			moreCommand = true;
			commandLength = i+1;
			if(_outFile == ""){
				_outFile= tempFileName;
				nextInputFileName = tempFileName;
			}else{
				nextInputFileName = _outFile;
			}
		}
	}
	if(moreCommand){
		str.erase(0, commandLength);
	}else{
		str="";
	}

	for (int i=0;i<args.size();i++){
		if(args[i]==""){
			args.erase(args.begin()+i,args.begin()+i+1);
		}
	}
	bool reBool = checkIsBackstage(args[0]);
	cout<<_inputFile<<"|"<<_outFile<<"|"<<args[0]<<"|"<<args[1]<<endl;
	return execute(args, _inputFile, _outFile, reBool);
}



int execute_string(string str, string outFile=""){
	static int fileNameIndex=0;
	string re = checkReplacement(str);
	while(re!= ""){
		char sbuffer [33];
		sprintf(sbuffer, "chamo_%d.txt", fileNameIndex);
		string fileName = sbuffer;
		fileNameIndex++;
		execute_string(re, fileName);
		str = replaceStringWithFileName(str, fileName);
		re = checkReplacement(str);
	}
	bool succAll= true;
	string nextInput;
	while(str!=""){
		string _nextInput;
		if(execute_string_no_plancement(str, outFile, _nextInput, nextInput)!=1){
			cout<<"one command failed!"<<endl;
			succAll = false;
			break;
		}
		nextInput = _nextInput;
	}
	return succAll;
}

void main_loop(void) {

	int status = 1;
	do {
		string comLine;
		cout << "chamo>";
		char a;
		bool done = false;
		do {
			a = getchar();
			if (a == EOF || a == '\n') {
				done = true;
				if (a == EOF) {
					status = 0;
				}
			} else {
				comLine.push_back(a);
			}
		} while (!done);
		if (comLine == "quit" || status == 0) {
			status = 0;
		} else {
			status = execute_string(comLine);
		}
	} while (status);
}

int main(int argc, char **argv) {
	main_loop();
	//vector<string> args;
	//args.push_back("gedit");
	//execute(args,"input.txt","chamo.txt",true);
}
