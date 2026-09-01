#include<iostream>
#include<stack>
#include<queue>
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
  vector<bool> vis(g.adj.size(),false);
  stack<int> s;
  s.push(source);
  vis[source] = true;
  while(!s.empty()){
    int node = s.top(); //getting the top element of stack cause in stl we pop() return nothing 
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

int main(){
  Graph g(4);
  g.addEdge(2,3);
  g.addEdge(3,0);
  g.addEdge(3,1);
  
  g.printGraph();
  DFS(g,0);
  return 0;

}