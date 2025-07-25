#include <iostream>
#include <string>

#include "Screen.h"

using namespace std;

#ifndef FRAME_H
#define FRAME_H

class Frame {
	int frame_mem;
	int address = 0;
	Screen* process = nullptr;
	bool is_occupied =false;
	int lower_bound;
	int upper_bound;
	std::string contents;

public:
	void newFrame(int frameMem, int frame_add);
	void AssignProcess(Screen* process_ptr);
	bool CheckIsOccupied();
	void EmptyFrame();
	Screen* CheckContents();
};

#endif
