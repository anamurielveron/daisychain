#include<iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <list>
#include <windows.h>
#include "Screen.h"

using namespace std;

void Screen::screen() {
	clear();
    //Display session name and time created
    printColor("Welcome to " + name + "\n\n", YELLOW);
    printColor("Time created: " + arrivalTimestamp + "\n", YELLOW);
    printColor("Memory allocated: " + to_string(allocatedMemory) + " bytes\n", YELLOW);

    printPlaceHolderConsoles();
    while (true) {
        std::string command;
        printColor("~> ", CYAN);
        std::getline(std::cin, command);
        if (command == "process-smi") {
            printColor("Screen name: " + name + "\n", CYAN);
            printColor("ID: " + to_string(id) + "\n", CYAN);
            printColor("Current instruction line: " + to_string(executedInstructions) + "/" + to_string(totalInstructions) + "\n", CYAN);
            printColor("Memory allocated: " + to_string(allocatedMemory) + " bytes\n", CYAN);
            if (hasMemoryViolation) {
                printColor("Memory violation detected at " + memoryViolationTime + " - Address: " + memoryViolationAddress + "\n", RED);
            }
            PrintProcessRunning();
        }
        else if (command == "exit") {
            break;
        }
    }
    system("cls");

	/*printPlaceHolderConsoles();*/

	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
		if (command == "process-smi") {
			printColor("Screen name: " + name + "\n", CYAN);
			printColor("ID: " + to_string(id) + "\n", CYAN);
			printColor("Current instruction line: " + to_string(executedInstructions) + "/" + to_string(totalInstructions) + "\n", CYAN);
			PrintProcessRunning();
		}
		else if (command == "exit") {
			break;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
    printBanner();
    printSubtitle();
}

void Screen::run() {
    while (executedInstructions < totalInstructions) {
        RunInstructions(rand() % 6);
    }
}

void Screen::RunInstructions(int instructionToRun) {
    if (instructionToRun == 0) {
        PRINT(printMsg);
    }
    else if (instructionToRun == 1) {
        int varNum = rand() % 101;
        string variable = "var" + to_string(varNum);
        DECLARE(variable, rand() % 500);
    }
    else if (instructionToRun == 2) {
        int varNum[3];
        string variable[3];
        for (int i = 0; i < 3; i++) {
            varNum[i] = rand() % 101;
            if (rand())
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

void Screen::PrintProcessRunning() {
    printColor("Log: \n", CYAN);
    for (string print : printLogs) {
        printColor(print + "\n", INVERTED);
    }
}

bool Screen::CheckVariable(string input) {
    for (char c : input) {
        if (!isdigit(c)) {
            return false;
        }
    }

    return true;
}

int Screen::FindVariable(string varInput) {
    for (int i = 0; i < 32; i++) {
        if (vars[i].varName == varInput) {
            return i;
        }
        else if (vars[i].varName == "empty") {
            return -1;
        }
    }

    return -1;
}

uint16_t Screen::ValueAssignment(string variable) {
    int foundIndex;
    uint16_t constantVar;

    if (CheckVariable(variable)) {
        constantVar = static_cast<uint16_t>(stoi(variable));
    }
    else {
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

void Screen::PRINT(string msg) {
    printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " " + msg);
    executedInstructions++;
}

void Screen::DECLARE(string name, uint16_t val) {
    bool exists = false;
    int foundIndex = FindVariable(name);

    if (foundIndex == -1) {
        for (int i = 0; i < 32; i++) {
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

    executedInstructions++;
}

void Screen::ADD(string sum, string addend1, string addend2) {
    int varSumIndex = FindVariable(sum);
    uint16_t varAddEnd1 = ValueAssignment(addend1);
    uint16_t varAddEnd2 = ValueAssignment(addend2);
    uint16_t tempSum = varAddEnd1 + varAddEnd2;

    if (varSumIndex == -1) {
        DECLARE(sum, tempSum);
    }
    else {
        vars[varSumIndex].value = tempSum;
    }

    executedInstructions++;
}

void Screen::SUB(string diff, string subend1, string subend2) {
    int varDiffIndex = FindVariable(diff);
    uint16_t varSubEnd1 = ValueAssignment(subend1);
    uint16_t varSubEnd2 = ValueAssignment(subend2);
    uint16_t tempDiff = varSubEnd1 - varSubEnd2; // Fixed: should be subtraction, not addition

    if (varDiffIndex == -1) {
        DECLARE(diff, tempDiff);
    }
    else {
        vars[varDiffIndex].value = tempDiff;
    }

    executedInstructions++;
}

void Screen::FOR(int iterations) {
    nestedLoopNum++;

    list<int> instructionToRun;

    if (rand() % 2 == 0) {
        instructionToRun.push_back(rand() % 6);
    }
    else {
        int i = 0;
        int arrLen = (rand() % 10) + 2;
        while (i < arrLen) {
            instructionToRun.push_back(rand() % 6);
            i++;
        }
    }

    for (int i = 0; i < iterations; i++) {
        for (int j : instructionToRun) {
            RunInstructions(j);
        }
    }

    printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " Looping finished!");
}

void Screen::SLEEP(int cycles) {
    Sleep(cycles);
    executedInstructions++;
}

// New memory-related methods
void Screen::READ(string varName, int memoryAddress) {
    if (!isValidMemoryAddress(memoryAddress)) {
        // Memory access violation
        hasMemoryViolation = true;
        memoryViolationTime = getCurrentTimestamp();
        memoryViolationAddress = "0x" + to_string(memoryAddress);
        finished.store(true);
        return;
    }

    uint16_t value = 0;
    if (memoryMap.find(memoryAddress) != memoryMap.end()) {
        value = memoryMap[memoryAddress];
    }

    DECLARE(varName, value);
}

void Screen::WRITE(int memoryAddress, uint16_t value) {
    if (!isValidMemoryAddress(memoryAddress)) {
        // Memory access violation
        hasMemoryViolation = true;
        memoryViolationTime = getCurrentTimestamp();
        memoryViolationAddress = "0x" + to_string(memoryAddress);
        finished.store(true);
        return;
    }

    memoryMap[memoryAddress] = value;
    executedInstructions++;
}

bool Screen::isValidMemoryAddress(int address) {
    // For simplicity, assume valid addresses are within the allocated memory range
    // In a real implementation, this would check against the process's virtual memory space
    return address >= 0 && address < allocatedMemory;
}

Screen::Screen(int id, unsigned int totalInstructions, string arrivalTime, const string& processName, int memorySize, Memory* mem)
	: id(id), totalInstructions(totalInstructions), arrivalTime(arrivalTime), name(processName), executedInstructions(0)
{
	// Memory setup
	this->memorySize = memorySize;
	this->memory = mem;
	this->memory->LRU_AssignProcessToFrame(name); // If using LRU memory handling

	// Generate dummy instructions (as before)
	for (unsigned int i = 0; i < totalInstructions; ++i) {
		instructions.push_back("DUMMY_INSTRUCTION");
	}
}

// Return the instruction vector
std::vector<std::string> Screen::GetInstructionVector() const {
	return instructions;
}

// Print all declared variables
void Screen::PrintVariables() const {
	cout << "Variables:" << endl;
	for (const auto& var : vars) {
		if (var.varName != "empty") {
			cout << "  " << var.varName << " = " << var.value << endl;
		}
	}
}



void Screen::parseCustomInstructions(const string& instructions) {
    stringstream ss(instructions);
    string instruction;

    while (getline(ss, instruction, ';')) {
        // Trim whitespace
        instruction.erase(0, instruction.find_first_not_of(" \t\n\r\f\v"));
        instruction.erase(instruction.find_last_not_of(" \t\n\r\f\v") + 1);

        if (!instruction.empty()) {
            customInstructions.push_back(instruction);
        }
    }
}

void Screen::executeCustomInstruction(const string& instruction) {
    stringstream ss(instruction);
    string command;
    ss >> command;

    if (command == "DECLARE") {
        string varName;
        uint16_t value;
        ss >> varName >> value;
        DECLARE(varName, value);
    }
    else if (command == "PRINT") {
        string message;
        getline(ss, message);
        // Remove quotes if present
        if (message.front() == '"' && message.back() == '"') {
            message = message.substr(1, message.length() - 2);
        }
        PRINT(message);
    }
    else if (command == "ADD") {
        string result, operand1, operand2;
        ss >> result >> operand1 >> operand2;
        ADD(result, operand1, operand2);
    }
    else if (command == "SUB") {
        string result, operand1, operand2;
        ss >> result >> operand1 >> operand2;
        SUB(result, operand1, operand2);
    }
    else if (command == "READ") {
        string varName, addressStr;
        ss >> varName >> addressStr;
        // Convert hex string to int
        int address = stoi(addressStr, 0, 16);
        READ(varName, address);
    }
    else if (command == "WRITE") {
        string addressStr;
        uint16_t value;
        ss >> addressStr >> value;
        // Convert hex string to int
        int address = stoi(addressStr, 0, 16);
        WRITE(address, value);
    }
}

void Screen::ExecuteInstruction(int coreNum) {
    if (executedInstructions < totalInstructions) {
        if (hasCustomInstructions) {
            // Execute custom instruction
            if (customInstructionIndex < customInstructions.size()) {
                executeCustomInstruction(customInstructions[customInstructionIndex]);
                customInstructionIndex++;
                executedInstructions++;
            }
        }
        else {
            // Execute random instruction
            executedInstructions++;
            RunInstructions(rand() % 6);
        }
    }

    if (executedInstructions >= totalInstructions) {
        finished.store(true);
    }
}

// Getters and setters
int Screen::GetPID() const { return id; }
string Screen::GetName() const { return name; }
string Screen::GetArrivalTime() const { return arrivalTimestamp; }
unsigned int Screen::GetExecutedInstructions() const { return executedInstructions; }
unsigned int Screen::GetTotalInstructions() const { return totalInstructions; }
int Screen::GetCoreValue() const { return coreAssigned; }
void Screen::SetCoreValue(int value) { coreAssigned = value; }
vector<string> Screen::GetPrintLogs() const { return printLogs; }
bool Screen::IsFinished() const { return finished.load(); }
void Screen::SetFinished(bool status) { finished.store(status); }
int Screen::GetAllocatedMemory() const { return allocatedMemory; }
bool Screen::HasMemoryViolation() const { return hasMemoryViolation; }
string Screen::GetMemoryViolationTime() const { return memoryViolationTime; }
string Screen::GetMemoryViolationAddress() const { return memoryViolationAddress; }