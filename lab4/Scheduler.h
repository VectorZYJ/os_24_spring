//
// Created by Yujia Zhu on 4/25/24.
//

#ifndef LAB4_SCHEDULER_H
#define LAB4_SCHEDULER_H

#include <deque>
#include <vector>
using namespace std;

typedef struct {
    int id, track;
    int arr_time, start_time, end_time;
} t_req;


class Scheduler {
public:
    deque<t_req*> IO_queue;
    deque<t_req*> *Q;
    int head, direction;
    int add, active;
    bool print_q;
    Scheduler();
    virtual void add_request(t_req* req);
    virtual bool have_pending_request();
    virtual t_req* get_next_request();
};

class FIFO: public Scheduler {
public:
    FIFO();
    t_req* get_next_request();
};

class SSTF: public Scheduler {
public:
    SSTF();
    t_req* get_next_request();
};

class LOOK: public Scheduler {
public:
    LOOK();
    t_req* get_next_request();
};

class CLOOK: public Scheduler {
public:
    CLOOK();
    t_req* get_next_request();
};

class FLOOK: public Scheduler {
public:
    FLOOK();
    void add_request(t_req* req);
    bool have_pending_request();
    t_req* get_next_request();
};


#endif //LAB4_SCHEDULER_H
