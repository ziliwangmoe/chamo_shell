#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

int chamo_launch(string appName, vector<string> args) {
	pid_t pid;
	int status;
	char **pArgs = new char*[args.size()];
	for (int i=0;i<args.size();i++){
		pArgs[i] = new char[args.size()];
		for (int j=0;j<args[i].size();j++){
			pArgs[i][j] = args[i][j];
		}
	}

	pid = fork();
	if (pid == 0) {
		if (execvp(appName.c_str(), pArgs) == -1) {
			cout<<"something wrong!!"<<endl;
		}
		exit (EXIT_FAILURE);
	} else if (pid < 0) {
		cout<<"something wrong!!"<<endl;
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int chamo_execute(string appName, vector<string> args) {
	int i;

	if (appName == "") {
		return 1;
	}

	return chamo_launch(appName, args);
}

void main_loop(void) {
	string comLine;
	string appName;
	vector<string> args;
	int status;
	do {
		cout << "chamo>";
		getline(std::cin, comLine);
		appName = comLine;
		status = chamo_execute(appName, args);

	} while (status);
}

int main(int argc, char **argv) {
	main_loop();
}
