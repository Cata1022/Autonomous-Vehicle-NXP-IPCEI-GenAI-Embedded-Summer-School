#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

enum TurnDirection {
    TURN_NONE,
    TURN_FORWARD,
    TURN_LEFT,
    TURN_RIGHT
};

extern volatile TurnDirection next_turn_direction;
extern bool emergency_stop;

void initPID();
void updatePID(int current_x, bool line_lost, bool intersection_stop);

#endif // PID_CONTROLLER_H
