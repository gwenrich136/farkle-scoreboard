#ifndef MOCK_CONTROL_PAD_H
#define MOCK_CONTROL_PAD_H

#include "ButtonActions.h"
#include <queue>

class ControlPad {
public:
    ControlPad() = default;

    // The real methods (can be no-ops for the mock)
    void addButton(int pin, ButtonAction buttonAction) {}
    ButtonAction read();

    // Mock-specific method to simulate a button press
    void press(ButtonAction action);

private:
    std::queue<ButtonAction> button_press_queue;
};

#endif // MOCK_CONTROL_PAD_H
