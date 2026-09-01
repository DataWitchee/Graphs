#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

struct Edge{
  int to; //destination node
  int capacity;  //remaining capacity of that edge
  int rev; //index of the reverse edge in adj list
  Edge(int to,int capacity,int rev){
    this->to = to;
    this->capacity = capacity;
    this->rev = rev;
  }
};

class Graph{
    public:
    int V;
    vector<vector<Edge>> adj;
    Graph(int V){
        this->V = V;
        adj.resize(V);
        
    }
    void addEdge(int u,int v,int capacity){
      adj[u].push_back({v,capacity,adj[v].size()});
      adj[v].push_back({u,capacity,adj[u].size()});
    }


};