#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

using namespace std;

#include "Process.h"

#ifndef FCFSSCHEDULER_H
#define FCFSSCHEDULER_H

class FCFSScheduler {
private:
    static const int NUM_CORES = 4;
    Process cores[NUM_CORES];
    queue<Process> readyQueue;
    queue<Process> doneQueue;
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    Process emptyProcess;
    thread schedulerThread;

public:
    FCFSScheduler();
    ~FCFSScheduler();
    void Start();
    void Stop();
    void SchedulerLoop();
    void CreateProcess();
    void DisplayStatus();
    bool IsRunning();
    queue<Process> GetFinishedProcesses();
};

#endif // FCFSSCHEDULER_H

