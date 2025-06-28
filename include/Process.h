#include <iostream>
#include <string>
#include <vector>

using namespace std;

#ifndef PROCESS_H
#define PROCESS_H

class Process 
{
private:
    int id;
    unsigned int totalInstructions;
    unsigned int executedInstructions;
    string arrivalTimestamp;
    int coreAssigned;
    vector<string> printLogs;
    string name; // Human-readable name like screen_01
    atomic<bool> finished; // To indicate if the process has finished

public:
    Process(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName)
        : id(newId), totalInstructions(newTotalInstructions), arrivalTimestamp(timeArrived),
        executedInstructions(0), coreAssigned(-1), name(processName), finished(false) {}

    // Delete copy constructor and copy assignment operator for std::atomic member
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    // Allow move constructor and move assignment operator
    Process(Process&& other) noexcept
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

    Process& operator=(Process&& other) noexcept {
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


    void ExecuteInstruction(int coreNum);
    void AddPrintLog(const string& message, int coreNum);
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

#endif // PROCESS_H
