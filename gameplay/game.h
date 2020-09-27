#pragma once

#include <glm/glm.hpp>

namespace gameplay {

    const unsigned MAX_ENEMY_COUNT = 32;

    struct state {
        bool interruptible, cancelable, targetable;
        float time_out;
    };

    extern const state dead;

    struct agent {
        glm::vec3 position;
        float yaw;

        const state* active_state = &dead;
        float state_time;
    };

    struct game {
        game();

        agent player;
        agent enemies[MAX_ENEMY_COUNT];

        // TODO: write update function
    };

}
