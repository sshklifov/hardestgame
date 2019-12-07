#include "Setup.h"
#include "glad.h"
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cassert>

static GLuint program;
static GLuint vao;
static GLuint vbo;
static GLuint ebo;
static GLuint tex;

// OpengL / GLFW specific stuff

static void CleanUp()
{
    glDeleteTextures(1, &tex);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwTerminate();
}

static void ErrorCallback(int code, const char* s)
{
    fprintf(stderr, "GLFW cb error[code=%d]: %s\n", code, s);
}

GLFWwindow* InitializeGLFW(int wwidth, int wheight)
{
    if (!glfwInit())
    {
        fprintf(stderr, "failed to init GLFW\n");
        return NULL;
    }
    atexit(CleanUp);

    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::unique_ptr<GLFWwindow,void(*)(GLFWwindow*)> window(nullptr, glfwDestroyWindow);
    window.reset(glfwCreateWindow(wwidth, wheight, "OpenGL Application", NULL, NULL));
    if (!window)
    {
        fprintf(stderr, "cannot create window with w=%d h=%d\n", wwidth, wheight);
        return NULL;
    }

    glfwMakeContextCurrent(window.get());
    glfwSwapInterval(1);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "could not load opengl library\n");
        return NULL;
    }
    glViewport(0, 0, wwidth, wheight);

    int bufsiz = wwidth * wheight * 3;
    float* fb = (float*)malloc(bufsiz * sizeof(float));
    if (!fb)
    {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }
    glfwSetWindowUserPointer(window.get(), fb);

    return window.release();
}

static GLuint CompileShader(GLenum type, const char* src)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

#ifndef NDEBUG
    int s;
    glGetShaderiv(id, GL_COMPILE_STATUS, &s);
    if (!s)
    {
        char buf[512];
        glGetShaderInfoLog(id, 512, NULL, buf);
        fprintf(stderr, "failed to compile shader (type=%d):\n%s\n", type, buf);
        return 0;
    }
#endif

    return id;
}

void InitializeGL()
{
    const char* vertSrc =
    {
        "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "layout (location = 1) in vec2 in_coords;\n"
        "out vec2 vert_coords;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(pos, 1.0);\n"
        "   vert_coords = in_coords;\n"
        "}\n"
    };

    const char* fragSrc =
    {
        "#version 330 core\n"
        "in vec2 vert_coords;\n"
        "out vec4 color;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "   color = texture(tex, vert_coords);\n"
        "}\n"
    };

    GLuint vertSha = CompileShader(GL_VERTEX_SHADER, vertSrc);
    assert(vertSha != 0);

    GLuint fragSha = CompileShader(GL_FRAGMENT_SHADER, fragSrc);
    assert(fragSha != 0);

    program = glCreateProgram();
    glAttachShader(program, vertSha);
    glAttachShader(program, fragSha);
    glLinkProgram(program);
    
#ifndef NDEBUG
    int s;
    glGetProgramiv(program, GL_LINK_STATUS, &s);
    if (!s)
    {
        char buf[512];
        glGetProgramInfoLog(program, 512, NULL, buf);
        fprintf(stderr, "failed to link program:\n%s\n", buf);
        exit(EXIT_FAILURE);
    }
#endif

    glDeleteShader(vertSha);
    glDeleteShader(fragSha);

    float vertices[] =
    {
        1.f, 1.f, 0.f, 1.f, 1.f,
        1.f, -1.f, 0.f, 1.f, 0.f,
        -1.f, -1.f, 0.f, 0.f, 0.f,
        -1.f, 1.f, 0.f, 0.f, 1.f,
    };

    unsigned indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(sizeof(float)*3));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SwapBuffers(GLFWwindow* window, float* fb)
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, fb);
    glUseProgram(program);
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glfwSwapBuffers(window);
}
