#include <string>
#include <iostream>
#include <list>

#include "utils.h"
#include "Process.h"


using namespace std;

#ifndef SESSION_H
#define SESSION_H


class Session {
	struct Variables {
		string varName = "empty";
		int value = 0;
	};

	string name;
	Process* processPtr;
	string currentLine;
	int totalLines = 0;
	string timestamp;
	string printMsg;
	Variables vars[100];
	list<string> printedLines;
	int nestedLoopNum = 0;

public:
	void newSession(string scrName, string timeCreated, Process &assignedProcess);
	string GetName();
	void screen();
	void run();
	void RunInstructions(int instructionToRun);
	void PrintProcessRunning();
	int FindVariable(string varInput);
	bool CheckVariable(string input);
	int ValueAssignment(string variable);
	void PRINT(string msg);
	void DECLARE(string name, int val);
	void ADD(string sum, string addend1, string addend2);
	void SUB(string diff, string subend1, string subend2);
	void FOR(int iterations);
	void SLEEP(int cycles);
};

#endif // SESSION_H