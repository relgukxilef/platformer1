#include "gameplay.h"

using namespace gameplay;

const state
    gameplay::dead {none, {}, 0, &dead},
    idle {interruptible | cancelable | steerable | targetable, {}, 0, &idle},
    gameplay::evading {none, {0, 0, 0}, 0.4, &idle},
    gameplay::charging {interruptible | targetable, {}, 0.8, &idle},
    gameplay::hit {targetable, {}, 0.3, &idle},
    stunned {interruptible | cancelable | targetable, {}, 0.5, &idle};

game::game() {
    player.position = {0, 0, 0};
    player.yaw = 3.14;
    player.active_state = &idle;

    enemies[0].position = {0, 2, 0};
    enemies[0].yaw = 0;
    enemies[0].active_state = &idle;
}

void update(game& game, agent& agent, float delta) {
    agent.state_time += delta;
    while (
        agent.state_time > agent.active_state->time_out &&
        agent.active_state != agent.active_state->next
    ) {
        // apply transitation effects
        if (agent.active_state == &charging) {
            if (game.player.active_state->flags & interruptible) {
                game.player.active_state = &hit;
                game.player.state_time = 0;
            }
        }

        agent.state_time -= agent.active_state->time_out;
        agent.active_state = agent.active_state->next;
    }
}

void gameplay::game::update(const gameplay::input& input, float delta) {
    glm::vec2 motion = input.motion;
    motion *= 1.f / std::max(1.f, length(motion));

    // buttons
    bool pressed[BUTTON_COUNT];
    bool released[BUTTON_COUNT];
    for (auto i = 0u; i < BUTTON_COUNT; i++) {
        pressed[i] = !last_input.buttons[i] && input.buttons[i];
        released[i] = last_input.buttons[i] && !input.buttons[i];
    }

    if (pressed[input::evade]) {
        if (player.active_state->flags & cancelable) {
            player.active_state = &evading;
            player.state_time = 0;
        }
    }

    last_input = input;

    if (player.active_state->flags & steerable) {
        player.position += glm::vec3(motion, 0) * 4.0f * delta;

        if (dot(motion, motion) > 0.01) {
            player.yaw = atan2f(-motion.x, -motion.y);
        }
    }

    player.position += glm::mat3(
        -cos(player.yaw), sin(player.yaw), 0,
        -sin(player.yaw), -cos(player.yaw), 0,
        0, 0, 0
    ) * player.active_state->relative_linear_motion * delta;

    // AI
    for (auto& enemy : enemies) {
        if (enemy.active_state == &idle && enemy.state_time > 2) {
            enemy.active_state = &charging;
            enemy.state_time = 0;
        }
    }

    // logic
    ::update(*this, player, delta);
    for (auto& enemy : enemies) {
        ::update(*this, enemy, delta);
    }

}
