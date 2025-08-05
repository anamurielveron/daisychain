#include<iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <list>
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include "Screen.h"
#include <unordered_map>
#include <mutex>

#include "Memory.h"
#include <atomic>


using namespace std;

extern std::unordered_map<uint32_t, uint16_t> globalMemory;
extern std::mutex globalMemoryMutex;
extern const uint32_t MEMORY_MIN_ADDRESS;
extern const uint32_t MEMORY_MAX_ADDRESS;

void Screen::screen() {
	//Display session name and time created
	printColor("Welcome to " + name + "\n\n", YELLOW);
	printColor("Time created: " + arrivalTimestamp + "\n", YELLOW);

	for (const auto& instr : instructions) {
		ExecuteStringInstruction(instr);
	}

	printPlaceHolderConsoles();

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
}

void Screen::run() {
	while (executedInstructions < totalInstructions) {
		RunInstructions(rand() % 7);
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
			if(rand())
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

int Screen::ValueAssignment(string variable) {
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

void Screen::PRINT(string msg) {
	printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " " + msg);
}

void Screen::DECLARE(string name, int val) {
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
}

void Screen::ADD(string sum, string addend1, string addend2) {
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
}

void Screen::SUB(string diff, string subend1, string subend2) {
	int varDiffIndex = FindVariable(diff);
	int varSubEnd1 = ValueAssignment(subend1);
	int varSubEnd2 = ValueAssignment(subend2);
	int tempDiff = varSubEnd1 - varSubEnd2;

	if (varDiffIndex == -1) {
		DECLARE(diff, tempDiff);
	}
	else {
		vars[varDiffIndex].value = tempDiff;
	}

	//printedLines.push_back(getCurrentTimestamp() + " Subtracted: " + subend1 + " + " + subend2 + " = " + diff);
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
		while(i < arrLen) {
			instructionToRun.push_back(rand() % 6);
			i++;
		}
	}

	for (int i = 0; i < iterations; i++) {
		for(int j: instructionToRun) {
			RunInstructions(j);
		}
	}

	printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " Looping finished!");
}

void Screen::SLEEP(int cycles) {
	Sleep(cycles);
}


void Screen::ExecuteInstruction(int coreNum) {
	if (executedInstructions < totalInstructions) {
		executedInstructions++;
		RunInstructions(rand() % 6);
		// Simulate a PRINT instruction
		//string message = "\"Hello world from " + name + "! (Inst: " + to_string(executedInstructions) + ")\"";
		//AddPrintLog(message, coreNum);
		if (executedInstructions == totalInstructions) {
			finished.store(true); // Use store for atomic boolean
		}
	}
}

/*
void Screen::AddPrintLog(const string& message, int coreNum) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    string am_pm = (st.wHour >= 12) ? "PM" : "AM";
    int hour = st.wHour % 12;
    if (hour == 0) hour = 12;

    char timestamp[100];
    sprintf_s(timestamp, "(%02d/%02d/%04d %02d:%02d:%02d%s)",
        st.wMonth, st.wDay, st.wYear,
        hour, st.wMinute, st.wSecond, am_pm.c_str());

    string logEntry = string(timestamp) + " Core: " + to_string(coreNum) + " " + message;
    printLogs.push_back(logEntry);
}
*/

Screen::Screen(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName)
	: id(newId),
	totalInstructions(newTotalInstructions),
	executedInstructions(0),
	arrivalTimestamp(timeArrived),
	coreAssigned(-1),
	name(processName),
	finished(false),
	memory(nullptr) // important to initialize
{
	instructions.clear(); // if you want it empty
	for (int i = 0; i < 100; ++i) {
		vars[i].varName = "empty";
		vars[i].value = 0;
	}

	memory->AddNewProcess(name, instructions, process_mem_size);
}

Screen::Screen(const string& processName, Memory* mem, const vector<string>& instructions)
	: id(-1),
	totalInstructions(instructions.size()),
	executedInstructions(0),
	arrivalTimestamp(getCurrentTimestamp()),
	coreAssigned(-1),
	name(processName),
	finished(false),
	memory(mem),
	instructions(instructions)
{
	for (int i = 0; i < 100; ++i) {
		vars[i].varName = "empty";
		vars[i].value = 0;
	}

	memory->AddNewProcess(name, instructions, process_mem_size);
}

