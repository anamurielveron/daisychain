#include <process.h>
#include <iostream>
#include <string>

#pragma once

#include "Screen.h"

#ifndef BASESCHEDULER_H
#define BASESCHEDULER_H

class BaseScheduler {
public:
    virtual ~BaseScheduler() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void SchedulerLoop() = 0;
    virtual void CreateProcess(bool isBatch = false, const string& userProvidedName = "") = 0;

    // New methods for MO2
    virtual void CreateProcessWithMemory(bool isBatch, const string& processName, int memorySize) = 0;
    virtual void CreateProcessWithInstructions(const string& processName, int memorySize, const string& instructions) = 0;

    virtual void DisplayStatus(ostream& os) = 0;
    virtual bool IsRunning() = 0;
    virtual Screen* GetProcessByName(const string& name) = 0;
};

#endif // BASESCHEDULER