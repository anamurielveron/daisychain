#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include "Frame.h"

using namespace std;

class Memory {
private:
    int max_mem;
    int num_of_frames;
    int mem_per_frame;
    std::vector<Frame> mem_frames;
    std::vector<int> memoryData; // actual memory to store int values

public:
    Memory(int mem_limit, int frame_mem);

    void FF_AssignProcessToFrame(string process, int processSize);
    void FF_DetachProcessFromMemory(string process);
    void GenerateMemoryReport(int quantumCycle);

    int Read(int address) const {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory read out of bounds");
        return memoryData[address];
    }

    void Write(int address, int value) {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory write out of bounds");
        memoryData[address] = value;
    }
};

#endif
