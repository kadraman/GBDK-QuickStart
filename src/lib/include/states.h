#ifndef STATES_H
#define STATES_H

typedef enum {
    STATE_TITLE_SCREEN = 0,
    STATE_GAMEPLAY,
    STATE_GAME_OVER,
    STATE_WIN
} GameStateID;

typedef struct {
    void (*init)(void);
    void (*update)(void);
    void (*cleanup)(void);
} GameState;

void switch_state(GameStateID new_state);
void run_current_state(void);

extern const GameState state_title;
extern const GameState state_gameplay;
extern const GameState state_gameover;
extern const GameState state_win;

#endif
