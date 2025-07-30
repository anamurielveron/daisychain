#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

using namespace std;

#ifndef SCREEN_H
#define SCREEN_H

class Screen 
{
private:
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
    Variables vars[100];
    int nestedLoopNum = 0;

    vector<string> instructions; // Holds the list of instructions for the process

public:
    Screen(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName)
        : id(newId), totalInstructions(newTotalInstructions), arrivalTimestamp(timeArrived),
        executedInstructions(0), coreAssigned(-1), name(processName), finished(false) {}
    
    Screen(const string& processName, int memorySize, const vector<string>& instructions);

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

};

#endif // SCREEN_H
