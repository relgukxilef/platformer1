#pragma once

#include <openvr/openvr_mingw.hpp>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include "rendering.h"

namespace rendering {
    struct vr_window {
        vr_window();
        ~vr_window();

        void set_textures(GLuint left, GLuint right);

        void wait_and_poll();
        void submit();

        unsigned width, height;
        glm::mat4 view_projection_matrices[2];

        vr::IVRSystem *vr_system;
        vr::TrackedDevicePose_t
            tracked_device_poses[vr::k_unMaxTrackedDeviceCount];

        vr::EVREye eyes[2] = {vr::Eye_Left, vr::Eye_Right};
        vr::Texture_t vr_frame_textures[2];
        glm::mat4 projection_matrices[2];
    };
}

