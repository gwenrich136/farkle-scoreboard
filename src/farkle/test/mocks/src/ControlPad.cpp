#include "ControlPad.h"

void ControlPad::begin() {
    // No-op for mock
}

void ControlPad::setToggleState(ScoreDisplayMode mode) {
    toggle_state = mode;
}

void ControlPad::press(ButtonAction action) {
    GameInput input;
    input.action = action;
    input.rotationDelta = 0;
    input.scoreDisplayMode = toggle_state;
    input_queue.push(input);
}

void ControlPad::rotate(int delta) {
    GameInput input;
    input.action = ButtonAction::NONE;
    input.rotationDelta = delta;
    input.scoreDisplayMode = toggle_state;
    input_queue.push(input);
}

void ControlPad::simulate(GameInput input) {
    input.scoreDisplayMode = toggle_state;
    input_queue.push(input);
}

GameInput ControlPad::read() {
    if (input_queue.empty()) {
        GameInput input;
        input.action = ButtonAction::NONE;
        input.rotationDelta = 0;
        input.scoreDisplayMode = toggle_state;
        return input;
    }
    GameInput input = input_queue.front();
    input.scoreDisplayMode = toggle_state; // ensure it uses latest toggle state
    input_queue.pop();
    return input;
}
