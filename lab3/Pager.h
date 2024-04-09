//
// Created by Yujia Zhu on 4/6/24.
//

#ifndef LAB3_PAGER_H
#define LAB3_PAGER_H

#include <vector>
#include "Process.h"

typedef struct {
    int id, pid, vpage, used, time;
    unsigned int age;
    pte_t* pte;
} frame_t;

class Pager {
public:
    vector<frame_t>* frame_table;
    bool is_print;
    int instr_cnt, NRU_cnt, frame_size;
    Pager();
    virtual frame_t* select_victim_frame();
};

class FIFO: public Pager {
public:
    int hand;
    FIFO(vector<frame_t>* frame_table, bool is_print);
    frame_t* select_victim_frame();
};

class Clock: public Pager {
public:
    int hand;
    Clock(vector<frame_t>* frame_table, bool is_print);
    frame_t* select_victim_frame();
};

class Random: public Pager {
public:
    int rofs;
    vector<int>* randvals;
    Random(vector<frame_t>* frame_table, bool is_print, vector<int>* randvals);
    frame_t* select_victim_frame();
};

class NRU: public Pager {
public:
    int hand;
    NRU(vector<frame_t>* frame_table, bool is_print);
    frame_t* select_victim_frame();
};

class Aging: public Pager {
public:
    int hand;
    Aging(vector<frame_t>* frame_table, bool is_print);
    frame_t* select_victim_frame();
};

class WorkingSet: public Pager {
public:
    int hand, TAU;
    WorkingSet(vector<frame_t>* frame_table, bool is_print);
    frame_t* select_victim_frame();
};

#endif //LAB3_PAGER_H
