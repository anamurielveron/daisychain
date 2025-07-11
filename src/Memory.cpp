#include "Memory.h"

Memory::Memory(int mem_limit, int frame_mem)
	: max_mem(mem_limit), mem_per_frame(frame_mem) {
	num_of_frames = max_mem / mem_per_frame;
	mem_frames.resize(num_of_frames);

	int address = mem_per_frame;

	for (int i = 0; i < num_of_frames; i++) {
		mem_frames[i].newFrame(mem_per_frame, address);
		address = address + mem_per_frame;
	}
}

void Memory::FF_AssignProcessToFrame(string process, int processSize) {
	int mem_to_alloc = processSize / mem_per_frame;

	int i, j = 0;

	while (i < num_of_frames) {
		int slotFound = 0;
		j = i;
		while (slotFound < mem_to_alloc) {
			if (mem_frames[j].CheckIsOccupied()) {
				break;
			}
			slotFound++;
			j++;
		}

		if (slotFound == mem_to_alloc) {
			int k = 0;
			while (k < mem_to_alloc) {
				mem_frames[i].AssignProcess(process);
				i++;
			}
			break;
		}

		i++;
	}
}

void Memory::FF_DetachProcessFromMemory(string process) {
	for (int i = 0; i < num_of_frames; i++) {
		if (process == mem_frames[i].CheckContents()) {
			mem_frames[i].EmptyFrame();
		}
	}
}