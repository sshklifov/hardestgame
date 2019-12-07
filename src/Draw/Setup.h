#ifndef SETUP_INCLUDED
#define SETUP_INCLUDED

#include <glad.h>
#include <GLFW/glfw3.h>

extern GLFWwindow* InitializeGLFW(int wwidth, int wheight);
extern void InitializeGL();
extern void SwapBuffers(GLFWwindow* window, float* fb);

#endif
