//
// Created by Yujia Zhu on 2/25/24.
//

#include "Scheduler.h"

Scheduler::Scheduler() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_FCFS;
}

void Scheduler::add_process(Process *proc) {

}

Process* Scheduler::get_next_process() {
    return 0;
}


FCFS::FCFS() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_FCFS;
    this->runQueue.clear();
}

void FCFS::add_process(Process *proc) {
    this->runQueue.push_back(proc);
}

Process* FCFS::get_next_process() {
    if (this->runQueue.empty()) return 0;
    Process* p = this->runQueue.front();
    this->runQueue.pop_front();
    return p;
}


LCFS::LCFS() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_LCFS;
    this->runQueue.clear();
}

void LCFS::add_process(Process *proc) {
    this->runQueue.push_back(proc);
}

Process* LCFS::get_next_process() {
    if (this->runQueue.empty()) return 0;
    Process* p = this->runQueue.back();
    this->runQueue.pop_back();
    return p;
}


SRTF::SRTF() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_SRTF;
    this->runQueue.clear();
}

void SRTF::add_process(Process *proc) {
    deque<Process*>::iterator it, pos;
    pos = this->runQueue.end();
    for (it = this->runQueue.begin(); it != this->runQueue.end(); ++it)
        if ((*it)->rem > proc->rem) {
            pos = it;
            break;
        }
    this->runQueue.insert(pos, proc);
}

Process* SRTF::get_next_process() {
    if (this->runQueue.empty()) return 0;
    Process* p = this->runQueue.front();
    this->runQueue.pop_front();
    return p;
}


RR::RR() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_RR;
    this->runQueue.clear();
}

RR::RR(int quantum) {
    this->quantum = quantum;
    this->maxprio = 4;
    this->type = TYPE_RR;
    this->runQueue.clear();
}

void RR::add_process(Process *proc) {
    this->runQueue.push_back(proc);
}

Process* RR::get_next_process() {
    if (this->runQueue.empty()) return 0;
    Process* p = this->runQueue.front();
    this->runQueue.pop_front();
    return p;
}



PRIO::PRIO() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_PRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

PRIO::PRIO(int quantum) {
    this->quantum = quantum;
    this->maxprio = 4;
    this->type = TYPE_PRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

PRIO::PRIO(int quantum, int maxprio) {
    this->quantum = quantum;
    this->maxprio = maxprio;
    this->type = TYPE_PRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

void PRIO::update_highest_prio() {
    for (int i=this->maxprio-1; i>=0; i--)
        if (! this->activeQ[i].empty()) {
            this->highest_prio = i;
            return;
        }
    this->highest_prio = -1;
}

void PRIO::add_process(Process* proc) {
    if (proc->dynamic_priority < 0) {
        proc->dynamic_priority = proc->static_priority - 1;
        this->expiredQ[proc->dynamic_priority].push_back(proc);
    }
    else {
        this->activeQ[proc->dynamic_priority].push_back(proc);
        update_highest_prio();
    }
}

Process* PRIO::get_next_process() {
    if (this->highest_prio == -1) {
        swap(this->activeQ, this->expiredQ);
        // printf("switched queues\n");
        update_highest_prio();
    }
    if (this->highest_prio == -1) return 0;
    if (this->activeQ[this->highest_prio].empty()) return 0;
    Process* p = this->activeQ[this->highest_prio].front();
    this->activeQ[this->highest_prio].pop_front();
    update_highest_prio();
    return p;
}



PREPRIO::PREPRIO() {
    this->quantum = 10000;
    this->maxprio = 4;
    this->type = TYPE_PREPRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

PREPRIO::PREPRIO(int quantum) {
    this->quantum = quantum;
    this->maxprio = 4;
    this->type = TYPE_PREPRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

PREPRIO::PREPRIO(int quantum, int maxprio) {
    this->quantum = quantum;
    this->maxprio = maxprio;
    this->type = TYPE_PREPRIO;
    this->activeQ = new deque<Process*> [this->maxprio];
    this->expiredQ = new deque<Process*> [this->maxprio];
    this->highest_prio = -1;
}

void PREPRIO::update_highest_prio() {
    for (int i=this->maxprio-1; i>=0; i--)
        if (! this->activeQ[i].empty()) {
            this->highest_prio = i;
            return;
        }
    this->highest_prio = -1;
}

void PREPRIO::add_process(Process* proc) {
    if (proc->dynamic_priority < 0) {
        proc->dynamic_priority = proc->static_priority - 1;
        this->expiredQ[proc->dynamic_priority].push_back(proc);
    }
    else {
        this->activeQ[proc->dynamic_priority].push_back(proc);
        update_highest_prio();
    }
}

Process* PREPRIO::get_next_process() {
    if (this->highest_prio == -1) {
        swap(this->activeQ, this->expiredQ);
        // printf("switched queues\n");
        update_highest_prio();
    }
    if (this->highest_prio == -1) return 0;
    if (this->activeQ[this->highest_prio].empty()) return 0;
    Process* p = this->activeQ[this->highest_prio].front();
    this->activeQ[this->highest_prio].pop_front();
    update_highest_prio();
    return p;
}