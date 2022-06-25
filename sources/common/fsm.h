#ifndef COMMON_FSM_H_
#define COMMON_FSM_H_

#include <cstddef>

template <typename StateData, int STATES_NUM>
class FSM {
 public:
    typedef int StateIdx;
    typedef void (*TransitionStateFunc)(StateData*);
    typedef StateIdx (*TriggerFunc)(StateData*);

    typedef TriggerFunc Triggers[STATES_NUM];
    typedef TransitionStateFunc TransitionsMatrix[STATES_NUM][STATES_NUM];

 private:
    const Triggers *const             Trigers_;
    const TransitionsMatrix *const    TransitionMatrix_;

 public:
    FSM(const Triggers* triggers, const TransitionsMatrix* tmatrix)
    : Trigers_(triggers)
    , TransitionMatrix_(tmatrix) {
    }

    StateIdx    Process(StateData* data, StateIdx state) const {
        StateIdx             next_state;
        TransitionStateFunc  transition_func;

        while (data->GetRun()) {
            next_state = (*Trigers_)[state](data);
            transition_func = (*TransitionMatrix_)[state][next_state];
            if (transition_func != NULL)
                transition_func(data);
            state = next_state;
        }
        return state;
    }
};

#endif  // COMMON_FSM_H_
