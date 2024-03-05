//
// Created by Yujia Zhu on 2/24/24.
//

#include "Process.h"

int Process::id = 0;

Process::Process() {
    this->pid = 0;
    this->AT = 0;
    this->TC = 0;
    this->CB = 0;
    this->IO = 0;
    this->FT = 0;
    this->TT = 0;
    this->IT = 0;
    this->CW = 0;
    this->rem = 0;
    this->cpu_burst = 0;
    this->io_burst = 0;
    this->static_priority = 0;
    this->dynamic_priority = 0;
    this->state_ts = 0;
    this->state = STATE_CREATED;
}

Process::Process(int AT, int TC, int CB, int IO) {
    this->pid = id++;
    this->AT = AT;
    this->TC = TC;
    this->CB = CB;
    this->IO = IO;
    this->FT = 0;
    this->TT = 0;
    this->IT = 0;
    this->CW = 0;
    this->rem = TC;
    this->cpu_burst = 0;
    this->io_burst = 0;
    this->static_priority = 0;
    this->dynamic_priority = 0;
    this->state_ts = AT;
    this->state = STATE_CREATED;
}