Screen::Screen(const string& processName, int memorySize, const vector<string>& instr, Memory* mem)
	: id(-1),
	totalInstructions(instr.size()),
	executedInstructions(0),
	arrivalTimestamp(getCurrentTimestamp()),
	coreAssigned(-1),
	name(processName),
	finished(false),
	instructions(instr),
	memory(mem) 
{
	for (int i = 0; i < 100; ++i) {  // Initialize ALL 100 variables
		vars[i].varName = "empty";
		vars[i].value = 0;
	}

	memory->AddNewProcess(name, instructions, process_mem_size);
}

Screen::Screen(Screen&& other) noexcept
	: id(other.id),
	totalInstructions(other.totalInstructions),
	executedInstructions(other.executedInstructions),
	arrivalTimestamp(std::move(other.arrivalTimestamp)),
	coreAssigned(other.coreAssigned),
	printLogs(std::move(other.printLogs)),
	name(std::move(other.name)),
	finished(other.finished.load()),
	nestedLoopNum(other.nestedLoopNum),
	instructions(std::move(other.instructions)),
	memory(other.memory)
{
	// Copy the vars array
	for (int i = 0; i < 100; ++i) {
		vars[i] = other.vars[i];
	}

	// Reset the other object
	other.id = -1;
	other.totalInstructions = 0;
	other.executedInstructions = 0;
	other.coreAssigned = -1;
	other.finished.store(false);
	other.nestedLoopNum = 0;
	other.memory = nullptr;

	//memory->AddNewProcess(name, instructions, process_mem_size);
}


void Screen::ExecuteStringInstruction(const std::string& instr) {
	istringstream iss(instr);
	string opcode;
	iss >> opcode;

	if (opcode == "DECLARE") {
		string varName, valueStr;
		iss >> varName >> valueStr;
		int value = this->ValueAssignment(valueStr);
		this->DECLARE(varName, value);
	}
	else if (opcode == "ADD") {
		string target, op1, op2;
		iss >> target >> op1 >> op2;
		this->ADD(target, op1, op2);
	}
	else if (opcode == "SUB") {
		string target, op1, op2;
		iss >> target >> op1 >> op2;
		this->SUB(target, op1, op2);
	}
	else if (opcode == "WRITE") {
		string addressStr, sourceVar;
		iss >> addressStr >> sourceVar;

		// Safety check for memory pointer
		if (!this->memory) {
			printColor("Error: Memory object is null for WRITE operation\n", RED);
			return;
		}

		try {
			int addr = std::stoi(addressStr, nullptr, 0); // handles hex like 0x500
			int value = this->ValueAssignment(sourceVar);

			printColor("WRITE: Writing value " + to_string(value) + " to address " + addressStr + "\n", CYAN);

			this->memory->Write(addr, value);

			printColor("WRITE operation completed successfully\n", GREEN);
		}
		catch (const std::exception& e) {
			printColor("Error in WRITE operation: " + string(e.what()) + "\n", RED);
		}
	}
	else if (opcode == "READ") {
		string destVar, addressStr;
		iss >> destVar >> addressStr;

		// Safety check for memory pointer
		if (!this->memory) {
			printColor("Error: Memory object is null for READ operation\n", RED);
			return;
		}

		try {
			int addr = std::stoi(addressStr, nullptr, 0);

			printColor("READ: Reading from address " + addressStr + "\n", CYAN);

			int value = this->memory->Read(addr);
			this->DECLARE(destVar, value);

			printColor("READ operation completed successfully, value: " + to_string(value) + "\n", GREEN);
		}
		catch (const std::exception& e) {
			printColor("Error in READ operation: " + string(e.what()) + "\n", RED);
		}
	}
	else if (opcode == "PRINT") {
		string rest;
		getline(iss, rest);

		// Handle PRINT("text" + variable) format
		size_t openParen = rest.find('(');
		size_t closeParen = rest.rfind(')');

		if (openParen != string::npos && closeParen != string::npos) {
			string content = rest.substr(openParen + 1, closeParen - openParen - 1);

			size_t openQuote = content.find('"');
			size_t closeQuote = content.find('"', openQuote + 1);

			if (openQuote != string::npos && closeQuote != string::npos) {
				string literal = content.substr(openQuote + 1, closeQuote - openQuote - 1);

				// Check for concatenation with +
				size_t plusPos = content.find('+', closeQuote);
				if (plusPos != string::npos) {
					string varName = content.substr(plusPos + 1);
					varName.erase(0, varName.find_first_not_of(" \t"));
					varName.erase(varName.find_last_not_of(" \t") + 1);

					int val = this->ValueAssignment(varName);
					this->PRINT(literal + to_string(val));
				}
				else {
					this->PRINT(literal);
				}
			}
		}
		else {
			// Fallback to original parsing
			size_t openQuote = rest.find('"');
			size_t closeQuote = rest.rfind('"');

			if (openQuote != string::npos && closeQuote != string::npos && closeQuote > openQuote) {
				string content = rest.substr(openQuote + 1, closeQuote - openQuote - 1);
				size_t plusPos = content.find('+');
				if (plusPos != string::npos) {
					string literal = content.substr(0, plusPos);
					string varName = content.substr(plusPos + 1);
					varName.erase(remove_if(varName.begin(), varName.end(), ::isspace), varName.end());
					int val = this->ValueAssignment(varName);
					this->PRINT(literal + to_string(val));
				}
				else {
					this->PRINT(content);
				}
			}
		}
	}
	memory->MoveToNextInstruction(name);
}

