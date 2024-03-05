#include <cstdio>
#include <unistd.h>
#include <string>
#include <vector>
#include <deque>
#include "Process.h"
#include "Event.h"
#include "Scheduler.h"
using namespace std;

typedef deque<Event*>::iterator deqIt;
typedef deque<Process*>::iterator deqItp;

Event* evt;
deque<Event*> evtQueue;
Process* proc;
Process* CURRENT_RUNNING_PROCESS = 0;
vector<Process*> proc_list;
vector<int> randvals;
Scheduler* THE_SCHEDULER;

string filename, rfilename;
int AT, TC, CB, IO;
int num_rands, ofs = 0, quantum = -1, maxprio = 4;
int time_cpubusy = 0, time_iobusy = 0, num_processes = 0, finishtime = 0;
int PREVIOUS_TIME = 0, CURRENT_TIME = 0, NUM_BLOCKED_EVT = 0, NUM_BLOCKED_PROC = 0, NUM_PREEMPT = 0;
bool CALL_SCHEDULER, is_printE = false, is_printV = false, is_printP = false, is_printT = false;
char type;

void Initialization();
void readProcess();
void printEvtQueue();
void printV(int timestamp, int howlong, State oldState, Process* proc);
void printP(bool cond1, bool cond2, int time);
void printT();
void printSUM();
void put_event(Event* event);
void remove_event(int pid);
Event* get_event();
int get_next_event_time();
int get_next_pending_event_time(int pid);
int myrandom(int burst);
void Simulation();
string stateToString(State state);
string evtStateToString(Event* event);


int main(int argc, char *argv[]) {
    int c;
    while ((c = getopt (argc, argv, "vteps:")) != -1)
        switch (c) {
            case 'v':
                is_printV = true; break;
            case 't':
                is_printT = true; break;
            case 'e':
                is_printE = true; break;
            case 'p':
                is_printP = true; break;
            case 's':
                sscanf(optarg, "%c%d:%d", &type, &quantum, &maxprio); break;
            case '?':
                if (optopt == 's') {
                    fprintf (stderr, "Unknown Scheduler spec.\n");
                    return 1;
                }
            default:
                break;
        }
    if (type != 'F' && type !='L' && type != 'S' && type != 'R' && type == 'P' && type == 'E') {
        fprintf (stderr, "Unknown Scheduler spec.\n");
        return 1;
    }
    if (type == 'R' || type == 'P' || type == 'E') {
        if (quantum == -1) {
            fprintf (stderr, "Invalid scheduler param <%c>.\n", type);
            return 1;
        }
    }
    if (optind == argc) {
        fprintf(stderr, "Missing filename and randfile.\n");
        return 1;
    }
    else if (optind == argc-1) {
        fprintf(stderr, "Missing randfile.\n");
        return 1;
    }
    else {
        filename = argv[optind];
        rfilename = argv[optind + 1];
    }
    Initialization();
    readProcess();
    Simulation();
    printSUM();
    return 0;
}


void Initialization() {
    freopen(rfilename.c_str(), "r", stdin);
    scanf("%d", &num_rands);
    int rand;
    for (int i=0; i<num_rands; i++) {
        scanf("%d", &rand);
        randvals.push_back(rand);
    }
    fclose(stdin);
    switch (type){
        case 'F':
            THE_SCHEDULER = new FCFS();
            break;
        case 'L':
            THE_SCHEDULER = new LCFS();
            break;
        case 'S':
            THE_SCHEDULER = new SRTF();
            break;
        case 'R':
            THE_SCHEDULER = new RR(quantum);
            break;
        case 'P':
            THE_SCHEDULER = new PRIO(quantum, maxprio);
            break;
        case 'E':
            THE_SCHEDULER = new PREPRIO(quantum, maxprio);
            break;
    }
}


