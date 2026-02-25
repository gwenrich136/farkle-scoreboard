#ifndef MOCK_CONTROL_PAD_H
#define MOCK_CONTROL_PAD_H

#include "Input.h"
#include <queue>

class ControlPad {
public:
    ControlPad() = default;

    // The real methods (can be no-ops for the mock)
    // Removed addButton as per real class change
    GameInput read();

    // Mock-specific method to simulate input
    void press(ButtonAction action);
    void rotate(int delta);
    void simulate(GameInput input);

private:
    std::queue<GameInput> input_queue;
};

#endif // MOCK_CONTROL_PAD_H
