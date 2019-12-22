#include <HardestGame.h>
#include <Setup.h>
#include <Movement.h>
#include <Draw.h>

#include <Planner.h>
#include <ParallelPlanner.h>

#include <cstdio>
#include <ctime>
#include <cstring>
#include <cassert>

// LevelEditor.cpp
extern void LevelEditor();

const int samples = 500;
const int threads = 5;

static GLFWwindow* window;
static Color* fb;

static int draw = 0;
static int freeze = 0;
static int cont = 0;
void KeyCallback(GLFWwindow*, int key, int, int action, int)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        draw ^= 1;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        freeze ^= 1;
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        cont = 1;
    }
}

static int xclick = -1;
static int yclick = -1;
static bool clicked = false;
void MouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        xclick = xpos;
        yclick = height-ypos;
        clicked = true;
    }
}

void ShowPlan(const std::vector<Direction>& plan)
{
    int rep = 0;
    size_t idx = 0;

    IBox player = LevelDscr::Get().player;
    std::vector<EnemyPath> enemies = LevelDscr::Get().enemies;

    while (idx<plan.size() && draw && !glfwWindowShouldClose(window))
    {
        DrawLevel(fb);
        DrawPlayer(fb, player);
        for (const EnemyPath& e : enemies)
        {
            DrawEnemy(fb, e.pos);
        }
        SwapBuffers(window, (float*)fb);
        glfwPollEvents();

        AdvancePlayer(player, plan[idx]);
        for (EnemyPath& e : enemies)
        {
            AdvanceEnemy(e);
            if (PlayerDies(player, e.pos)) break;
        }

        ++rep;
        if (rep == Planner::nRepeatMove)
        {
            rep = 0;
            ++idx;
        }
    }
}

void InteractiveLoop()
{
    window = InitializeGLFW(width, height);
    fb = (Color*)glfwGetWindowUserPointer(window);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    InitializeGL();

    int rnd = time(NULL);
    printf("seed:%d\n", rnd);
    Planner planner(samples, rnd);
    while(!glfwWindowShouldClose(window))
    {
        DrawLevel(fb);
        planner.ForEachPlayer([](const PlayerInfo& player)
        {
            DrawPlayer(fb, player.GetLastPos(), player.IsAlive() ? 1.f : 0.3f);
        });
        for (const EnemyPath& e : LevelDscr::Get().enemies)
        {
            DrawEnemy(fb, e.pos);
        }

        SwapBuffers(window, (float*)fb);

        glfwWaitEvents();
        if (!freeze && planner.SearchForSolution())
        {
            break;
        }
        if (draw)
        {
            // Change PlayerInfo members to public
            /* int bestFitness = INT_MIN; */
            /* std::vector<Direction> bestPlan; */
            /* planner.ForEachPlayer([&bestFitness, &bestPlan](const PlayerInfo& player) */
            /* { */
            /*     if (player.GetFitness() > bestFitness) */
            /*     { */
            /*         bestFitness = player.GetFitness(); */
            /*         bestPlan = player.plan; */
            /*     } */
            /* }); */
            /* ShowPlan(bestPlan); */
            /* draw = false; */
        }
        if (clicked)
        {
            // Change PlayerInfo members to public
            /* std::vector<Direction> plan; */
            /* planner.ForEachPlayer([&plan](const PlayerInfo& player) */
            /* { */
            /*     if (plan.empty() && */
            /*         player.pos.xmin <= xclick && player.pos.xmax >= xclick && */
            /*         player.pos.ymin <= yclick && player.pos.ymax >= yclick) */
            /*     { */
            /*         plan = player.plan; */
            /*     } */
            /* }); */
            /* draw = true; */
            /* ShowPlan(plan); */
            /* draw = false; */
            /* clicked = false; */
        }
        if (cont)
        {
            clock_t then = clock();
            while (!planner.SearchForSolution());
            clock_t elapsed = clock()-then;

            double secs = (double)elapsed / CLOCKS_PER_SEC;
            printf("%lf secs elapsed (single core)\n", secs);

            break;
        }
    }

    if (planner.FoundSolution())
    {
        int siz = planner.GetSolution().size() * Planner::pixelsPerMove;
        printf("solution length (in pixels): %d\n", siz);

        draw = true;
        while(!glfwWindowShouldClose(window) && draw)
        {
            ShowPlan(planner.GetSolution());
        }
    }
    else
    {
        assert(planner.ExhaustedSearch());
        fprintf(stderr, "No solution found\n");
    }
}

void FindSolutionParallel()
{
    int rnd[threads+1];

    FILE* fp = fopen("/dev/random", "r");
    assert(fp);
    int s = fread(&rnd, sizeof(int), threads+1, fp);
    assert(s == threads+1);
    fclose(fp);

    ParallelPlanner planner(samples, threads, rnd);
    time_t now = time(NULL);
    while (!planner.SearchForSolution());
    time_t elapsed = time(NULL) - now;
    printf("%ld secs elapsed\n", elapsed);

    if (planner.FoundSolution())
    {
        int siz = planner.GetSolution().size() * Planner::pixelsPerMove;
        printf("solution length (in pixels): %d\n", siz);

        printf("show solution (y for yes): ");
        char buf[8];
        scanf("%8s", buf);
        if (strcmp(buf, "y") == 0)
        {
            window = InitializeGLFW(width, height);
            fb = (Color*)glfwGetWindowUserPointer(window);
            InitializeGL();

            draw = true;
            while(!glfwWindowShouldClose(window) && draw)
            {
                ShowPlan(planner.GetSolution());
            }
        }
    }
    else
    {
        assert(planner.ExhaustedSearch());
        fprintf(stderr, "No solution found\n");
    }
}

int main(int argc, char** argv)
{
    if (argc == 2 && strcmp(argv[1], "i") == 0)
    {
        InteractiveLoop();
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "p") == 0)
    {
        FindSolutionParallel();
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "e") == 0)
    {
        LevelEditor();
        return 0;
    }

    printf("usage: ./dr OPTION\n");
    printf("i: interactive mode\n");
    printf("\tmove mouse to increase generations\n");
    printf("\tpress space to freeze (toggle). you can click on a player to visualize its path.\n");
    printf("\tpress g to visualize the best player (toggle)\n");
    printf("\tpress c to continue to solution (or lack thereof)\n");
    printf("p: parallel mode\n");
    printf("e: editor mode\n");
    return 1;
}
