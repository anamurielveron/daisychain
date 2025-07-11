#include "Frame.h"

void Frame::newFrame(int frameMem, int frame_add)
{
	frame_mem = frameMem;
	address = frame_add;
}

void Frame::AssignProcess(string process_name) {
	process = process_name;
	is_occupied = true;
}

bool Frame::CheckIsOccupied() {
	return is_occupied;
}

void Frame::EmptyFrame() {
	is_occupied = false;
	process;
}

string Frame::CheckContents() {
	return process;
}