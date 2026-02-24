#include "states.h"

static GameStateID current_state_id;
static const GameState* current_state_ptr;

static const GameState* states[] = {
    &state_title,
    &state_gameplay,
    &state_gameover
};

void switch_state(GameStateID new_state) {
    if (current_state_ptr && current_state_ptr->cleanup) {
        current_state_ptr->cleanup();
    }
    current_state_id = new_state;
    current_state_ptr = states[new_state];
    if (current_state_ptr && current_state_ptr->init) {
        current_state_ptr->init();
    }
}

void run_current_state(void) {
    if (current_state_ptr && current_state_ptr->update) {
        current_state_ptr->update();
    }
}
