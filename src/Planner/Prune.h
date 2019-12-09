#include "Planner.h"
#include <algorithm> // partition

static IPoint GetCenter(const IBox& b)
{
    return IPoint{(b.xmin+b.xmax) / 2, (b.ymin+b.ymax) / 2};
}

template <typename URNG>
void Prune(std::vector<PlayerInfo>& players, URNG& gen, bool wantPart = 0)
{
    // Partition so as to prioritize dead players for pruning
    if (wantPart)
    {
        std::partition(players.begin(), players.end(),
            [](const PlayerInfo& info)
            {
                return info.dieIdx<0;
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
            IPoint lhs = GetCenter(players[i].pos);
            IPoint rhs = GetCenter(players[j].pos);

            int manh = abs(lhs.x-rhs.x) + abs(lhs.y-rhs.y);
            if (manh < Planner::pruneManhThreshold)
            {
                if (players[i].pruneProtect)
                {
                    int rnd = std::uniform_int_distribution<int>(1, 100)(gen);
                    if (rnd <= 100-Planner::percentPruneProtectOverride) continue;
                }
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
