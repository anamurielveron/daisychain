#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>

#include <vector>

#include "Frame.h"

using namespace std;

class Memory {
	int max_mem;
	std::vector<Frame*> mem_frames;
	int num_of_frames;
	int mem_per_frame;

public:
	Memory(int mem_limit, int frame_mem);
	void FF_AssignProcessToFrame(Screen* process);
	void FF_DetachProcessFromMemory(Screen* process);
	void GenerateMemoryReport(int quantumCycle);
	void PrintMemoryToConsole(int quantumCycle);
	void displayProcessSymbols(Screen* process);
	~Memory();
};

#endif
