#ifndef PARALLEL_PLANER_INCLUDED
#define PARALLEL_PLANER_INCLUDED

#include <HardestGame.h> // TODO remove

// circular include
class ParallePlanner;
#include "Planner.h"

#include <vector>
#include <random>

class ParallelPlanner
{
public:
    ParallelPlanner(int samples, int nthreads, int* seed);

    ParallelPlanner(const ParallelPlanner&) = delete;
    void operator=(const ParallelPlanner&) = delete;

    bool FoundSol() const;
    const std::vector<Direction>& SeeSol() const;
    bool Bricked() const;

    bool NewGen();

private:
    static void* DoWork(void* context);

private:
    std::vector<Planner> workers;
    int workaholicIdx;
    std::default_random_engine gen;
};

#endif