void readProcess() {
    freopen(filename.c_str(), "r", stdin);
    while (scanf("%d%d%d%d", &AT, &TC, &CB, &IO) != EOF) {
        proc = new Process(AT, TC, CB, IO);
        proc->static_priority = myrandom(THE_SCHEDULER->maxprio);
        proc->dynamic_priority = proc->static_priority - 1;
        evt = new Event(proc, proc->AT, STATE_CREATED, STATE_READY);
        evtQueue.push_back(evt);
        proc_list.push_back(proc);
        num_processes = num_processes + 1;
    }
    fclose(stdin);
    if (is_printE) {
        deqIt it;
        printf("ShowEventQ:");
        for (it = evtQueue.begin(); it!=evtQueue.end(); ++it)
            printf("  %d:%d", (*it)->evtTimeStamp, (*it)->evtProcess->pid);
        printf("\n");
    }
}


void printEvtQueue() {
    deqIt it;
    for (it = evtQueue.begin(); it!=evtQueue.end(); ++it)
        printf("%d:%d:%s ", (*it)->evtTimeStamp, (*it)->evtProcess->pid, evtStateToString((*it)).c_str());
}


void printV(int timestamp, int howlong, State oldState, Process* proc) {
    if (! is_printV) return;
    if (proc->rem == 0)
        printf("%d %d %d: Done\n", timestamp, proc->pid, howlong);
    else {
        printf("%d %d %d: %s -> %s", timestamp, proc->pid, howlong, stateToString(oldState).c_str(), stateToString(proc->state).c_str());
        switch (proc->state) {
            case STATE_RUNNING:
                printf(" cb=%d rem=%d prio=%d\n", proc->cpu_burst, proc->rem, proc->dynamic_priority);
                break;
            case STATE_BLOCKED:
                printf("  ib=%d rem=%d\n", proc->io_burst, proc->rem);
                break;
            case STATE_READY:
                if (oldState == STATE_RUNNING)
                    printf(" cb=%d rem=%d prio=%d\n", proc->cpu_burst, proc->rem, proc->dynamic_priority);
                else
                    printf("\n");
                break;
            default:
                printf("\n");
                break;
        }
    }
}



void printP(bool cond1, bool cond2, int time) {
    if (! is_printP) return;
    string res = (cond1 && cond2)? "YES" : "NO";
    printf("    --> PrioPreempt Cond1=%d Cond2=%d (%d) --> %s\n", cond1, cond2, time, res.c_str());
}


void printT() {
    if (! is_printT) return;
    deqItp it;
    switch (THE_SCHEDULER->type) {
        case TYPE_FCFS:
        case TYPE_LCFS:
        case TYPE_SRTF:
        case TYPE_RR:
            printf("SCHED (%d): ", int(THE_SCHEDULER->runQueue.size()));
            for (it = THE_SCHEDULER->runQueue.begin(); it!=THE_SCHEDULER->runQueue.end(); ++it)
                printf(" %d:%d", (*it)->pid, (*it)->state_ts);
            printf("\n");
            break;
        case TYPE_PRIO:
        case TYPE_PREPRIO:
            printf("{");
            for (int i=THE_SCHEDULER->maxprio-1; i>=0; i--) {
                printf("[");
                if (! THE_SCHEDULER->activeQ[i].empty()) {
                    printf("%d", THE_SCHEDULER->activeQ[i].front()->pid);
                    for (it = THE_SCHEDULER->activeQ[i].begin()+1; it!=THE_SCHEDULER->activeQ[i].end(); ++it)
                        printf(",%d", (*it)->pid);
                }
                printf("]");
            }
            printf("} : {");
            for (int i=THE_SCHEDULER->maxprio-1; i>=0; i--) {
                printf("[");
                if (! THE_SCHEDULER->expiredQ[i].empty()) {
                    printf("%d", THE_SCHEDULER->expiredQ[i].front()->pid);
                    for (it = THE_SCHEDULER->expiredQ[i].begin()+1; it!=THE_SCHEDULER->expiredQ[i].end(); ++it)
                        printf(",%d", (*it)->pid);
                }
                printf("]");
            }
            printf("}\n");
            break;
    }

}


