#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <unistd.h>
#include "Scheduler.h"
using namespace std;

typedef enum {T_ADD, T_ISSUE, T_FINISH, T_GET} t_info;

string filename = "../input0", line;
vector<t_req*> req_list;
deque<t_req*> arrive_queue;
ifstream file;
Scheduler* THE_SCHEDULER;
t_req* CURRENT_REQ;

char type = 'N';
int CURRENT_TIME = 0;
int print_v = false, print_q = false, print_f = false;
int total_time = 0, tot_movement = 0, max_waittime = 0;
double io_utilization = 0, avg_turnaround = 0, avg_waittime = 0;

void read_operations();
void initialize();
void simulation();
void print_info(t_info type, int time, int id, int para1, int para2);
void print_req();
void print_sum();


int main(int argc, char *argv[]) {
    int c;
    while ((c = getopt (argc, argv, "vqfs::")) != -1)
        switch (c) {
            case 'v':
                print_v = true; break;
            case 'q':
                print_q = true; break;
            case 'f':
                print_f = true; break;
            case 's':
                if (optarg) sscanf(optarg, "%c", &type);
                else type = 'N';
                break;
            case '?':
                fprintf (stderr, "Unknown Option.\n");
                break;
            default:
                break;
        }
    if (type != 'N' && type !='S' && type != 'L' && type != 'C' && type != 'F') {
        fprintf (stderr, "Unknown Scheduler type.\n");
        return 1;
    }
    if (optind == argc) {
        fprintf(stderr, "Missing filename.\n");
        return 1;
    }
    else filename = argv[optind];

    read_operations();
    initialize();
    simulation();
    print_req();
    print_sum();
    return 0;
}


void simulation() {
    if (print_v) printf("TRACE\n");
    while (true) {
        t_req* req = NULL;
        if (! arrive_queue.empty())
            req = arrive_queue.front();
        // A new IO arrived at the system at this current time
        if (req != NULL && req->arr_time == CURRENT_TIME) {
            print_info(T_ADD, CURRENT_TIME, req->id, req->track, 0);
            THE_SCHEDULER->add_request(req);
            arrive_queue.pop_front();
        }
        // An IO is qctive and completed at this time
        if (CURRENT_REQ != NULL && THE_SCHEDULER->head == CURRENT_REQ->track) {
            CURRENT_REQ->end_time = CURRENT_TIME;
            print_info(T_FINISH, CURRENT_TIME, CURRENT_REQ->id, CURRENT_REQ->end_time - CURRENT_REQ->arr_time, 0);
            CURRENT_REQ = NULL;
        }
        // No IO request active now
        if (CURRENT_REQ == NULL) {
            // Exist pending requests
            if (THE_SCHEDULER->have_pending_request()) {
                CURRENT_REQ = THE_SCHEDULER->get_next_request();
                if (THE_SCHEDULER->active != -1)
                    print_info(T_GET, CURRENT_TIME, CURRENT_REQ->id, THE_SCHEDULER->active, 0);
                print_info(T_ISSUE, CURRENT_TIME, CURRENT_REQ->id, CURRENT_REQ->track, THE_SCHEDULER->head);
                CURRENT_REQ->start_time = CURRENT_TIME;
            }
            // All IO requests from input are processed
            else if (arrive_queue.empty()) {
                total_time = CURRENT_TIME;
                break;
            }
        }
        // An IO is active
        if (CURRENT_REQ != NULL) {
            if (CURRENT_REQ->track == THE_SCHEDULER->head)
                continue;
            THE_SCHEDULER->head += THE_SCHEDULER->direction;
            tot_movement++;
        }
        CURRENT_TIME++;
    }
}


void initialize() {
    switch(type) {
        case 'N':
            THE_SCHEDULER = new FIFO();
            break;
        case 'S':
            THE_SCHEDULER = new SSTF();
            break;
        case 'L':
            THE_SCHEDULER = new LOOK();
            break;
        case 'C':
            THE_SCHEDULER = new CLOOK();
            break;
        case 'F':
            THE_SCHEDULER = new FLOOK();
            break;
    }
    THE_SCHEDULER->print_q = print_q;
}


void read_operations() {
    file.open(filename.c_str());
    int idx = 0;
    while (getline(file, line) && line[0] == '#') { }
    do {
        t_req* req = new t_req;
        sscanf(line.c_str(), "%d %d", &req->arr_time, &req->track);
        req->id = idx++;
        arrive_queue.push_back(req);
        req_list.push_back(req);
    } while (getline(file, line));
}


void print_info(t_info msg_type, int time, int id, int para1, int para2) {
    if (print_v) {
        switch (msg_type) {
            case T_ADD:
                printf("%d:%6d add %d\n", time, id, para1);
                break;
            case T_ISSUE:
                printf("%d:%6d issue %d %d\n", time, id, para1, para2);
                break;
            case T_FINISH:
                printf("%d:%6d finish %d\n", time, id, para1);
                break;
            case T_GET:
                break;
        }
    }
    if (print_f && msg_type == T_GET) printf("%d:%6d get Q=%d\n", time, id, para1);
}


void print_req() {
    for (int i=0; i<req_list.size(); i++) {
        t_req* req = req_list[i];
        printf("%5d: %5d %5d %5d\n", req->id, req->arr_time, req->start_time, req->end_time);
    }
}


void print_sum() {
    long long total_waittime = 0, total_turnaround = 0;
    int num_req = req_list.size();
    for (int i=0; i<num_req; i++) {
        t_req* req = req_list[i];
        total_turnaround += req->end_time - req->arr_time;
        total_waittime += req->start_time - req->arr_time;
        max_waittime = max(max_waittime, req->start_time - req->arr_time);
    }
    io_utilization = double(tot_movement) / total_time;
    avg_turnaround = double(total_turnaround) / num_req;
    avg_waittime = double(total_waittime) / num_req;
    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n",
           total_time, tot_movement, io_utilization, avg_turnaround, avg_waittime, max_waittime);
}