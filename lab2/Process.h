#ifndef LAB2_PROCESS_H
#define LAB2_PROCESS_H

typedef enum {STATE_CREATED, STATE_READY, STATE_RUNNING, STATE_BLOCKED} State;
typedef enum {TRANS_TO_READY, TRANS_TO_PREEMPT, TRANS_TO_RUN, TRANS_TO_BLOCK} StateTransition;


class Process {
    static int id;
public:
    Process();
    Process(int AT, int TC, int CB, int IO);
    int pid;
    int AT, TC, CB, IO;
    int FT, TT, IT, CW;
    int cpu_burst, io_burst, rem, state_ts;
    int static_priority, dynamic_priority;
    State state;
};


#endif //LAB2_PROCESS_H
