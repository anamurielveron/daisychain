#include "Process.h"
#include "utils.h"

#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>


class RRScheduler {
private:
    static const int NUM_CORES = 4;
    static const int QUANTUM = 10;

    struct CoreSlot {
        Process proc;
        int qRemaining;
        bool isEmpty;

        CoreSlot() : qRemaining(0), isEmpty(true) {}
    };

    CoreSlot cores[NUM_CORES];
    queue<Process> readyQueue;
    queue<Process> doneQueue;
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    Process emptyProcess;
    thread schedulerThread;

public:
    RRScheduler();
    ~RRScheduler();
    void Start();
    void Stop();
    void SchedulerLoop();
    void CreateProcess();
    void DisplayStatus();
    bool IsRunning();
    queue<Process> GetFinishedProcesses();
};