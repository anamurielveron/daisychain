#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>

#include "utils.h"
class Memory;


using namespace std;

class Screen 
{
private:
    struct Variables {
        string varName = "empty";
        int value = 0;
    };
    struct SymbolInfo {
        int value;
        int memoryAddress; // Address in memory
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
    Variables vars[100];
    int nestedLoopNum = 0;
    unordered_map<string, SymbolInfo> symbolTable;

    vector<string> instructions; // Holds the list of instructions for the process
    Memory* memory;
    int process_mem_size;

public:
    Screen(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName, Memory* mem);
    Screen(const string& processName, Memory* mem, const vector<string>& instructions);
    Screen(const string& processName, int memorySize, const vector<string>& instructions, Memory* mem);


    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

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

    Screen(Screen&& other) noexcept;
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

    void ExecuteStringInstruction(const std::string& instr);
    void SetPID(int newId);


    void DeclareVariable(const std::string& name, int value, int address);
    bool GetVariable(const std::string& name, int& value, int& address) const;
    bool READ(const std::string& var, uint32_t memory_address);
    bool WRITE(uint32_t memory_address, uint16_t value);
};

#endif // SCREEN_H
