#include<iostream>
#include<stack>
#include<queue>
#include<algorithm>
#include<vector>

using namespace std;


class Graph{
  int V;
  public:
  vector<vector<int>> adj;
  
  Graph(int V){
    this -> V = V;
    adj.resize(V);
  }

  void addEdge(int u,int v){
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  void printGraph(){
    for(int i=0;i<V;i++){
        cout<<i<<" -> ";
        for(int neighb : adj[i]){
            cout<<neighb<<",";
        }
        cout<<endl;
       
    }
    cout<<endl;
  }
};

void DFS(Graph &g,int source){
    vector<bool> vis(g.adj.size(),false); //to mark visited nodes
    stack<int> s;
    s.push(source);
    vis[source] = true;
    while(!s.empty()){
        int node = s.top(); 
        s.pop();
        cout<<node<<",";
        for(int neigh : g.adj[node]){
            if(vis[neigh] == false){
                s.push(neigh);
                vis[neigh] = true;
            }
        }

    }

}

bool hasPath(Graph &g,int src ,int dst,vector<bool> &vis){
 
  vis[src] = true;
  if(src == dst) return true;
  for(int neigh : g.adj[src]){
    if(!vis[neigh]){
      if(hasPath(g,neigh,dst,vis)){
        return true;
      }
    }
  }
  return false;
}

int main(){
    Graph g(7);
    g.addEdge(0,1);
    g.addEdge(1,6);
    g.addEdge(1,2);
    g.addEdge(2,3);
    g.addEdge(2,4);
    g.addEdge(4,5);
    DFS(g,0);
    cout<<endl;
    vector<bool> vis(g.adj.size(), false);

    if(hasPath(g, 0, 5, vis)){
    cout << "Path exists";
  } else {
    cout << "No path";
}
    return 0;
}

