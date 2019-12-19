#include "Planner.h"
#include <algorithm>
#include <Movement.h>

// PlannerUtility.cpp
extern IPoint GetCenter(const IBox& b);

template <typename URNG>
void Prune(std::vector<PlayerInfo>& players, URNG& , bool wantPart = 0)
{
    // TODO place a parameter here
    if (players.size() < 200) return;

    // Partition so as to prioritize dead players for pruning
    if (wantPart)
    {
        std::partition(players.begin(), players.end(),
            [](const PlayerInfo& info)
            {
                return info.IsDead();
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
            if (manh < Planner::nRepeatMove*playerSpeed*0.8)
            {
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
    players.erase(players.begin()+eraseAfter+1, players.end());
}