void Screen::SetPID(int newId) {
	id = newId;
}

void Screen::DeclareVariable(const std::string& name, int value, int address) {
    if (symbolTable.size() >= 32) {
        // Symbol table full, ignore new declarations
        printLogs.push_back("Symbol table full. Variable '" + name + "' not declared.");
        return;
    }
    // Clamp value to uint16_t
    uint16_t clamped = static_cast<uint16_t>(std::max(0, std::min(value, 0xFFFF)));
    symbolTable[name] = { clamped, address };
}

bool Screen::GetVariable(const std::string& name, int& value, int& address) const {
    auto it = symbolTable.find(name);
    if (it != symbolTable.end()) {
        value = it->second.value;
        address = it->second.memoryAddress;
        return true;
    }
    return false;
}

bool Screen::READ(const std::string& var, uint32_t memory_address) {
    if (memory_address < MEMORY_MIN_ADDRESS || memory_address > MEMORY_MAX_ADDRESS) {
        printLogs.push_back("Access violation at address 0x" + to_string(memory_address) + ". Process terminated.");
        finished.store(true);
        return false;
    }
    uint16_t value = 0;
    {
        std::lock_guard<std::mutex> lock(globalMemoryMutex);
        auto it = globalMemory.find(memory_address);
        if (it != globalMemory.end()) value = it->second;
    }
    DeclareVariable(var, value, memory_address);
    printLogs.push_back("READ " + var + " from 0x" + to_string(memory_address) + " = " + to_string(value));
    return true;
}

bool Screen::WRITE(uint32_t memory_address, uint16_t value) {
    if (memory_address < MEMORY_MIN_ADDRESS || memory_address > MEMORY_MAX_ADDRESS) {
        printLogs.push_back("Access violation at address 0x" + to_string(memory_address) + ". Process terminated.");
        finished.store(true);
        return false;
    }
    value = std::min<uint16_t>(value, 0xFFFF);
    {
        std::lock_guard<std::mutex> lock(globalMemoryMutex);
        globalMemory[memory_address] = value;
    }
    printLogs.push_back("WRITE " + to_string(value) + " to 0x" + to_string(memory_address));
    return true;
}

void testReadWrite(Screen* process) {
	// READ my_var 0x1000
	process->READ("my_var", 0x1000);

	// WRITE 0x2000 42
	process->WRITE(0x2000, 42);
}

int Screen::GetPID() const { return id; }
string Screen::GetName() const { return name; }
string Screen::GetArrivalTime() const { return arrivalTimestamp; }
unsigned int Screen::GetExecutedInstructions() const { return executedInstructions; }
unsigned int Screen::GetTotalInstructions() const { return totalInstructions; }
int Screen::GetCoreValue() const { return coreAssigned; }
void Screen::SetCoreValue(int value) { coreAssigned = value; }
vector<string> Screen::GetPrintLogs() const { return printLogs; }
bool Screen::IsFinished() const { return finished.load(); } // Use load for atomic boolean
void Screen::SetFinished(bool status) { finished.store(status); } // Use store for atomic boolean