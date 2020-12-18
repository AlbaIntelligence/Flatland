#include "flatland_map.h"

#include<iostream>


bool FlatlandMap::generate_instance(int num_of_agents)
{
	num_of_agents = min(num_of_agents, (int)agent_ids.size());
	if (num_of_agents == 0)
		return false;
	start_ids.resize(num_of_agents);
	goal_locations.resize(num_of_agents);
	r_velocities.resize(num_of_agents);
	heuristics.resize(num_of_agents);
	list<int>::const_iterator agent = agent_ids.begin();
	for (int i = 0; i < num_of_agents; i++)
	{
		start_ids[i] = all_start_ids[*agent];
		goal_locations[i] = all_goal_locations[*agent];
		r_velocities[i] = all_r_velocities[*agent];
		heuristics[i] = compute_heuristics(*agent);
		++agent;
	}
	return true;
}


void FlatlandMap::generate_agent_order()
{
	agent_ids.clear();
	int v = 1;
	while (agent_ids.size() < all_start_ids.size())
	{
		for (int i = 0; i < (int)all_start_ids.size(); i++)
		{
			if (all_r_velocities[i] == v)
				agent_ids.push_back(i);
		}
		v++;
	}

}


void FlatlandMap::update_paths(const vector<Path*>& new_paths)
{
	for (const auto& path : new_paths)
	{
		paths[agent_ids.front()] = *path;
		agent_ids.pop_front();
	}
}


bool FlatlandMap::load_map(string fname)
{
  string line;
  ifstream myfile (fname.c_str());
  if (myfile.is_open()) 
  {
    getline (myfile,line);
    boost::char_separator<char> sep(",");
    boost::tokenizer< boost::char_separator<char> > tok(line, sep);
    boost::tokenizer< boost::char_separator<char> >::iterator beg=tok.begin();
    num_vertexes = atoi ( (*beg).c_str() ); // read number of rows
    node2loc.resize(num_vertexes);
    edges.resize(num_vertexes);
    edges_r.resize(num_vertexes);


    // read nodes
    for (int i=0; i<num_vertexes; i++) {
      getline (myfile, line);
      boost::tokenizer< boost::char_separator<char> > l_tok(line, sep);
      boost::tokenizer< boost::char_separator<char> >::iterator l_beg= l_tok.begin();

			int location = atoi((*l_beg).c_str());
      node2loc[i] = location;
    }
    // read edge
    getline (myfile, line);
    boost::tokenizer< boost::char_separator<char> > edge_num_tok(line, sep);
    boost::tokenizer< boost::char_separator<char> >::iterator edge_num_beg=edge_num_tok.begin();
    int num_edges= atoi ( (*edge_num_beg).c_str() ); // read number of rows
    // read nodes
    for (int i=0; i<num_edges; i++) {
      getline (myfile, line);
      boost::tokenizer< boost::char_separator<char> > l_tok(line, sep);
      boost::tokenizer< boost::char_separator<char> >::iterator l_beg=l_tok.begin();

			int from = atoi((*l_beg).c_str());
			l_beg++;
			int to = atoi((*l_beg).c_str());
      edges[from].push_back(to);
      edges_r[to].push_back(from);
    }
    myfile.close();
    return true;
  }
  return false;
}

bool FlatlandMap::load_agents(string fname)
{
	string line;

	ifstream myfile(fname.c_str());

	if (myfile.is_open())
	{
		getline(myfile, line);
		boost::char_separator<char> sep(",");
		boost::tokenizer< boost::char_separator<char> > tok(line, sep);
		boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin();
		int num_of_agents = atoi((*beg).c_str());
		all_start_ids.resize(num_of_agents);
		all_goal_ids.resize(num_of_agents);
		all_goal_locations.resize(num_of_agents);
		all_r_velocities.resize(num_of_agents);
		paths.resize(num_of_agents);
		for (int i = 0; i < num_of_agents; i++)
		{
			getline(myfile, line);
			boost::tokenizer< boost::char_separator<char> > col_tok(line, sep);
			boost::tokenizer< boost::char_separator<char> >::iterator c_beg = col_tok.begin();
			// read start [row,col] for agent i
			all_start_ids[i] = atoi((*c_beg).c_str());
			c_beg++;
			all_goal_locations[i] = atoi((*c_beg).c_str());
			c_beg++;
			int num_goal_ids = atoi((*c_beg).c_str()); // number of goal ids
			c_beg++;
			for (int j = 0; j < num_goal_ids; j++)
			{
				all_goal_ids[i].push_back(atoi((*c_beg).c_str()));
				c_beg++;
			}
			all_r_velocities[i] = atoi((*c_beg).c_str());
		}
		myfile.close();
		return true;
	}
	return false;
}

list<int> FlatlandMap::adjacent_vertices(int vertex_id) const
{
  list<int> vertices;
  for (auto i: edges[vertex_id]){
    vertices.push_back(i);
  }
	// for (int direction = 0; direction < 5; direction++)
	// {
	// 	int next_id = vertex_id + moves_offset[direction];
	// 	if (0 <= next_id && next_id < cols * rows && !my_map[next_id] && abs(next_id % cols - vertex_id % cols) < 2)
	// 		vertices.push_back(next_id);
	// }
	return vertices;
}

