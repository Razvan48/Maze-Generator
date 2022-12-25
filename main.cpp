#include <iostream>
#include <vector>
#include <queue>

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
const double WINDOW_HEIGHT = 768.0;

const int DIM_X_BOARD = 64.0;
const int DIM_Y_BOARD = 64.0;

const double DIM_X_RECT = WINDOW_WIDTH / (1.0 * DIM_Y_BOARD);
const double DIM_Y_RECT = WINDOW_HEIGHT / (1.0 * DIM_X_BOARD);

const double PERCENT = 0.1;

const double DELTA_X_RECT = DIM_X_RECT * PERCENT;
const double DELTA_Y_RECT = DIM_Y_RECT * PERCENT;

int maze[1 + DIM_X_BOARD + 1][1 + DIM_Y_BOARD + 1];

const double WAITING_TIME = 0.0; // in seconds

double currentTimeMaze = 0.0;
double lastTimeMaze = 0.0;

int startX = 1;
int startY = 1;

int endX = DIM_X_BOARD;
int endY = DIM_Y_BOARD;

int crtX = startX;
int crtY = startY;

vector<pair<int, int>> stack;

int indexQueue;
queue<pair<int, int>> q[2];

int dist[1 + DIM_X_BOARD + 1][1 + DIM_Y_BOARD + 1];

bool partOfChain[1 + DIM_X_BOARD][1 + DIM_Y_BOARD];

vector<pair<int, int>> solution;

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

bool drawCurrentPos = true;

