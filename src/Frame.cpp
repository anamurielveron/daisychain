#include "Frame.h"

void Frame::newFrame(int frameMem, int frame_add) {
    frame_mem = frameMem;
    address = frame_add;
}

void Frame::AssignProcess(Screen* process_obj) {
    process = process_obj;
    is_occupied = true;
}

bool Frame::CheckIsOccupied() {
    return is_occupied;
}

void Frame::EmptyFrame() {
    is_occupied = false;
    process = nullptr;
}

Screen* Frame::CheckContents() {
    return process;
}