list<int> FlatlandMap::children_vertices(int vertex_id) const
{
  auto list = adjacent_vertices(vertex_id);
  if (allowed_wait)
    list.push_back(vertex_id);
  return list;
}


list<int> FlatlandMap::adjacent_vertices_r(int vertex_id) const
{
	list<int> vertices;
  for (auto i: edges_r[vertex_id]){
    vertices.push_back(i);
      }
	// for (int direction = 0; direction < 5; direction++)
	// {
	// 	int next_id = vertex_id + moves_offset[direction];
	// 	if (0 <= next_id && next_id < cols * rows && !my_map[next_id] && abs(next_id % cols - vertex_id % cols) < 2)
	// 		vertices.push_back(next_id);
	// }
	return vertices;
}


bool FlatlandMap::is_node_conflict(int id_0, int id_1) const
{return node2loc[id_0] == node2loc[id_1];}

bool FlatlandMap::is_edge_conflict(int from_0, int to_0, int from_1, int to_1) const
{return node2loc[to_0] == node2loc[from_1] && node2loc[to_1] == node2loc[from_0];}


void FlatlandMap::preprocessing_heuristics()
{
	size_t num_of_agents = start_ids.size();
	heuristics.resize(num_of_agents);
	for (size_t i = 0; i < num_of_agents; i++)
	{
		compute_heuristics(i);
    // std::cout <<"H for agent " << i << " in its start position: " << heuristics[i][start_locations[i]] << std::endl;

	}
}

// compute low-level heuristics
vector<int> FlatlandMap::compute_heuristics(int agent_id)
{
	struct MyNode
	{
		int id;
		int g_val;
		bool in_openlist;

		// the following is used to comapre nodes in the OPEN list
		struct compare_node
		{
			// returns true if n1 > n2 (note -- this gives us *min*-heap).
			bool operator()(const MyNode* n1, const MyNode* n2) const
			{
				return n1->g_val >= n2->g_val;
			}
		};  // used by OPEN (heap) to compare nodes (top of the heap has min g-val)

			// define a typedefs for handles to the heaps (allow up to quickly update a node in the heap)
		typedef fibonacci_heap< MyNode*, boost::heap::compare<MyNode::compare_node> >
			::handle_type open_handle_t;

		open_handle_t open_handle;

		MyNode() {}
		MyNode(int id, int g_val, bool in_openlist = false) : id(id), g_val(g_val), in_openlist(in_openlist) {}

		// The following is used for checking whether two nodes are equal
		// we say that two nodes, s1 and s2, are equal if
		// both agree on the location and timestep
		struct eqnode
		{
			bool operator()(const MyNode* s1, const MyNode* s2) const
			{
				return (s1 == s2) || (s1 && s2 &&
					s1->id == s2->id);
			}
		};

		// The following is used for generating the hash value of a nodes
		// /* TODO:  */his is needed because otherwise we'll have to define the specilized template inside std namespace
		struct NodeHasher
		{
			std::size_t operator()(const MyNode* n) const
			{
				return std::hash<int>()(n->id);
			}
		};
	};


	// generate a heap that can save nodes (and a open_handle)
	fibonacci_heap< MyNode*, compare<MyNode::compare_node> > heap;
	fibonacci_heap< MyNode*, compare<MyNode::compare_node> >::handle_type open_handle;
	// generate hash_map (key is a node pointer, data is a node handler,
	//                    NodeHasher is the hash function to be used,
	//                    eqnode is used to break ties when hash values are equal)
	unordered_set<MyNode*, MyNode::NodeHasher, MyNode::eqnode> nodes;

	for (auto id : all_goal_ids[agent_id])
	{
		auto root = new MyNode(id, 0);
		root->open_handle = heap.push(root);  // add root to heap
		nodes.insert(root);       // add root to hash_table (nodes)
	}
	
	while (!heap.empty())
	{
		MyNode* curr = heap.top();
		heap.pop();
		auto neighbours = adjacent_vertices_r(curr->id);
		for (auto next_loc : neighbours)
		{
			int next_g_val = (int)curr->g_val + 1;
			MyNode* next = new MyNode(next_loc, next_g_val);
			auto it = nodes.find(next);
			if (it == nodes.end()) {  // add the newly generated node to heap and hash table
				next->open_handle = heap.push(next);
				nodes.insert(next);
			}
			else {  // update existing node's g_val if needed (only in the heap)
				delete(next);  // not needed anymore -- we already generated it before
				MyNode* existing_next = *it;
				if (existing_next->g_val > next_g_val) {
					existing_next->g_val = next_g_val;
					heap.update((*it)->open_handle);
				}
			}
		}
	}
	// iterate over all nodes
	vector<int> rst(map_size(), INT_MAX);
	for (auto it = nodes.begin(); it != nodes.end(); it++)
	{
		MyNode* s = *it;
		rst[s->id] = s->g_val;
		delete (s);
	}

  // for (int i = 0; i < heuristics.size(); i ++){
  //   std::cout << i << ":  " << heuristics[i] << std::endl;
  // }

	nodes.clear();
	heap.clear();
	return rst;
}