void printSUM() {
    Process* proc;
    switch (THE_SCHEDULER->type) {
        case TYPE_FCFS:
            printf("FCFS\n");
            break;
        case TYPE_LCFS:
            printf("LCFS\n");
            break;
        case TYPE_SRTF:
            printf("SRTF\n");
            break;
        case TYPE_RR:
            printf("RR %d\n", THE_SCHEDULER->quantum);
            break;
        case TYPE_PRIO:
            printf("PRIO %d\n", THE_SCHEDULER->quantum);
            break;
        case TYPE_PREPRIO:
            printf("PREPRIO %d\n", THE_SCHEDULER->quantum);
            break;
    }
    double total_TT = 0, total_CW = 0;
    for (int i=0; i<proc_list.size(); i++) {
        proc = proc_list[i];
        total_TT = total_TT + proc->TT;
        total_CW = total_CW + proc->CW;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", proc->pid, proc->AT, proc->TC, proc->CB, proc->IO, proc->static_priority, proc->FT, proc->TT, proc->IT, proc->CW);
    }
    double cpu_util = 100.0 * (time_cpubusy / (double) finishtime);
    double io_util = 100.0 * (time_iobusy / (double) finishtime);
    double throughput = 100.0 * (num_processes / (double) finishtime);
    double avg_TT = total_TT / (double) num_processes;
    double avg_CW = total_CW / (double) num_processes;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", finishtime, cpu_util, io_util, avg_TT, avg_CW, throughput);
}


void put_event(Event* event) {
    if (is_printE) {
        printf("  AddEvent(%d:%d:%s):  ", event->evtTimeStamp, event->evtProcess->pid, evtStateToString(event).c_str());
        printEvtQueue();
        printf("==>  ");
    }
    deqIt it, pos = evtQueue.end();
    for (it = evtQueue.begin(); it != evtQueue.end(); ++it)
        if ((*it)->evtTimeStamp > event->evtTimeStamp) {
            pos = it;
            break;
        }
    evtQueue.insert(pos, event);
    if (event->transition == TRANS_TO_PREEMPT)
        NUM_PREEMPT++;
    if (event->evtOldState == STATE_BLOCKED)
        NUM_BLOCKED_PROC++;
    if (event->evtNewState == STATE_BLOCKED)
        NUM_BLOCKED_EVT++;
    if (is_printE) {
        printEvtQueue();
        printf("\n");
    }
}


void remove_event(int pid) {
    deqIt it;
    for (it = evtQueue.begin(); it != evtQueue.end(); ++it)
        if ((*it)->evtProcess->pid == pid)
            break;
    Event* event = (*it);
    if (is_printP)
        printf("RemoveEvent(%d): \n", event->evtProcess->pid);
    evtQueue.erase(it);
    if (event->transition == TRANS_TO_PREEMPT)
        NUM_PREEMPT--;
    if (event->evtOldState == STATE_BLOCKED)
        NUM_BLOCKED_PROC--;
    if (event->evtNewState == STATE_BLOCKED)
        NUM_BLOCKED_EVT--;
}


Event* get_event() {
    if (evtQueue.empty()) return 0;
    Event* event = evtQueue.front();
    evtQueue.pop_front();
    if (event->transition == TRANS_TO_PREEMPT)
        NUM_PREEMPT--;
    if (event->evtOldState == STATE_BLOCKED)
        NUM_BLOCKED_PROC--;
    if (event->evtNewState == STATE_BLOCKED)
        NUM_BLOCKED_EVT--;
    return event;
}

int get_next_event_time() {
    if (evtQueue.empty()) return -1;
    else return evtQueue.front()->evtTimeStamp;
}

