#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view = glm::lookAt(Position, Position + Orientation, Up);
    projection = glm::perspective(glm::radians(FOVdeg), (float)(width / height), nearPlane, farPlane);

    cameraMatrix = projection * view;
}

void Camera::Matrix(Shader& shader, const char* uniform)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Position += speed * -glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Position += speed * -Orientation;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Position += speed * glm::normalize(glm::cross(Orientation, Up));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Position += speed * Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        Position += speed * -Up;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 0.4f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
    {
        speed = 0.1f;
    }

    // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    // {
    //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    //     if (firstClick)
    //     {
    //         glfwSetCursorPos(window, width / 2, height / 2);
    //         firstClick = false;
    //     }

    //     double mouseX;
    //     double mouseY;
    //     glfwGetCursorPos(window, &mouseX, &mouseY);

    //     float rotx = sensitivity * (float)(mouseY - (height / 2)) / height;
    //     float roty = sensitivity * (float)(mouseX - (height / 2)) / height;

    //     glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotx), glm::normalize(glm::cross(Orientation, Up)));

    //     if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -Up) <= glm::radians(5.0f))))
    //     {
    //         Orientation = newOrientation;
    //     }

    //     Orientation = glm::rotate(Orientation, glm::radians(-roty), Up);

    //     glfwSetCursorPos(window, width / 2, height / 2);
    // }
    // else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    // {
    //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //     firstClick = true;
    // }
}