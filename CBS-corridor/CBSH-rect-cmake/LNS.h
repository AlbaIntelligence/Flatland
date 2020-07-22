#pragma once
#include "ICBSSearch.h"

#define DEFAULT_GROUP_SIZE 4
class LNS
{
public:
    //stats about CBS
    double runtime_corridor = 0;
    int HL_num_expanded = 0;
    int HL_num_generated = 0;
    int LL_num_expanded = 0;
    int LL_num_generated = 0;
    int num_standard = 0;
    int num_corridor2 = 0;
    int num_corridor = 0;
    int num_start = 0;
    int num_chasing = 0;

    LNS(AgentsLoader& al, FlatlandLoader& ml,
        const double& f_w, const constraint_strategy& c,
        const int& agent_priority_strategy,
        const options& options1,
        const bool& corridor2,
        const bool& trainCorridor1,
        const bool& chasing):
        al(al), ml(ml), f_w(f_w), c(c), agent_priority_strategy(agent_priority_strategy), options1(options1),
        corridor2(corridor2), trainCorridor1(trainCorridor1), chasing(chasing) {}
    bool run(double time_limit);

private:
    std::clock_t start_time = 0;
    double runtime = 0;
    AgentsLoader& al;
    FlatlandLoader& ml;
    const double& f_w;
    const constraint_strategy& c;
    const int& agent_priority_strategy;
    const options& options1;

    //data for neighbors
    vector<int> neighbors;
    list<Path> neighbor_paths;
    int neighbor_sum_of_costs = 0;
    int neighbor_makespan = 0;
    int delta_costs = 0;
    int group_size = DEFAULT_GROUP_SIZE; // this is useful only when we use CBS to replan


    vector<int> intersections;
    map<int, list<int>> start_locations;  // <start location, corresponding agents>

    // intput params
    double time_limit = 0;
    const bool& corridor2;
    const bool& trainCorridor1;
    const bool& chasing;
    int neighbor_generation_strategy = 0; // 0: random walk; 1: start; 2: intersection
    int prirority_ordering_strategy = 0; // 0: random; 1: max regret
    int replan_strategy = 0; // 0: CBS; 1: prioritized planning

    //stats about each iteration
    typedef tuple<int, double, double, double, int,
            int, int, int, int, int, int> IterationStats;
    list<IterationStats> iteration_stats;

    bool getInitialSolution();

    void replanByPP();
    bool replanByCBS();

    void generateNeighborByRandomWalk(boost::unordered_set<int>& tabu_list);
    void generateNeighborByStart();
    void generateNeighborByIntersection();

    void sortNeighborsRandomly();
    void sortNeighborsByRegrets();
    void sortNeighborsByStrategy();

    //tools
    void updateNeighborPaths();
    void updateNeighborPathsCosts();
    void addAgentPath(int agent, const Path& path);
    void deleteNeighborPaths();
    void quickSort(vector<int>& agent_order, int low, int high, bool regret);
    void randomWalk(int agent_id, const PathEntry& start, int start_timestep,
                    set<int>& neighbor, int neighbor_size, int upperbound);

    void updateCBSResults(const MultiMapICBSSearch<FlatlandLoader>& cbs)
    {
        runtime = (double)(std::clock() - start_time) / CLOCKS_PER_SEC;
        runtime_corridor += cbs.runtime_corridor/CLOCKS_PER_SEC;
        HL_num_expanded += cbs.HL_num_expanded;
        HL_num_generated += cbs.HL_num_generated;
        LL_num_expanded += cbs.LL_num_expanded;
        LL_num_generated += cbs.LL_num_generated;
        num_standard += cbs.num_standard;
        num_corridor2 += cbs.num_corridor2;
        num_corridor += cbs.num_corridor;
        num_start+=cbs.num_start;
        num_chasing += cbs.num_chasing;
    }

    bool hasConflicts(const vector<Path>& paths) const;


    inline bool compareByRegrets(int a1, int a2)
    {
        int r1 = (int)(al.paths_all[a1].size() -
                       al.agents_all[a1].distance_to_goal / al.agents_all[a1].speed);
        int r2 = (int)(al.paths_all[a2].size() -
                       al.agents_all[a2].distance_to_goal / al.agents_all[a2].speed);
        if (r1 == r2)
            return rand() % 2;
        else
            return r1  > r2;
    }
};