void draw()
{
    int x;
    int y;

    vertices.clear();

    for (int i = 1; i <= DIM_X_BOARD; i++)
    {
        for (int j = 1; j <= DIM_Y_BOARD; j++)
        {
            if (dist[i][j] > 0)
            {
                x = j - 1;
                y = DIM_X_BOARD - i;

                drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
            }
        }
    }

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 1.0, 1.0, 0.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }

    //////////////////////////////////////////////////////

    vertices.clear();

    for (int i = 1; i <= DIM_X_BOARD; i++)
    {
        for (int j = 1; j <= DIM_Y_BOARD; j++)
        {
            if (partOfChain[i][j])
            {
                x = j - 1;
                y = DIM_X_BOARD - i;

                drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
            }
        }
    }

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 0.0, 1.0, 1.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }

    //////////////////////////////////////////////////////

    vertices.clear();

    x = startY - 1;
    y = DIM_X_BOARD - startX;

    drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 1.0, 0.0, 0.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }

    ////////////////////////////////////////////////////

    vertices.clear();

    x = endY - 1;
    y = DIM_X_BOARD - endX;

    drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 0.0, 1.0, 0.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }

    ////////////////////////////////////////////////////

    if (drawCurrentPos)
    {
        vertices.clear();

        x = crtY - 1;
        y = DIM_X_BOARD - crtX;

        drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);

        if (vertices.size() > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

            glUniform3f(colourPath, 0.0, 0.0, 1.0);

            glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
        }
    }

    ////////////////////////////////////////////////////

    vertices.clear();

    for (int i = 1; i <= DIM_X_BOARD; i++)
    {
        for (int j = 1; j <= DIM_Y_BOARD; j++)
        {
            if (maze[i][j] > 0 || (i == startX && j == startY))
            {
                x = j - 1;
                y = DIM_X_BOARD - i;

                if (maze[i][j] != 1)
                {
                    if (maze[i - 1][j] != 2)
                    {
                        drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT - 2.0 * DELTA_Y_RECT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
                    }
                    else if (maze[i - 1][j] != -1)
                    {

                    }
                }
                if (maze[i][j] != 2)
                {
                    if (maze[i + 1][j] != 1)
                    {
                        drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT + DELTA_Y_RECT) / 2.0);
                    }
                    else if (maze[i + 1][j] != -1)
                    {

                    }
                }
                if (maze[i][j] != 3)
                {
                    if (maze[i][j + 1] != 4)
                    {
                        drawRectangle((2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH - 2.0 * DELTA_X_RECT) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * (x + 1) * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
                    }
                    else if (maze[i][j + 1] != -1)
                    {

                    }
                }
                if (maze[i][j] != 4)
                {
                    if (maze[i][j - 1] != 3)
                    {
                        drawRectangle((2.0 * x * DIM_X_RECT - WINDOW_WIDTH) / 2.0, (2.0 * y * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0, (2.0 * x * DIM_X_RECT - WINDOW_WIDTH + 2.0 * DELTA_X_RECT) / 2.0, (2.0 * (y + 1) * DIM_Y_RECT - WINDOW_HEIGHT) / 2.0);
                    }
                    else if (maze[i][j - 1] != -1)
                    {

                    }
                }
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

bool generatingMaze;
bool findingSolution;
bool findingChain;

void initMaze()
{
    maze[crtX][crtY] = -1;

    generatingMaze = true;

    stack.emplace_back(crtX, crtY);

    q[indexQueue].emplace(crtX, crtY);

    dist[crtX][crtY] = 1;

    for (int i = 0; i <= DIM_X_BOARD + 1; i++)
    {
        maze[i][0] = -1;
        maze[i][1 + DIM_Y_BOARD] = -1;
    }

    for (int j = 0; j <= DIM_Y_BOARD + 1; j++)
    {
        maze[0][j] = -1;
        maze[1 + DIM_X_BOARD][j] = -1;
    }
}

int dx[] = { -1,    1,  -1,  0,  0 };
int dy[] = { -1,    0,   0, -1,  1 };

void generateMaze()
{
    if (!stack.empty())
    {
        int x = stack.back().first;
        int y = stack.back().second;

        crtX = x;
        crtY = y;

        vector<int> possibleDirections;

        for (int k = 1; k <= 4; k++)
        {
            int newX = x + dx[k];
            int newY = y + dy[k];

            if (maze[newX][newY] == 0)
                possibleDirections.emplace_back(k);
        }

        if (!possibleDirections.empty())
        {
            int direction = possibleDirections[rand() % (int)possibleDirections.size()];

            maze[x + dx[direction]][y + dy[direction]] = direction;

            stack.emplace_back(x + dx[direction], y + dy[direction]);
        }
        else
        {
            stack.pop_back();
        }
    }
    else
    {
        generatingMaze = false;

        drawCurrentPos = false;

        findingSolution = true;
    }
}

void findSolution()
{
    while (!q[indexQueue].empty())
    {
        int x = q[indexQueue].front().first;
        int y = q[indexQueue].front().second;

        q[indexQueue].pop();

        int invMaze;

        if (maze[x][y] == 1)
            invMaze = 2;
        else if (maze[x][y] == 2)
            invMaze = 1;
        else if (maze[x][y] == 3)
            invMaze = 4;
        else if (maze[x][y] == 4)
            invMaze = 3;
        else
            invMaze = -1;

        for (int k = 1; k <= 4; k++)
        {
            int newX = x + dx[k];
            int newY = y + dy[k];

            if (maze[newX][newY] != -1 && dist[newX][newY] == 0)
            {
                int invNew;

                if (maze[newX][newY] == 1)
                    invNew = 2;
                else if (maze[newX][newY] == 2)
                    invNew = 1;
                else if (maze[newX][newY] == 3)
                    invNew = 4;
                else if (maze[newX][newY] == 4)
                    invNew = 3;
                else
                    invNew = -1;

                if ((invMaze != -1 && x + dx[invMaze] == newX && y + dy[invMaze] == newY) || (invNew != -1 && newX + dx[invNew] == x && newY + dy[invNew] == y))
                {
                    dist[newX][newY] = dist[x][y] + 1;
                    q[1 - indexQueue].emplace(newX, newY);
                }
            }
        }
    }

    indexQueue = 1 - indexQueue;

    if (q[indexQueue].empty())
    {
        findingSolution = false;

        findingChain = true;
    }
}

void findChain()
{
    solution.emplace_back(endX, endY);

    crtX = endX;
    crtY = endY;

    while (crtX != startX || crtY != startY)
    {
        int invMaze;

        if (maze[crtX][crtY] == 1)
            invMaze = 2;
        else if (maze[crtX][crtY] == 2)
            invMaze = 1;
        else if (maze[crtX][crtY] == 3)
            invMaze = 4;
        else if (maze[crtX][crtY] == 4)
            invMaze = 3;
        else
            invMaze = -1;

        for (int k = 1; k <= 4; k++)
        {
            int newX = crtX + dx[k];
            int newY = crtY + dy[k];

            if (maze[newX][newY] != -1 || (newX == startX && newY == startY))
            {
                int invNew;

                if (maze[newX][newY] == 1)
                    invNew = 2;
                else if (maze[newX][newY] == 2)
                    invNew = 1;
                else if (maze[newX][newY] == 3)
                    invNew = 4;
                else if (maze[newX][newY] == 4)
                    invNew = 3;
                else
                    invNew = -1;

                if ((invMaze != -1 && crtX + dx[invMaze] == newX && crtY + dy[invMaze] == newY) || (invNew != -1 && newX + dx[invNew] == crtX && newY + dy[invNew] == crtY))
                {
                    if (dist[crtX][crtY] == dist[newX][newY] + 1)
                    {
                        crtX = newX;
                        crtY = newY;

                        solution.emplace_back(crtX, crtY);
                    }
                }
            }
        }
    }

    findingChain = false;
}

int main()
{
    srand(time(NULL));

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Maze Generator", 0, 0);
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

    initMaze();

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        handleInput(window);

        if (generatingMaze)
        {
            currentTimeMaze = glfwGetTime();
            if (currentTimeMaze - lastTimeMaze > WAITING_TIME)
            {
                lastTimeMaze = currentTimeMaze;
                generateMaze();
            }
        }
        else if (findingSolution)
        {
            currentTimeMaze = glfwGetTime();
            if (currentTimeMaze - lastTimeMaze > WAITING_TIME)
            {
                lastTimeMaze = currentTimeMaze;
                findSolution();
            }
        }
        else if (findingChain)
        {
            findChain();
        }
        else
        {
            if (!solution.empty())
            {
                int x = solution.back().first;
                int y = solution.back().second;

                solution.pop_back();

                partOfChain[x][y] = true;
            }
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