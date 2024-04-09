//
// Created by Yujia Zhu on 4/6/24.
//

#include <cstdio>
#include "Pager.h"

Pager::Pager() {
    this->frame_table = 0;
    this->is_print = false;
    this->instr_cnt = 0;
    this->NRU_cnt = 0;
    this->frame_size = 0;
}

frame_t* Pager::select_victim_frame() {
    return 0;
}


FIFO::FIFO(vector<frame_t>* frame_table, bool is_print) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->hand = -1;
}

frame_t* FIFO::select_victim_frame() {
    hand = (hand + 1) % frame_size;
    frame_t* frame = &(*this->frame_table)[hand];
    if (is_print) printf("ASELECT %d\n", hand);
    return frame;
}


Clock::Clock(vector<frame_t>* frame_table, bool is_print) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->hand = -1;
}

frame_t* Clock::select_victim_frame() {
    hand = (hand + 1) % frame_size;
    int start = hand, lapse = 1;
    while ((*this->frame_table)[hand].pte->referenced) {
        (*this->frame_table)[hand].pte->referenced = 0;
        hand = (hand + 1) % frame_size;
        lapse = lapse + 1;
    }
    if (is_print) printf("ASELECT %d %d\n", start, lapse);
    frame_t* frame = &(*this->frame_table)[hand];
    return frame;
}


Random::Random(vector<frame_t>* frame_table, bool is_print, vector<int> *randvals) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->randvals = randvals;
    this->rofs = 0;
}

frame_t* Random::select_victim_frame() {
    int rnum = (*this->randvals)[rofs++];
    if (rofs == this->randvals->size()) rofs = 0;
    int idx = rnum % frame_size;
    return &(*this->frame_table)[idx];
}


NRU::NRU(vector<frame_t> *frame_table, bool is_print) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->hand = 0;
}

frame_t* NRU::select_victim_frame() {
    bool reset = false;
    int class0 = -1, class1 = -1, class2 = -1, class3 = -1, target = -1;
    if (this->NRU_cnt >= 48) {
        reset = true;
        this->NRU_cnt = 0;
    }
    int start = hand, num_scanned = 0;
    for (int i=0; i<frame_size; i++) {
        int idx = (hand + i) % frame_size;
        frame_t* frame = &(*this->frame_table)[idx];
        int class_idx = 2*frame->pte->referenced + frame->pte->modified;
        num_scanned = i+1;
        if (class_idx == 0 && class0 == -1) {
            class0 = idx;
            if (! reset) break;
        }
        if (class_idx == 1 && class1 == -1) class1 = idx;
        if (class_idx == 2 && class2 == -1) class2 = idx;
        if (class_idx == 3 && class3 == -1) class3 = idx;
        if (reset) frame->pte->referenced = 0;
    }
    if (class0 != -1) hand = class0, target = 0;
    else if (class1 != -1) hand = class1, target = 1;
    else if (class2 != -1) hand = class2, target = 2;
    else if (class3 != -1) hand = class3, target = 3;
    if (is_print) printf("ASELECT %d %d | %d %d %d\n", start, reset, target, hand, num_scanned);
    frame_t* frame = &(*this->frame_table)[hand];
    hand = (hand + 1) % frame_size;
    return frame;
}


Aging::Aging(vector<frame_t> *frame_table, bool is_print) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->hand = 0;
}

frame_t* Aging::select_victim_frame() {
    if (is_print) printf("ASELECT %d-%d | ", hand, (hand+frame_size-1) % frame_size);
    unsigned int min_time = 0xFFFFFFFF;
    int target = hand;
    for (int i=0; i<frame_size; i++) {
        int idx = (hand + i) % frame_size;
        frame_t* frame = &(*this->frame_table)[idx];
        frame->age = frame->age >> 1;
        if (frame->pte->referenced) {
            frame->age = (frame->age | 0x80000000);
            frame->pte->referenced = 0;
        }
        if (is_print) printf("%d:%x ", idx, frame->age);
        if (frame->age < min_time) {
            min_time = frame->age;
            target = idx;
        }
    }
    hand = target;
    if (is_print) printf("| %d\n", hand);
    frame_t* frame = &(*this->frame_table)[hand];
    hand = (hand + 1) % frame_size;
    return frame;
}


WorkingSet::WorkingSet(vector<frame_t> *frame_table, bool is_print) {
    this->frame_table = frame_table;
    this->frame_size = int(frame_table->size());
    this->is_print = is_print;
    this->hand = 0;
    this->TAU = 49;
}

frame_t* WorkingSet::select_victim_frame() {
    if (is_print) printf("ASELECT %d-%d | ", hand, (hand+frame_size-1) % frame_size);
    int target = hand, smallest_time = 0x7FFFFFFF;
    for (int i=0; i<frame_size; i++) {
        int idx = (hand + i) % frame_size;
        frame_t* frame = &(*this->frame_table)[idx];
        if (is_print) printf("%d(%d %d:%d %d) ", idx, frame->pte->referenced, frame->pid, frame->vpage, frame->time);
        if (frame->pte->referenced) {
            frame->time = instr_cnt;
            frame->pte->referenced = 0;
        }
        else {
            if (instr_cnt - frame->time > TAU) {
                if (is_print) printf("STOP(%d) ", i+1);
                target = idx;
                break;
            }
            else {
                if (frame->time < smallest_time) {
                    smallest_time = frame->time;
                    target = idx;
                }
            }
        }
    }
    hand = target;
    if (is_print) printf("| %d\n", hand);
    frame_t* frame = &(*this->frame_table)[hand];
    hand = (hand + 1) % frame_size;
    return frame;
}