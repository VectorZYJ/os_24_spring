#ifndef LAB2_EVENT_H
#define LAB2_EVENT_H

#include "Process.h"

class Event {
public:
    Event();
    Event(Process* process, int timestamp, State oldState, State newState);
    StateTransition setTransition() const;

    int evtTimeStamp;
    Process* evtProcess;
    State evtOldState, evtNewState;
    StateTransition transition;
};


#endif //LAB2_EVENT_H
