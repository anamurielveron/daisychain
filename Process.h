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
    int bt;
    int currentProcessedBT = 0;
    string at;
    int core = -1;
    vector<string> printLogs;

public:
    bool processDone = false;

    void NewProcess(int newId, int newBT, string timeArrived);
    void IncreaseProcessBT();
    void CreateNewFile();
    void AddPrintLog(const string& message, int coreNum);
    int GetPID() const;
    string GetAT() const;
    int GetCurrentProgress() const;
    int GetBT() const;
    int GetCoreValue() const;
    void SetCoreValue(int value);
    vector<string> GetPrintLogs() const;
};

#endif // PROCESS_H
