#pragma once

#include <glm/glm.hpp>

namespace gameplay {

    const unsigned MAX_ENEMY_COUNT = 32;
    const unsigned BUTTON_COUNT = 8;

    struct input {
        enum : unsigned {
            normal_attack, evade,
        };

        glm::vec2 motion;
        bool buttons[BUTTON_COUNT];
    };

    enum : unsigned {
        none = 0,
        interruptible = 1,
        cancelable = 2,
        steerable = 4,
        targetable = 8,
    };

    struct state {
        // TODO: allow multiple edges
        // TODO: make state hierarchical
        // child states may overwrite parent edges
        // edges may connect childs of different parents
        unsigned flags;
        glm::vec3 relative_linear_motion;
        float time_out;
        const state* next;
    };

    struct event {
        bool area_of_effect, lock_on;
        bool affects_team, affects_opponent;
        const state* state;
    };

    extern const state dead, hit, charging, evading, attacking;

    struct agent {
        glm::vec3 position;
        float yaw;

        const state* active_state = &dead;
        float state_time;
    };

    struct game {
        game();
        void update(const input& input, float delta);

        agent player;
        agent enemies[MAX_ENEMY_COUNT];

        input last_input;
    };

}
