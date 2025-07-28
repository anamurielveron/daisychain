#ifndef BASESCHEDULER_H
#define BASESCHEDULER_H

#include <process.h>
#include <iostream>
#include <string>
#include <memory>
#include <queue>

#pragma once

#include "Screen.h"


class BaseScheduler {

protected:
    std::queue<std::shared_ptr<Screen>> processQueue;
    Memory* memory;  // Pointer to shared memory

public:
    virtual ~BaseScheduler() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void SchedulerLoop() = 0;
    virtual void CreateProcess(bool isBatch = false, const string& userProvidedName = "") = 0; // Added userProvidedName
    virtual void DisplayStatus(ostream& os) = 0; // Modified to take ostream
    virtual bool IsRunning() = 0;
    // Modified GetProcessByName to search through finished processes as well
    // and to return nullptr if the process is finished, so the main thread
    // doesn't try to access a deallocated object.
    virtual Screen* GetProcessByName(const string& name) = 0;

    virtual void setMemory(Memory* mem) {
        this->memory = mem;
    }

    virtual void SetBatchEnabled(bool enabled) = 0; // Dummy generation

    virtual void AddProcess(std::shared_ptr<Screen> process) = 0;
    virtual void ExecuteCycle(int quantum = 0) = 0;
    virtual bool IsEmpty() const = 0;
    virtual void PrintQueue() const = 0;
    virtual ~BaseScheduler() = default;
};

#endif // BASESCHEDULER