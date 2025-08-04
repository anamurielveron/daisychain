#include <iostream>
#include <string>


using namespace std;

#ifndef FRAME_H
#define FRAME_H

class Frame {
	int frame_mem = 0;
	int address = 0;
	string process;
	bool is_occupied;
	int lower_bound = 0;
	int upper_bound = 0;
	std::string contents;

public:
	void newFrame(int frameMem, int frame_add);
	void AssignProcess(string process_name);
	bool CheckIsOccupied();
	void EmptyFrame();
	string CheckContents();
};

#endif
