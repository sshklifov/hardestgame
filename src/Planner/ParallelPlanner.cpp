#include "ParallelPlanner.h"
#include "Utility.h"

#include <pthread.h>
#include <algorithm>
#include <cassert>

pthread_mutex_t threadMtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threadCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t finishedMtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t finishedCond = PTHREAD_COND_INITIALIZER;

int nFinished = 0;
int globalSeq = 0;

int ParallelPlanner::instances = 0;

void* ParallelPlanner::PerThreadWork(void* context)
{
    Planner* worker = (Planner*)context;
    bool change = false;
    int localSeq = 0;

    while (!change)
    {
        pthread_mutex_lock(&threadMtx);
        while (localSeq == globalSeq)
        {
            pthread_cond_wait(&threadCond, &threadMtx);
        }
        if (globalSeq == -1)
        {
            pthread_mutex_unlock(&threadMtx);
            return NULL;
        }
        assert(globalSeq-localSeq == 1);
        pthread_mutex_unlock(&threadMtx);

        int oldSteps = worker->steps;
        while (worker->steps-oldSteps <= threadSyncPerSteps && !change)
        {
            change = worker->SearchForSolution();
        }

        pthread_mutex_lock(&finishedMtx);
        ++nFinished;
        pthread_mutex_unlock(&finishedMtx);
        pthread_cond_signal(&finishedCond);
        ++localSeq;
    }

    return NULL;
}

ParallelPlanner::ParallelPlanner(int samples, int nthreads, int* seeds) :
    gen(seeds[nthreads]), solutionWorker(-1)
{
    assert(instances == 0);
    ++instances;

    workers.reserve(nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
        workers.emplace_back(samples, seeds[i]);
    }
    threads.resize(nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
        int s = pthread_create(&threads[i], NULL, PerThreadWork, &workers[i]);
        assert(s==0);
    }
}

ParallelPlanner::~ParallelPlanner()
{
    // do not change instances variable

    pthread_mutex_lock(&threadMtx);
    globalSeq = -1;
    pthread_mutex_unlock(&threadMtx);
    pthread_cond_broadcast(&threadCond);

    for (int i = 0; i < (int)threads.size(); ++i)
    {
        int s = pthread_join(threads[i], NULL);
        assert(s == 0);
    }
}

bool ParallelPlanner::FoundSolution() const
{
    return solutionWorker >= 0;
}

const std::vector<Direction>& ParallelPlanner::GetSolution() const
{
    assert(FoundSolution() && workers[solutionWorker].FoundSolution());
    return workers[solutionWorker].GetSolution();
}

bool ParallelPlanner::ExhaustedSearch() const
{
#ifndef NDEBUG
    int nExhaust = 0;
    for (const Planner& worker : workers)
    {
        if (worker.ExhaustedSearch()) ++nExhaust;
    }
    assert(nExhaust==0 || nExhaust == (int)workers.size());
#endif

    return workers[0].ExhaustedSearch();
}

bool ParallelPlanner::SearchForSolution()
{
    if (FoundSolution() || ExhaustedSearch()) return true;

    int nthreads = workers.size();
    int samples = workers[0].samples;

    nFinished = 0;

    pthread_mutex_lock(&threadMtx);
    ++globalSeq;
    pthread_mutex_unlock(&threadMtx);
    pthread_cond_broadcast(&threadCond);

    pthread_mutex_lock(&finishedMtx);
    while (nFinished < nthreads)
    {
        pthread_cond_wait(&finishedCond, &finishedMtx);
    }
    pthread_mutex_unlock(&finishedMtx);

    for (int i = 0; i < nthreads; ++i)
    {
        if (workers[i].FoundSolution() && !FoundSolution())
        {
            solutionWorker = i;
            return true;
        }
        if (workers[i].ExhaustedSearch()) return true;
    }

    std::vector<PlayerInfo> playerPool;
    playerPool.reserve(samples * nthreads);
    for (int t = 0; t < nthreads; ++t)
    {
        assert(workers[t].samples == samples);

        for (PlayerInfo& player : workers[t].players)
        {
            playerPool.push_back(std::move(player));
        }
    }

    int toDelete = nthreads*samples/2;
    std::sort(playerPool.begin(), playerPool.end(),
        [](const PlayerInfo& lhs, const PlayerInfo& rhs)
        {
            return lhs.GetFitness() > rhs.GetFitness();
        });
    NatSelect(playerPool, toDelete);

    assert((int)playerPool.size() > samples);
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

    return false;
}
