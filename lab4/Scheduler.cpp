//
// Created by Yujia Zhu on 4/25/24.
//

#include <cstdio>
#include <cmath>
#include <climits>
#include "Scheduler.h"

Scheduler::Scheduler() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
    add = -1;
    active = -1;
    Q = NULL;
}

bool Scheduler::have_pending_request() {
    return (! IO_queue.empty());
}

void Scheduler::add_request(t_req *req) {
    IO_queue.push_back(req);
}

t_req* Scheduler::get_next_request() {
    return NULL;
}



FIFO::FIFO() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
}

t_req* FIFO::get_next_request() {
    if (IO_queue.empty()) return NULL;
    deque<t_req*>::iterator it;
    if (print_q) {
        printf("        Get: (");
        for (it = IO_queue.begin(); it != IO_queue.end(); ++it)
            printf("%d:%d ", (*it)->id, abs((*it)->track - head));
    }

    t_req* req = IO_queue.front();
    IO_queue.pop_front();
    if (print_q) printf(") --> %d\n", req->id);

    if (req->track > head) direction = 1;
    if (req->track < head) direction = -1;
    return req;
}



SSTF::SSTF() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
}

t_req* SSTF::get_next_request() {
    if (IO_queue.empty()) return NULL;
    deque<t_req*>::iterator it, pos;
    int min_distance = INT_MAX;
    if (print_q) printf("        Get: (");
    for (it = IO_queue.begin(); it != IO_queue.end(); ++it) {
        int dist = abs((*it)->track - head);
        if (dist < min_distance) {
            min_distance = dist;
            pos = it;
        }
        if (print_q) printf("%d:%d ", (*it)->id, dist);
    }
    t_req* req = (*pos);
    IO_queue.erase(pos);
    if (print_q) printf(") --> %d\n", req->id);

    if (req->track > head) direction = 1;
    if (req->track < head) direction = -1;
    return req;
}



LOOK::LOOK() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
}

t_req* LOOK::get_next_request() {
    if (IO_queue.empty()) return NULL;
    deque<t_req*>::iterator it, pos;

    int min_distance = INT_MAX;

    if (print_q) printf("        Get: (");
    for (it = IO_queue.begin(); it != IO_queue.end(); ++it) {
        int dist = (*it)->track - head;
        if (dist*direction >= 0) {
            if (abs(dist) < min_distance) {
                min_distance = abs(dist);
                pos = it;
            }
            if (print_q) printf("%d:%d ", (*it)->id, abs(dist));
        }
    }

    if (min_distance == INT_MAX) {
        direction *= -1;
        if (print_q) printf(") --> Change direction to %d\n", direction);
        if (print_q) printf("        Get: (");
        for (it = IO_queue.begin(); it != IO_queue.end(); ++it) {
            int dist = (*it)->track - head;
            if (dist*direction >= 0) {
                if (abs(dist) < min_distance) {
                    min_distance = abs(dist);
                    pos = it;
                }
                if (print_q) printf("%d:%d ", (*it)->id, abs(dist));
            }
        }
    }
    t_req* req = (*pos);
    IO_queue.erase(pos);
    if (print_q) printf(") --> %d dir=%d\n", req->id, direction);
    return req;
}



CLOOK::CLOOK() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
}

t_req* CLOOK::get_next_request() {
    if (IO_queue.empty()) return NULL;
    deque<t_req*>::iterator it, pos1, pos2;

    int min_distance = INT_MAX, lowest_track = INT_MAX;

    if (print_q) printf("        Get: (");
    for (it = IO_queue.begin(); it != IO_queue.end(); ++it) {
        int dist = (*it)->track - head;
        if (dist >= 0 && dist < min_distance) {
            min_distance = dist;
            pos1 = it;
        }
        if ((*it)->track < lowest_track) {
            lowest_track = (*it)->track;
            pos2 = it;
        }
        if (print_q) printf("%d:%d ", (*it)->id, dist);
    }
    t_req* req;
    if (min_distance == INT_MAX) {
        req = (*pos2);
        IO_queue.erase(pos2);
        if (print_q) printf(") --> go to bottom and pick %d\n", req->id);
        direction = -1;
    }
    else {
        req = (*pos1);
        IO_queue.erase(pos1);
        if (print_q) printf(") --> %d\n", req->id);
        direction = 1;
    }
    return req;
}



FLOOK::FLOOK() {
    IO_queue.clear();
    head = 0;
    direction = 1;
    print_q = false;
    add = 0;
    active = 1;
    Q = new deque<t_req*> [2];
}

void FLOOK::add_request(t_req *req) {
    Q[add].push_back(req);
    if (print_q) {
        deque<t_req*>::iterator it;
        printf("   Q=%d ( ", add);
        for (it = Q[add].begin(); it != Q[add].end(); ++it)
            printf("%d:%d ", (*it)->id, (*it)->track);
        printf(")\n");
    }
}

t_req* FLOOK::get_next_request() {
    if (Q[active].empty()) swap(active, add);
    if (Q[active].empty()) return NULL;

    if (print_q) {
        printf("AQ=%d dir=%d curtrack=%d:", active, direction, head);
        deque<t_req*>::iterator it;
        printf("  Q[0] = ( ");
        for (it = Q[0].begin(); it != Q[0].end(); ++it) {
            int dist = (*it)->track - head;
            if (dist*direction >= 0)
                printf("%d:%d:%d ", (*it)->id, (*it)->track, abs(dist));
            else
                printf("%d:%d:%d ", (*it)->id, (*it)->track, -abs(dist));
        }
        printf(")");
        printf("  Q[1] = ( ");
        for (it = Q[1].begin(); it != Q[1].end(); ++it) {
            int dist = (*it)->track - head;
            if (dist*direction >= 0)
                printf("%d:%d:%d ", (*it)->id, (*it)->track, abs(dist));
            else
                printf("%d:%d:%d ", (*it)->id, (*it)->track, -abs(dist));
        }
        printf(")\n");
    }

    deque<t_req*>::iterator it, pos;

    int min_distance = INT_MAX;

    if (print_q) printf("        Get: (");
    for (it = Q[active].begin(); it != Q[active].end(); ++it) {
        int dist = (*it)->track - head;
        if (dist*direction >= 0) {
            if (abs(dist) < min_distance) {
                min_distance = abs(dist);
                pos = it;
            }
            if (print_q) printf("%d:%d:%d ", (*it)->id, (*it)->track, abs(dist));
        }
        else if (print_q) printf("%d:%d:%d ", (*it)->id, (*it)->track, -abs(dist));
    }

    if (min_distance == INT_MAX) {
        direction *= -1;
        if (print_q) printf(") --> Change direction to %d\n", direction);
        if (print_q) printf("        Get: (");
        for (it = Q[active].begin(); it != Q[active].end(); ++it) {
            int dist = (*it)->track - head;
            if (dist*direction >= 0) {
                if (abs(dist) < min_distance) {
                    min_distance = abs(dist);
                    pos = it;
                }
                if (print_q) printf("%d:%d:%d ", (*it)->id, (*it)->track, abs(dist));
            }
            else if (print_q) printf("%d:%d:%d ", (*it)->id, (*it)->track, -abs(dist));
        }
    }
    t_req* req = (*pos);
    Q[active].erase(pos);
    if (print_q) printf(") --> %d dir=%d\n", req->id, direction);
    return req;
}

bool FLOOK::have_pending_request() {
    return (! (Q[add].empty() && Q[active].empty()));
}