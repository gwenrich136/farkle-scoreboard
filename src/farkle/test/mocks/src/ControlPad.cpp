#include "ControlPad.h"

void ControlPad::press(ButtonAction action) {
    button_press_queue.push(action);
}

ButtonAction ControlPad::read() {
    if (button_press_queue.empty()) {
        return ButtonAction::NONE;
    }
    ButtonAction action = button_press_queue.front();
    button_press_queue.pop();
    return action;
}
