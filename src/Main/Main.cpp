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

const int samples = 200;
const int threads = 10;

static GLFWwindow* window;
static Color* fb;

static int doDraw = 0;
static int freeze = 0;
static int cont = 0;
void KeyCallback(GLFWwindow*, int key, int, int action, int)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        doDraw ^= 1;
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
    bool dead = false;
    std::vector<EnemyPath> enemies = LevelDscr::Get().enemies;

    while (idx<plan.size() && !dead && doDraw && !glfwWindowShouldClose(window))
    {
        DrawLevel(fb);
        DrawPlayer(fb, player, dead ? 0.f : 1.f);
        for (const EnemyPath& e : enemies)
        {
            DrawEnemy(fb, e.pos);
        }
        SwapBuffers(window, (float*)fb);
        glfwPollEvents();

        if (!dead) AdvancePlayer(player, plan[idx]);
        for (EnemyPath& e : enemies)
        {
            AdvanceEnemy(e);
            if (PlayerDies(player, e.pos)) dead=true;
        }

        idx += (rep+1) / Planner::nRepeatMove;
        rep = (rep+1) % Planner::nRepeatMove;
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
    // fixed
    rnd = 1576017494;
    printf("seed:%d\n", rnd);
    Planner planner(samples, rnd);
    while(!glfwWindowShouldClose(window))
    {
        DrawLevel(fb);
        planner.ForEachPlayer([](const PlayerInfo& player)
        {
            DrawPlayer(fb, player.pos, player.IsDead() ? 0.3f : 1.0f);
        });
        for (const EnemyPath& e : LevelDscr::Get().enemies)
        {
            DrawEnemy(fb, e.pos);
        }

        SwapBuffers(window, (float*)fb);
        glfwWaitEvents();
        if (!freeze)
        {
            if (planner.NextGen()) break;
        }

        if (doDraw)
        {
            int bestDst = INT_MAX;
            std::vector<Direction> bestPlan;
            planner.ForEachPlayer([&bestDst, &bestPlan](const PlayerInfo& player)
            {
                if (player.dst < bestDst)
                {
                    bestDst = player.dst;
                    bestPlan = player.plan;
                }
            });
            ShowPlan(bestPlan);
            doDraw = false;
        }
        if (clicked)
        {
            std::vector<Direction> plan;
            planner.ForEachPlayer([&plan](const PlayerInfo& player)
            {
                if (plan.empty() &&
                    player.pos.xmin <= xclick && player.pos.xmax >= xclick &&
                    player.pos.ymin <= yclick && player.pos.ymax >= yclick)
                {
                    plan = player.plan;
                }
            });
            doDraw = true;
            ShowPlan(plan);
            doDraw = false;
            clicked = false;
        }
        if (cont)
        {
            clock_t then = clock();
            while (!planner.NextGen());
            int elapsed = clock()-then;
            double secs = (double)elapsed / CLOCKS_PER_SEC;
            printf("%lf secs elapsed (single core)\n", secs);
            
            break;
        }
    }

    if (planner.FoundSolution())
    {
        int siz = planner.SeeSolution().size() * Planner::pixelsPerMove;
        printf("solution length (in pixels): %d\n", siz);

        doDraw = true;
        while(!glfwWindowShouldClose(window) && doDraw)
        {
            ShowPlan(planner.SeeSolution());
        }
    }
    else
    {
        assert(planner.Bricked());
        fprintf(stderr, "No solution found\n");
    }
}

void FindSolutionParallel()
{
    fprintf(stderr, "regular planner not finished!\n");
    exit(1);

    clock_t then = clock();

    int rnd[threads+1];

    FILE* fp = fopen("/dev/random", "r");
    assert(fp);
    int s = fread(&rnd, sizeof(int), threads+1, fp);
    assert(s == threads+1);
    fclose(fp);

    ParallelPlanner planner(samples, threads, rnd);
    while (!planner.NewGen());

    if (planner.FoundSol())
    {
        int siz = planner.SeeSol().size() * Planner::pixelsPerMove;
        printf("solution length (in pixels): %d\n", siz);

        int elapsed = clock()-then;
        double secs = (double)elapsed / CLOCKS_PER_SEC;
        printf("%lf secs elapsed per thread\n", secs/threads);

        printf("show solution (y for yes): ");
        char buf[128];
        scanf("%s", buf);
        if (strcmp(buf, "y") == 0)
        {
            window = InitializeGLFW(width, height);
            fb = (Color*)glfwGetWindowUserPointer(window);
            InitializeGL();

            doDraw = true;
            while(!glfwWindowShouldClose(window) && doDraw)
            {
                ShowPlan(planner.SeeSol());
            }
        }
    }
    else
    {
        assert(planner.Bricked());
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
