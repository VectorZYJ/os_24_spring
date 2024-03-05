#include "Event.h"


Event::Event() {
    this->evtProcess = 0;
    this->evtOldState = STATE_CREATED;
    this->evtNewState = STATE_READY;
    this->transition = setTransition();
    this->evtTimeStamp = 0;
}

Event::Event(Process* process, int timestamp, State oldState, State newState) {
    this->evtProcess = process;
    this->evtOldState = oldState;
    this->evtNewState = newState;
    this->transition = setTransition();
    this->evtTimeStamp = timestamp;
}

StateTransition Event::setTransition() const {
    switch (this->evtOldState) {
        case STATE_CREATED:
            return TRANS_TO_READY;
        case STATE_READY:
            return TRANS_TO_RUN;
        case STATE_RUNNING:
            if (this->evtNewState == STATE_BLOCKED) return TRANS_TO_BLOCK;
            else return TRANS_TO_PREEMPT;
        case STATE_BLOCKED:
            return TRANS_TO_READY;
    }
}