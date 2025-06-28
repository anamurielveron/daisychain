#include <iostream>
#include <string>

#pragma once

#include "Process.h"

#ifndef BASESCHEDULER_H
#define BASESCHEDULER_H

class BaseScheduler {
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
    virtual Process* GetProcessByName(const string& name) = 0;
};

#endif // BASESCHEDULER