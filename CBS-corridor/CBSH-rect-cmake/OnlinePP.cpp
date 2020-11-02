#include "OnlinePP.h"


OnlinePP::OnlinePP(AgentsLoader& al, FlatlandLoader& ml, const options& options1, float time_limit):
        al(al), ml(ml), options1(options1)
{
    high_resolution_clock::time_point start_time = Time::now();

    // group the agents by their start location, sorting by distance to goal
    unplanned_agents.resize(ml.map_size());
    for (Agent& agent : al.agents_all)
    {
        int start_location = agent.initial_location;
        auto it = unplanned_agents[start_location].begin();
        for (; it != unplanned_agents[start_location].end(); ++it)
        {
            if (al.agents_all.size() > 400 && agent.distance_to_goal < (*it)->distance_to_goal)
            {
                unplanned_agents[start_location].insert(it, &agent);
                break;
            }
            else if (al.agents_all.size() <= 400 && agent.distance_to_goal > (*it)->distance_to_goal)
            {
                unplanned_agents[start_location].insert(it, &agent);
                break;
            }
        }
        if (it == unplanned_agents[start_location].end())
            unplanned_agents[start_location].push_back(&agent);
    }

    // add all start locations to to be planned
    list<int> start_locations;
    for (size_t i = 0; i < unplanned_agents.size(); i++)
    {
        if (!unplanned_agents[i].empty())
        {
            start_locations.push_back(i);
        }
    }
    size_t planned_agents = 0;
    runtime = ((fsec)(Time::now() - start_time)).count();
    while(runtime < time_limit && planned_agents < al.getNumOfAllAgents())
    {
        to_be_planned_group = start_locations;
        updateToBePlanedAgents(start_locations.size());
        planned_agents += to_be_planned_agents.size();
        runtime = ((fsec)(Time::now() - start_time)).count();
        planPaths(time_limit - runtime);
        runtime = ((fsec)(Time::now() - start_time)).count();
    }
}

void OnlinePP::updateToBePlanedAgents(int max_num_agents)
{
    int count = 0;
    while (!to_be_planned_group.empty() && count < max_num_agents) {
        count++;
        int start = to_be_planned_group.front();
        to_be_planned_group.pop_front();
        if (unplanned_agents[start].empty())
            continue;
        auto agent = unplanned_agents[start].front();
        int count = 0;
        while (agent->malfunction_left > 0 && count < unplanned_agents[start].size()) // the agent is in malfunction
        {
            unplanned_agents[start].pop_front(); // move it to the last
            unplanned_agents[start].push_back(agent);
            agent = unplanned_agents[start].front(); // check the next one
            count++;
        }
        unplanned_agents[start].pop_front();
        to_be_planned_agents.push_back(agent);
    }
}

bool OnlinePP::planPaths(float time_limit)
{
    high_resolution_clock::time_point start_time = Time::now();
    int screen = options1.debug;
    al.num_of_agents = 1;
    al.agents.resize(1);
    int num_agents = (int)to_be_planned_agents.size();
    runtime = ((fsec)(Time::now() - start_time)).count();
    while (!to_be_planned_agents.empty() && runtime < time_limit)
    {
        auto agent = to_be_planned_agents.front();
        to_be_planned_agents.pop_front();
        assert(al.paths_all[agent->agent_id].empty());
        al.agents[0] = agent;
        SinglePlanning planner(ml,al,1,time_limit - runtime,options1);
        planner.search();
        addAgentPath(agent->agent_id, planner.path);
        runtime = ((fsec)(Time::now() - start_time)).count();
        if (runtime >= time_limit && planner.path.empty())
            to_be_planned_agents.push_back(agent);
    }
    cout << "Plan paths for " << num_agents - (int)to_be_planned_agents.size() << " agents" << endl;
    if (options1.debug)
    {
        runtime = ((fsec)(Time::now() - start_time)).count();
        cout << endl << endl << "Find a solution for " << al.getNumOfAllAgents() - al.getNumOfUnplannedAgents()
             << " agents (including " << al.getNumOfDeadAgents() << " dead agents) in " << runtime << " seconds!" << endl;
    }
    return true;
}


void OnlinePP::addAgentPath(int agent, const Path& path)
{
    assert(agent == al.agents_all[agent].agent_id);
    if(!al.constraintTable.insert_path(agent, path))
        exit(10);
    al.paths_all[agent] = path;
}