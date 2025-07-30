#include <process.h>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>

#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

#include "Screen.h"
#include "BaseScheduler.h"


#ifndef FCFSSCHEDULER_H
#define FCFSSCHEDULER_H

#include <atomic>

extern std::atomic<bool> enableBatch;

class FCFSScheduler : public BaseScheduler {
private:
    vector<unique_ptr<Screen>> cores; // Use unique_ptr
    queue<unique_ptr<Screen>> readyQueue; // Use unique_ptr
    queue<unique_ptr<Screen>> doneQueue; // Use unique_ptr
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    thread schedulerThread;
    atomic<long long> cpuCycles; // Use long long for cycles
    int numCores;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    int batchProcessFrequency;
    int delayPerExecution;

    std::atomic<bool> enableBatchFlag;

    Memory* memory;


public:
    FCFSScheduler(int num_cores, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec, Memory* mem = nullptr);
    ~FCFSScheduler();
    void Start() override;
    void Stop() override;
    void SchedulerLoop() override;
    void CreateProcess(bool isBatch = false, const string& userProvidedName = "") override;
    void DisplayStatus(ostream& os) override;
    bool IsRunning() override;
    Screen* GetProcessByName(const string& name) override;

    void SetBatchEnabled(bool enabled) override;

    void CreateProcessWithInstructions(const string& processName, int processSize, const vector<string>& instructions) override;

};

#endif // FCFSSCHEDULER_H

