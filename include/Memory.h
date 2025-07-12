#include <iostream>
#include <string>

#include <vector>

#include "Frame.h"

using namespace std;

#ifndef MEMORY_H
#define MEMORY_H

class Memory {
	int max_mem;
	std::vector<Frame> mem_frames;
	int num_of_frames;
	int mem_per_frame;

public:
	Memory(int mem_limit, int frame_mem);
	void FF_AssignProcessToFrame(string process, int processSize);
	void FF_DetachProcessFromMemory(string process);
	void GenerateMemoryReport(int quantumCycle);
};

#endif
