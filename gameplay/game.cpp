#include "game.h"

using namespace gameplay;

const state
    gameplay::dead {false, false, false, -1},
    idle {true, true, true, -1 },
    evading {false, false, false, 0.5},
    stunned {true, true, true, 0.5};

game::game() {
    player.position = {0, 0, 0};
    player.yaw = 3.14;
    player.active_state = &idle;

    enemies[0].position = {0, 2, 0};
    enemies[0].yaw = 0;
    enemies[0].active_state = &idle;
}
