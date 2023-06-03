#ifndef common_hpp
#define common_hpp

#include <iostream>
#include <string>
#include <mach/mach.h>

#include <GL/glew.h>
#include <glm/glm.hpp>

class CommonUtil
{
public:
    static inline void glfwErrorCallback(int error, const char *description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    static inline void printStartInfo()
    {
        // current date/time based on current system
        time_t now = time(0);
        char *dateTime = ctime(&now);
        // version, format -> x.xx.xxx
        std::string version("000001");

        std::cout << "enigine_version: " << version << std::endl;
        std::cout << "cpp_version: " << __cplusplus << std::endl;
        std::cout << "started_at: " << dateTime << std::endl;
    }

    // TODO: support other platforms than macOS
    static inline void refreshSystemMonitor(task_basic_info &t_info)
    {
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        task_info(mach_task_self(),
                  TASK_BASIC_INFO, (task_info_t)&t_info,
                  &t_info_count);
    }

    static inline float snappedClamp(float value, float min, float max, float snapLength)
    {
        float clamped = std::max(min, std::min(value, max));
        if (clamped > (max - snapLength))
            clamped = max;
        if (clamped < (min + snapLength))
            clamped = min;

        return clamped;
    }

    static inline float lerp(float x, float y, float t)
    {
        return x * (1.f - t) + y * t;
    }

    static inline float lerpAngle(float startAngle, float targetAngle, float t)
    {
        float shortestAngle = targetAngle - startAngle;

        // Wrap the angle to the range -π to π
        if (shortestAngle > M_PI)
            shortestAngle -= 2 * M_PI;
        else if (shortestAngle < -M_PI)
            shortestAngle += 2 * M_PI;

        // Perform linear interpolation
        float result = startAngle + t * shortestAngle;

        // Wrap the result to the range -π to π
        if (result > M_PI)
            result -= 2 * M_PI;
        else if (result < -M_PI)
            result += 2 * M_PI;

        return result;
    }

    static glm::vec2 cubicBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, double t)
    {
        double x = pow(1 - t, 3) * p0.x + 3 * t * pow(1 - t, 2) * p1.x + 3 * pow(t, 2) * (1 - t) * p2.x + pow(t, 3) * p3.x;
        double y = pow(1 - t, 3) * p0.y + 3 * t * pow(1 - t, 2) * p1.y + 3 * pow(t, 2) * (1 - t) * p2.y + pow(t, 3) * p3.y;
        return glm::vec2(x, y);
    }

    // TODO: primitive creation
    static inline void createQuad(unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
    {
        float quad_vertices[] = {
            // top left
            -1, 1,
            0.0f, 1.0f,
            // top right
            1, 1,
            1.0f, 1.0f,
            // bottom left
            -1, -1,
            0.0f, 0.0f,
            // bottom right
            1, -1,
            1.0f, 0.0f};
        unsigned int quad_indices[] = {0, 1, 2, 1, 2, 3};
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

#endif /* common_hpp */