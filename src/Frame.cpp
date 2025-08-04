#include "Frame.h"

void Frame::newFrame(int frameMem, int frame_add)
{
	frame_mem = frameMem;
	address = frame_add;
}

void Frame::AssignProcess(string process_name, Page* page) {
	current_process = process_name;
	assigned_Page = page;
}

bool Frame::CheckIsOccupied() {
	return is_occupied;
}

void Frame::EmptyFrame() {
	is_occupied = false;
	current_process = "";
}

string Frame::CheckContents() {
	return current_process;
}

int Frame::ReturnPageTally() {
	return assigned_Page->GetPageTally();
}

int Frame::GetFrameAddress() const {
	return address;
}