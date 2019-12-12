#include "Planner.h"
#include <algorithm> // partition
#include <Movement.h> // Clamp

// PlannerUtility.cpp
extern IPoint GetCenter(const IBox& b);

template <typename URNG>
void Prune(std::vector<PlayerInfo>& players, URNG& , bool wantPart = 0)
{
    // TODO
    if (players.size() < 200) return;

    // Partition so as to prioritize dead players for pruning
    if (wantPart)
    {
        std::partition(players.begin(), players.end(),
            [](const PlayerInfo& info)
            {
                /* return info.dieIdx<0; */
                return info.IsDead();
                // TODO
            });
    }

    // for a few samples n^2 is as good as a sliding window solution
    // also, points are not sparse

    int eraseAfter = players.size()-1;
    for (int i = players.size()-1; i >= 1; --i)
    {
        bool shouldRemove = false;
        for (int j = 0; j < i; ++j)
        {
            IPoint lhs = GetCenter(players[i].GetLastPos());
            IPoint rhs = GetCenter(players[j].GetLastPos());

            int manh = abs(lhs.x-rhs.x) + abs(lhs.y-rhs.y);
            /* const int minThreshold = playerSize / 2; */
            /* const int maxThreshold = 3*playerSize; */
            /* float t = Clamp(players[0].plan.size() / 50.f, 0.f, 1.f); */
            /* int threshold = maxThreshold - (maxThreshold-minThreshold)*t*t; */
            if (manh < Planner::nRepeatMove*playerSpeed*0.8)
            /* if (manh < threshold) */
            {
                /* if (players[i].pruneProtect) */
                /* { */
                /*     int rnd = std::uniform_int_distribution<int>(1, 100)(gen); */
                /*     if (rnd <= 100-Planner::percentPruneProtectOverride) continue; */
                /* } */
                shouldRemove = true;
                break;
            }
        }

        if (shouldRemove)
        {
            std::swap(players[i], players[eraseAfter]);
            --eraseAfter;
        }
    }

    if (eraseAfter < (int)players.size()-1)
    {
        players.erase(players.begin()+eraseAfter+1, players.end());
    }
}
