#include "Session.h"
#include "utils.h"
#include <windows.h>

void Session::newSession(string scrName, string timeCreated, Process &assignedProcess) {
	name = scrName;
	currentLine = "";
	timestamp = timeCreated;
	processPtr = &assignedProcess;

	printMsg = "Hello from " + scrName + "!";
}

string Session::GetName() {
	return name;
}

void Session::screen() {
	//Display session name and time created
	printColor("Welcome to " + name + "\n\n", YELLOW);
	printColor("Time created: " + timestamp + "\n", YELLOW);

	printPlaceHolderConsoles();
	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
		if (command == "process-smi") {
			printColor("Process name: " + name + "\n", CYAN);
			printColor("ID: " + to_string(processPtr->GetPID()) + "\n", CYAN);
			printColor("Current instruction line: " + to_string(printedLines.size()) + "/" + to_string(processPtr->GetBT()) + "\n", CYAN);
			PrintProcessRunning();
		}else if(command == "exit") {
			break;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
}

void Session::run() {
	while (totalLines < processPtr->GetBT()) {
		RunInstructions(rand() % 6);
	}
}

void Session::RunInstructions(int instructionToRun) {
	if (instructionToRun == 0) {
		PRINT(printMsg);
	}
	else if (instructionToRun == 1){
		int varNum = rand() % 101;
		string variable = "var" + to_string(varNum);
		DECLARE(variable, rand() % 500);
	}
	else if (instructionToRun == 2) {
		int varNum[3];
		string variable[3];
		for (int i = 0; i < 3; i++){
			varNum[i] = rand() % 101;
			variable[i] = "var" + to_string(varNum[i]);
		}
		
		ADD(variable[0], variable[1], variable[2]);
	}
	else if (instructionToRun == 3) {
		int varNum[3];
		string variable[3];
		for (int i = 0; i < 3; i++) {
			varNum[i] = rand() % 101;
			variable[i] = "var" + to_string(varNum[i]);
		}

		SUB(variable[0], variable[1], variable[2]);
	}
	else if (instructionToRun == 4) {
		if (nestedLoopNum == 3) {
			RunInstructions(rand() % 6);
		}
	}
	else {
		SLEEP(10);
	}
}

void Session::PrintProcessRunning() {
	printColor("Log: \n", CYAN);
	for (string print : printedLines) {
		printColor(print + "\n", INVERTED);
	}
}

bool Session::CheckVariable(string input) {
	for (char c : input) {
		if (!isdigit(c)) {
			return false;
		}
	}

	return true;
}

int Session::FindVariable(string varInput) {
	for (int i = 0; i < 100;i++) {
		if (vars[i].varName == varInput) {
			return i;
		}
		else if (vars[i].varName == "empty") {
			return -1;
		}
	}

	return -1;
}

int Session::ValueAssignment(string variable) {
	int foundIndex;
	int constantVar;

	if (CheckVariable(variable)) {
		constantVar = stoi(variable);
	}
	else
	{
		foundIndex = FindVariable(variable);
		if (foundIndex == -1) {
			DECLARE(variable, 0);
			constantVar = 0;
		}
		else {
			constantVar = vars[foundIndex].value;
		}
	}

	return constantVar;
}

void Session::PRINT(string msg) {
	printedLines.push_back(getCurrentTimestamp() + " Core: " + to_string(processPtr->GetCoreValue()) + " " + msg);
	totalLines++;
}

void Session::DECLARE(string name, int val) {
	bool exists = false;
	int foundIndex = FindVariable(name);

	if (foundIndex == -1) {
		for (int i = 0; i < 100; i++) {
			if (vars[i].varName == "empty") {
				vars[i].varName = name;
				vars[i].value = val;
				break;
			}
		}
	}
	else {
		vars[foundIndex].value = val;
	}

	//printedLines.push_back(getCurrentTimestamp() + " New variable added: " + name);
	totalLines++;
}

void Session::ADD(string sum, string addend1, string addend2) {
	int varSumIndex = FindVariable(sum);
	int varAddEnd1 = ValueAssignment(addend1);
	int varAddEnd2 = ValueAssignment(addend2);
	int tempSum = varAddEnd1 + varAddEnd2;

	if (varSumIndex == -1) {
		DECLARE(sum, tempSum);
	}
	else {
		vars[varSumIndex].value = tempSum;
	}

	//printedLines.push_back(getCurrentTimestamp() + " Added: " + addend1 + " + " + addend2 + " = " + sum);
	totalLines++;
}

void Session::SUB(string diff, string subend1, string subend2) {
	int varDiffIndex = FindVariable(diff);
	int varSubEnd1 = ValueAssignment(subend1);
	int varSubEnd2 = ValueAssignment(subend2);
	int tempDiff = varSubEnd1 + varSubEnd2;

	if (varDiffIndex == -1) {
		DECLARE(diff, tempDiff);
	}
	else {
		vars[varDiffIndex].value = tempDiff;
	}

	//printedLines.push_back(getCurrentTimestamp() + " Subtracted: " + subend1 + " + " + subend2 + " = " + diff);
	totalLines++;
}

void Session::FOR(int iterations) {
	nestedLoopNum++;
	int instructionToRun = rand() % 6;

	for (int i = 0; i < iterations; i++) {
		RunInstructions(instructionToRun);
		
	}
	printedLines.push_back(getCurrentTimestamp() + " Core: " + to_string(processPtr->GetCoreValue()) + " Looping finished!");
	totalLines++;
}

void Session::SLEEP(int cycles) {
	Sleep(cycles);
	totalLines++;
}