#include <iostream>
#include <vector>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

using namespace std;

const double WINDOW_WIDTH = 1024.0;
const double WINDOW_HEIGHT = 1024.0;

const int DIM_X_BOARD = 256.0;
const int DIM_Y_BOARD = 256.0;

const double DIM_X_RECT = WINDOW_WIDTH / (1.0 * DIM_X_BOARD);
const double DIM_Y_RECT = WINDOW_HEIGHT / (1.0 * DIM_Y_BOARD);

bool matrix[DIM_Y_BOARD][DIM_X_BOARD];
bool newMatrix[DIM_Y_BOARD][DIM_X_BOARD];

const double WAITING_TIME = 0.0125; // in seconds

double currentTimeConway = 0.0;
double lastTimeConway = 0.0;

const int HASH = 7;

const char* vertexShaderSource =
"#version 330 core \n"
"\n"
"layout (location = 0) in vec2 vertexPosition; \n"
"uniform mat4 ortho; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   gl_Position = ortho * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0); \n"
"\n"
"} \n"
"\0";

const char* fragmentShaderSource =
"#version 330 core \n"
"\n"
"out vec4 vertexColour; \n"
"uniform vec3 colour; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   vertexColour = vec4(colour, 1.0); \n"
"\n"
"} \n"
"\0";

unsigned int VAO;
unsigned int VBO;

double currentTime;
double previousTime;
double deltaTime;

void updateDeltaTime()
{
    currentTime = glfwGetTime();
    deltaTime = currentTime - previousTime;
    previousTime = currentTime;
}

void handleInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

vector<double> vertices;

/*
2----3/4
|  /   |
| /    |
1/5----6
(x1, y1) down-left
(x2, y2) up-right
*/

void drawRectangle(double x1, double y1, double x2, double y2)
{
    vertices.emplace_back(x1);
    vertices.emplace_back(y1);

    vertices.emplace_back(x1);
    vertices.emplace_back(y2);

    vertices.emplace_back(x2);
    vertices.emplace_back(y2);

    vertices.emplace_back(x2);
    vertices.emplace_back(y2);

    vertices.emplace_back(x1);
    vertices.emplace_back(y1);

    vertices.emplace_back(x2);
    vertices.emplace_back(y1);
}

int colourPath;

void draw()
{
    vertices.clear();

    for (int i = 0; i < DIM_X_BOARD; i++)
    {
        for (int j = 0; j < DIM_Y_BOARD; j++)
        {
            if (matrix[i][j])
            {
                int x = j;
                int y = (DIM_X_BOARD - 1) - i;

                drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
            }
        }
    }

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 1.0, 1.0, 1.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }
}

void initMatrix()
{
    for (int i = 0; i < DIM_X_BOARD; i++)
        for (int j = 0; j < DIM_Y_BOARD; j++)
            matrix[i][j] = (rand() % HASH == 0);
}

void Conway()
{
    int dx[] = { 0, 0, -1, 1, 1, -1,  1, -1 };
    int dy[] = { -1, 1,  0, 0, 1, -1, -1,  1 };

    for (int i = 0; i < DIM_X_BOARD; i++)
        for (int j = 0; j < DIM_Y_BOARD; j++)
            newMatrix[i][j] = false;

    for (int i = 1; i < DIM_X_BOARD - 1; i++)
    {
        for (int j = 1; j < DIM_Y_BOARD - 1; j++)
        {
            int num = 0;

            for (int k = 0; k < 8; k++)
                if (matrix[i + dx[k]][j + dy[k]])
                    num++;

            if (matrix[i][j])
            {
                if (num == 2 || num == 3)
                    newMatrix[i][j] = true;
            }
            else
            {
                if (num == 3)
                    newMatrix[i][j] = true;
            }
        }
    }

    for (int i = 0; i < DIM_X_BOARD; i++)
        for (int j = 0; j < DIM_Y_BOARD; j++)
            matrix[i][j] = newMatrix[i][j];
}

int main()
{
    srand(time(NULL));

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "John Conway's Game of Life", 0, 0);
    //glfwGetPrimaryMonitor();

    glfwMakeContextCurrent(window);

    glewInit();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    colourPath = glGetUniformLocation(shaderProgram, "colour");
    int orthoPath = glGetUniformLocation(shaderProgram, "ortho");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 ortho = glm::ortho(-0.5 * WINDOW_WIDTH, 0.5 * WINDOW_WIDTH, -0.5 * WINDOW_HEIGHT, 0.5 * WINDOW_HEIGHT);
    glUniformMatrix4fv(orthoPath, 1, GL_FALSE, glm::value_ptr(ortho));

    initMatrix();

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        handleInput(window);

        currentTimeConway = glfwGetTime();
        if (currentTimeConway - lastTimeConway > WAITING_TIME)
        {
            lastTimeConway = currentTimeConway;
            Conway();
        }

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}