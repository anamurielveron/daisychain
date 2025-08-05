#include <process.h>
#include "Screen.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>

#include "BaseScheduler.h"

#ifndef RRSCHEDULER_H
#define RRSCHEDULER_H

class RRScheduler : public BaseScheduler {
private:
    struct CoreSlot {
        unique_ptr<Screen> proc; // Use unique_ptr
        int qRemaining;
        bool isEmpty;

        CoreSlot() : qRemaining(0), isEmpty(true) {}
    };

    vector<CoreSlot> cores;
    queue<unique_ptr<Screen>> readyQueue; // Use unique_ptr
    queue<unique_ptr<Screen>> doneQueue; // Use unique_ptr
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    thread schedulerThread;
    atomic<long long> cpuCycles; // Use long long for cycles
    int numCores;
    int quantum;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    int batchProcessFrequency;
    int delayPerExecution;

public:
    RRScheduler(int num_cores, int quantum_cycles, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec);
    ~RRScheduler();
    void Start() override;
    void Stop() override;
    void SchedulerLoop() override;
    void CreateProcess(bool isBatch = false, const string& userProvidedName = "") override;
    void CreateProcessWithMemory(bool isBatch, const string& processName, int memorySize) override;
    void CreateProcessWithInstructions(const string& processName, int memorySize, const string& instructions) override;
    void DisplayStatus(ostream& os) override;
    bool IsRunning() override;
    Screen* GetProcessByName(const string& name) override;
};

#endif // RRSCHEDULER_H