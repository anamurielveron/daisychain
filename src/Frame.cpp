#include "Frame.h"

void Frame::newFrame(int frameMem, int frame_add)
{
	frame_mem = frameMem;
	address = frame_add;
}

void Frame::AssignProcess(string process_name, Page* page) {
	is_occupied = true;
	current_process = process_name;
	assigned_Page = page;
	is_occupied = true;
}

bool Frame::CheckIsOccupied() const {
	return is_occupied;
}

string Frame::CheckContents() const {
	return current_process;
}

void Frame::EmptyFrame() {
	is_occupied = false;
	current_process = "";
	assigned_Page = nullptr;
}

int Frame::ReturnPageTally() {
	return assigned_Page->GetPageTally();
}

int Frame::GetFrameAddress() const {
	return address;
}