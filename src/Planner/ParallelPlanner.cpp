#include "ParallelPlanner.h"
#include "Utility.h"

#include <pthread.h>
#include <algorithm>
#include <cassert>

void* ParallelPlanner::DoWork(void* context)
{
    /* Planner* worker = (Planner*)context; */
    /* bool change=false; */
    /* while (worker->generation < Planner::genPerStepInc && !change) */
    /* { */
    /*     change = worker->NextGen(); */
    /* } */
    /* if (change) return (void*)change; */

    /* int oldSteps = worker->steps; */
    /* change = worker->NextGen(); */
    /* assert(worker->steps > oldSteps); */
    /* return (void*)change; */
    return context;
}

ParallelPlanner::ParallelPlanner(int samples, int nthreads, int* seeds) :
    workaholicIdx(-1), gen(seeds[nthreads])
{
    for (int i = 0; i < nthreads; ++i)
    {
        workers.emplace_back(samples, seeds[i]);
    }
}

bool ParallelPlanner::FoundSol() const
{
    return workaholicIdx >= 0;
}

const std::vector<Direction>& ParallelPlanner::SeeSol() const
{
    assert(FoundSol() && workers[workaholicIdx].FoundSolution());
    return workers[workaholicIdx].SeeSolution();
}

bool ParallelPlanner::Bricked() const
{
#ifndef NDEBUG
    int ndead = 0;
    for (const Planner& worker : workers)
    {
        if (worker.Bricked()) ++ndead;
    }
    assert(ndead==0 || ndead == (int)workers.size());
#endif

    return workers[0].Bricked();
}

// return true if either FoundSol or Bricked is true
// i.e. no new generations will be made
bool ParallelPlanner::NewGen()
{
    if (FoundSol() || Bricked()) return true;

    int nthreads = workers.size();
    int samples = workers[0].samples;

    std::vector<pthread_t> threads(nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
        int s = pthread_create(&threads[i], NULL, DoWork, &workers[i]);
        assert(s==0);
    }

    bool fin = false;
    for (int i = 0; i < nthreads; ++i)
    {
        void* retval;
        int s = pthread_join(threads[i], &retval);
        assert(s == 0);

        if ((bool)retval)
        {
            if (workers[i].FoundSolution() && !FoundSol())
            {
                workaholicIdx = i;
            }
            fin = true;
        }
    }
    if (fin) return true;

    // merge all players from threads & prune
    std::vector<PlayerInfo> playerPool;
    playerPool.reserve(samples * nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
        assert(workers[i].samples == samples);

        for (int j = 0; j < (int)workers[i].players.size(); ++j)
        {
            playerPool.push_back(std::move(workers[i].players[j]));
        }
        /* workers[i].ForEachPlayer([&playerPool](const PlayerInfo& player) */
        /* { */
        /*     playerPool.push_back(player); */
        /* }); */
    }

    // TODO jk
    /* Prune(playerPool, gen, 1); */
    // randomly assign players to threads
    if ((int)playerPool.size() <= samples)
    {
        for (int i = 0; i < nthreads; ++i)
        {
            workers[i].players.assign(playerPool.begin(), playerPool.end());
        }
    }
    else
    {
        assert(samples > 0);
        int offset = playerPool.size();
        for (int i = 0; i < nthreads; ++i)
        {
            if ((int)playerPool.size() - offset < samples)
            {
                std::shuffle(playerPool.begin(), playerPool.end(), gen);
                offset = 0;
            }

            auto start = playerPool.begin() + offset;
            workers[i].players.assign(start, start+samples);
            offset += samples;
        }
    }

    return false;
}
