#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
    : width(width), height(height), Position(position)
{
}

void Camera::updateMatrix(int width, int height)
{
    view = glm::lookAt(Position, Position + Orientation, Up);
    projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

    cameraMatrix = projection * view;
}

bool Camera::MouseInScene(GLFWwindow* window, double mouseX, double mouseY)
{
    int xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);
    int startX = windowPos.x - xpos;
    int startY = windowPos.y - ypos;
    int endX = windowPos.x - xpos + viewport.x;
    int endY = windowPos.y - ypos + viewport.y;
    
    if (mouseX <= endX && mouseX >= startX && mouseY <= endY && mouseY >= startY)
        return true;
    else
        return false;
}

void Camera::Scroll(float wheel)
{
    Position += Orientation * sceneScrollSpeed * wheel;
}

double lastMouseX;
double lastMouseY;
double firstClick = false;
bool firstClickInScene = false;
double middleFirstClick = false;
bool middleFirstClickInScene = false;
void Camera::SceneInputs(GLFWwindow* window, float deltaTime)
{
    double mouseX;
    double mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    bool mouseInScene = MouseInScene(window, mouseX, mouseY);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        if (middleFirstClick)
        {
            middleFirstClick = false;
            if (mouseInScene)
            {
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                middleFirstClickInScene = true;
            }
        }
        else if (middleFirstClickInScene)
        {
            float offsetX = float(mouseX - lastMouseX);
            Position -= offsetX * scenePanSpeed * glm::normalize(glm::cross(Orientation, Up));
            
            float offsetY = float(mouseY - lastMouseY);
            Position += offsetY * scenePanSpeed * Up;

            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
    {
        middleFirstClick = true;
        middleFirstClickInScene = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (firstClick)
        {
            firstClick = false;
            if (mouseInScene)
            {
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                firstClickInScene = true;
            }
        }
        else if (firstClickInScene)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                Position += deltaTime * speed * Orientation;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                Position += deltaTime * speed * -glm::normalize(glm::cross(Orientation, Up));
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                Position += deltaTime * speed * -Orientation;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                Position += deltaTime * speed * glm::normalize(glm::cross(Orientation, Up));
            }
            float rotx = sensitivity * (float)(mouseY - lastMouseY) / height;
            float roty = sensitivity * (float)(mouseX - lastMouseX) / height;

            glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotx), glm::normalize(glm::cross(Orientation, Up)));

            if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -Up) <= glm::radians(5.0f))))
            {
                Orientation = newOrientation;
            }

            Orientation = glm::rotate(Orientation, glm::radians(-roty), Up);
            
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
    {
        firstClick = true;
        firstClickInScene = false;
    }
}