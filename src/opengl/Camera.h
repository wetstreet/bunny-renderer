#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "Shader.h"

class Camera
{
    public:
        glm::vec3 Position;
        glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 cameraMatrix = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        glm::vec2 windowPos;
        glm::vec2 viewport;

        glm::vec3 clearColor = glm::vec3(0.2f, 0.2f, 0.2f);

        float FOVdeg = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;

        bool firstClick = true;

        int width;
        int height;

        float speed = 6.0f;
        float sensitivity = 100.0f;
        float sceneScrollSpeed = 0.3f;
        float scenePanSpeed = 0.01f;

        Camera(int width, int height, glm::vec3 position);

        void updateMatrix(float width, float height);
        void Matrix(Shader& shader, const char* uniform);

        // input handling
        void SceneInputs(GLFWwindow* window, float deltaTime);
        bool MouseInScene(GLFWwindow* window, double mouseX, double mouseY);
        void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif //__CAMERA_H__