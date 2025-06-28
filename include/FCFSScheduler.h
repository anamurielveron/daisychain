#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

using namespace std;

#include "Process.h"
#include "BaseScheduler.h"

#ifndef FCFSSCHEDULER_H
#define FCFSSCHEDULER_H

class FCFSScheduler : public BaseScheduler {
private:
    vector<unique_ptr<Process>> cores; // Use unique_ptr
    queue<unique_ptr<Process>> readyQueue; // Use unique_ptr
    queue<unique_ptr<Process>> doneQueue; // Use unique_ptr
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

public:
    FCFSScheduler(int num_cores, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec);
    ~FCFSScheduler();
    void Start() override;
    void Stop() override;
    void SchedulerLoop() override;
    void CreateProcess(bool isBatch = false, const string& userProvidedName = "") override;
    void DisplayStatus(ostream& os) override;
    bool IsRunning() override;
    Process* GetProcessByName(const string& name) override;
};

#endif // FCFSSCHEDULER_H