int get_next_pending_event_time(int pid) {
    deqIt it;
    for (it = evtQueue.begin(); it!=evtQueue.end(); ++it)
        if ((*it)->evtProcess->pid == pid)
            return (*it)->evtTimeStamp;
    return -1;
}


int myrandom(int burst) {
    int res = 1 + (randvals[ofs] % burst);
    ofs = (ofs + 1) % num_rands;
    return res;
}

string evtStateToString(Event* event) {
    switch (event->evtNewState) {
        case STATE_CREATED:
            return "CREATED";
        case STATE_READY:
            if (event->evtOldState == STATE_RUNNING) return "PREEMPT";
            else return "READY";
        case STATE_RUNNING:
            return "RUNNG";
        case STATE_BLOCKED:
            return "BLOCK";
    }
}

string stateToString(State state) {
    switch (state) {
        case STATE_CREATED:
            return "CREATED";
        case STATE_READY:
            return "READY";
        case STATE_RUNNING:
            return "RUNNG";
        case STATE_BLOCKED:
            return "BLOCK";
    }
}

void Simulation() {
    Event *evt, *newEvt;
    Process* nextProc;
    State preState;
    while ((evt = get_event())) {
        Process* proc = evt->evtProcess;
        PREVIOUS_TIME = CURRENT_TIME;
        CURRENT_TIME = evt->evtTimeStamp;
        StateTransition transition = evt->transition;
        int timeInPrevState = CURRENT_TIME - proc->state_ts;
        delete evt; evt = 0;
        preState = proc->state;
        if (NUM_BLOCKED_PROC > 0 || preState == STATE_BLOCKED)
            time_iobusy = time_iobusy + (CURRENT_TIME - PREVIOUS_TIME);
        if (CURRENT_RUNNING_PROCESS != 0)
            time_cpubusy = time_cpubusy + (CURRENT_TIME - PREVIOUS_TIME);

        switch (transition) {
            case TRANS_TO_READY:
                // must come from BLOCKED or CREATED
                // must add to run queue, no event created
                proc->state = STATE_READY;
                proc->state_ts = CURRENT_TIME;
                proc->dynamic_priority = proc->static_priority - 1;
                THE_SCHEDULER->add_process(proc);
                printV(CURRENT_TIME, timeInPrevState, preState, proc);
                // Special for PREPRIO !!!!!!!!!!!!
                if (THE_SCHEDULER->type == TYPE_PREPRIO) {
                    if (CURRENT_RUNNING_PROCESS != 0) {
                        bool cond1 = proc->dynamic_priority > CURRENT_RUNNING_PROCESS->dynamic_priority;
                        int pending_time = get_next_pending_event_time(CURRENT_RUNNING_PROCESS->pid) - CURRENT_TIME;
                        bool cond2 = pending_time > 0;
                        printP(cond1, cond2, pending_time);
                        if (cond1 && cond2) {
                            remove_event(CURRENT_RUNNING_PROCESS->pid);
                            CURRENT_RUNNING_PROCESS->rem = CURRENT_RUNNING_PROCESS->rem + pending_time;
                            newEvt = new Event(CURRENT_RUNNING_PROCESS, CURRENT_TIME, STATE_RUNNING, STATE_READY);
                            put_event(newEvt);
                            CURRENT_RUNNING_PROCESS->cpu_burst = CURRENT_RUNNING_PROCESS->cpu_burst + pending_time;
                        }
                    }
                }
                if (NUM_BLOCKED_EVT == 0 && NUM_PREEMPT == 0)
                    CALL_SCHEDULER = true;
                break;
            case TRANS_TO_PREEMPT: // similar to TRANS_TO_READY
                // must come from RUNNING (preemption)
                // must add to run queue, no event created
                proc->state = STATE_READY;
                proc->state_ts = CURRENT_TIME;
                proc->dynamic_priority = proc->dynamic_priority - 1;
                if (THE_SCHEDULER->type == TYPE_RR)
                    proc->dynamic_priority = proc->static_priority - 1;
                if (proc == CURRENT_RUNNING_PROCESS)
                    CURRENT_RUNNING_PROCESS = 0;
                THE_SCHEDULER->add_process(proc);
                printV(CURRENT_TIME, timeInPrevState, preState, proc);
                if (NUM_BLOCKED_EVT == 0 && NUM_PREEMPT == 0)
                    CALL_SCHEDULER = true;
                break;
            case TRANS_TO_RUN:
                // create event for either preemption or blocking
                if (proc->rem > 0) {
                    if (proc->cpu_burst <= THE_SCHEDULER->quantum) {
                        proc->rem = proc->rem - proc->cpu_burst;
                        newEvt = new Event(proc, CURRENT_TIME+proc->cpu_burst, STATE_RUNNING, STATE_BLOCKED);
                        put_event(newEvt);
                        proc->cpu_burst = 0;
                    }
                    else {// !!!!!!!!!!!!! pay attention to preemption
                        proc->rem = proc->rem - THE_SCHEDULER->quantum;
                        newEvt = new Event(proc, CURRENT_TIME+THE_SCHEDULER->quantum, STATE_RUNNING, STATE_READY);
                        put_event(newEvt);
                        proc->cpu_burst = proc->cpu_burst - THE_SCHEDULER->quantum;
                    }
                }
                break;
            case TRANS_TO_BLOCK:
                // create an event for when process becomes READY again
                proc->state = STATE_BLOCKED;
                proc->state_ts = CURRENT_TIME;
                if (proc == CURRENT_RUNNING_PROCESS)
                    CURRENT_RUNNING_PROCESS = 0;
                CALL_SCHEDULER = true;
                if (proc->rem > 0) {
                    proc->io_burst = myrandom(proc->IO);
                    proc->IT = proc->IT + proc->io_burst;
                    printV(CURRENT_TIME, timeInPrevState, preState, proc);
                    newEvt = new Event(proc, CURRENT_TIME+proc->io_burst, STATE_BLOCKED, STATE_READY);
                    put_event(newEvt);
                }
                else {
                    finishtime = max(finishtime, CURRENT_TIME);
                    proc->FT = CURRENT_TIME;
                    proc->TT = proc->FT - proc->AT;
                    printV(CURRENT_TIME, timeInPrevState, preState, proc);
                }
                break;
        }



        if (CALL_SCHEDULER) {
            if (get_next_event_time() == CURRENT_TIME)
                continue; //process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            printT();
            if (CURRENT_RUNNING_PROCESS == 0)
                CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
            if (CURRENT_RUNNING_PROCESS == 0)
                continue;
            // create event to make this process runnable for same time.
            preState = STATE_READY;
            int timeInPrevState_1 = CURRENT_TIME - CURRENT_RUNNING_PROCESS->state_ts;
            CURRENT_RUNNING_PROCESS->state = STATE_RUNNING;
            CURRENT_RUNNING_PROCESS->state_ts = CURRENT_TIME;
            CURRENT_RUNNING_PROCESS->CW = CURRENT_RUNNING_PROCESS->CW + timeInPrevState_1;
            newEvt = new Event(CURRENT_RUNNING_PROCESS, CURRENT_TIME, STATE_READY, STATE_RUNNING);
            put_event(newEvt);
            if (CURRENT_RUNNING_PROCESS->cpu_burst == 0) {
                CURRENT_RUNNING_PROCESS->cpu_burst = myrandom(CURRENT_RUNNING_PROCESS->CB);
                if (CURRENT_RUNNING_PROCESS->rem < CURRENT_RUNNING_PROCESS->cpu_burst)
                    CURRENT_RUNNING_PROCESS->cpu_burst = CURRENT_RUNNING_PROCESS->rem;
            }
            printV(CURRENT_TIME, timeInPrevState_1, preState, CURRENT_RUNNING_PROCESS);
        }
    }
}