#ifndef SCREEN_H
#define SCREEN_H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "utils.h"
#include <map>


using namespace std;

class Screen 
{
private:
    std::map<std::string, int> symbolTable; // varName -> memoryAddress
    int memoryRequirement = 0;
    std::map<std::string, int> symbol_table;
    std::string process_name;
    int memory_address = -1;  // -1 means not assigned

    struct Variables {
        string varName = "empty";
        int value = 0;
    };
    int id;
    unsigned int totalInstructions;
    unsigned int executedInstructions;
    string arrivalTimestamp;
    int coreAssigned;
    vector<string> printLogs;
    string name; // Human-readable name like screen_01
    atomic<bool> finished; // To indicate if the process has finished
    string printMsg;
    Variables vars[32];
    int nestedLoopNum = 0;

    std::string pid;
    int processSize;
    std::unordered_map<std::string, int> symbolTable; // symbol name -> address

public:

    // For memory management
    void setMemoryRequirement(int mem);
    int getMemoryRequirement() const;

    // Symbol table handling
    void addToSymbolTable(const std::string& var, int memAddr);
    void clearSymbolTable();
    void printSymbolTable() const;

    void setProcessName(const std::string& name);
    std::string getProcessName() const;

    void setMemoryAddress(int address);
    int getMemoryAddress() const;


    void addSymbol(std::string symbol, int address) {
        symbol_table[symbol] = address;
    }

    std::map<std::string, int> getSymbolTable() const {
        return symbol_table;
    }

    Screen(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName)
        : id(newId), totalInstructions(newTotalInstructions), arrivalTimestamp(timeArrived),
        executedInstructions(0), coreAssigned(-1), name(processName), finished(false) {}

    // Delete copy constructor and copy assignment operator for std::atomic member
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    // Allow move constructor and move assignment operator
    Screen(Screen&& other) noexcept
        : id(other.id),
        totalInstructions(other.totalInstructions),
        executedInstructions(other.executedInstructions),
        arrivalTimestamp(std::move(other.arrivalTimestamp)),
        coreAssigned(other.coreAssigned),
        printLogs(std::move(other.printLogs)),
        name(std::move(other.name)),
        finished(other.finished.load()) { // Atomically load value for move
        other.id = 0; // Clear original
        other.totalInstructions = 0;
        other.executedInstructions = 0;
        other.coreAssigned = -1;
        other.finished.store(true); // Mark original as finished/invalidated
    }

    Screen& operator=(Screen&& other) noexcept {
        if (this != &other) {
            id = other.id;
            totalInstructions = other.totalInstructions;
            executedInstructions = other.executedInstructions;
            arrivalTimestamp = std::move(other.arrivalTimestamp);
            coreAssigned = other.coreAssigned;
            printLogs = std::move(other.printLogs);
            name = std::move(other.name);
            finished.store(other.finished.load()); // Atomically load and store

            other.id = 0; // Clear original
            other.totalInstructions = 0;
            other.executedInstructions = 0;
            other.coreAssigned = -1;
            other.finished.store(true); // Mark original as finished/invalidated
        }
        return *this;
    }

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
    void ExecuteInstruction(int coreNum);
    //void AddPrintLog(const string& message, int coreNum);
    int GetPID() const;
    string GetName() const;
    string GetArrivalTime() const; // Renamed for clarity
    unsigned int GetExecutedInstructions() const; // Renamed for clarity
    unsigned int GetTotalInstructions() const;
    int GetCoreValue() const;
    void SetCoreValue(int value);
    vector<string> GetPrintLogs() const;
    bool IsFinished() const;
    void SetFinished(bool status);

    Screen(std::string pid, int processSize);

    std::string getPID() const;
    int getProcessSize() const;

    void addSymbol(std::string symbol, int address);
    int getSymbolAddress(const std::string& symbol) const;
    bool hasSymbol(const std::string& symbol) const;

    const std::unordered_map<std::string, int>& getSymbolTable() const;
};

#endif // SCREEN_H
