#ifndef LAB2_SCHEDULER_H
#define LAB2_SCHEDULER_H
#include <deque>
#include "Process.h"
using namespace std;

typedef enum {TYPE_FCFS, TYPE_LCFS, TYPE_SRTF, TYPE_RR, TYPE_PRIO, TYPE_PREPRIO} SchedType;

class Scheduler {
public:
    SchedType type;
    int quantum, maxprio, highest_prio;
    deque<Process*> runQueue;
    deque<Process*> *activeQ, *expiredQ;
    Scheduler();
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
};


class FCFS: public Scheduler {
public:
    FCFS();
    void add_process(Process* proc);
    Process* get_next_process();
};


class LCFS: public Scheduler {
public:
    LCFS();
    void add_process(Process* proc);
    Process* get_next_process();
};


class SRTF: public Scheduler {
public:
    SRTF();
    void add_process(Process* proc);
    Process* get_next_process();
};


class RR: public Scheduler {
public:
    RR();
    RR(int quantum);
    void add_process(Process* proc);
    Process* get_next_process();
};


class PRIO: public Scheduler {
public:
    PRIO();
    PRIO(int quantum);
    PRIO(int quantum, int maxprio);
    void update_highest_prio();
    void add_process(Process* proc);
    Process* get_next_process();
};

class PREPRIO: public Scheduler {
public:
    PREPRIO();
    PREPRIO(int quantum);
    PREPRIO(int quantum, int maxprio);
    void update_highest_prio();
    void add_process(Process* proc);
    Process* get_next_process();
};



#endif //LAB2_SCHEDULER_H
