#ifndef PARALLEL_PLANER_INCLUDED
#define PARALLEL_PLANER_INCLUDED

#include "Planner.h"

#include <HardestGame.h>
#include <vector>
#include <random>

// Do not instantiate more than once

class ParallelPlanner
{
public:
    ParallelPlanner(int samples, int nthreads, int* seed);
    ~ParallelPlanner();

    ParallelPlanner(const ParallelPlanner&) = delete;
    void operator=(const ParallelPlanner&) = delete;

    bool FoundSolution() const;
    bool ExhaustedSearch() const;
    const std::vector<Direction>& GetSolution() const;

    bool SearchForSolution();

private:
    static void* PerThreadWork(void* context);

public:
    static const int threadSyncPerSteps = 4*Planner::incSteps;
private:
    static int instances;

private:
    std::default_random_engine gen;
    std::vector<Planner> workers;
    std::vector<pthread_t> threads;
    int solutionWorker;
};

#endif
