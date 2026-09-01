#include<iostream>
#include<queue>
#include<list>
#include<vector>

using namespace std;

class Graph{
    int V;
    vector<vector<int>> adj;

public:
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
        for(int neigh : adj[i]){
            cout<<neigh<<",";
        }
        cout<<endl;
    }}

    void BFS(int start){
        vector<bool> vis(V,false);
        queue<int> q;
        q.push(start);
        vis[start] = true;

        while(!q.empty()){
            int u = q.front();
            q.pop();
            cout<<u<<" ";
            for(int neighb : adj[u]){
                if(!vis[neighb]){
                    vis[neighb] = true;
                    q.push(neighb);
                }
            }
            cout<<endl;
        }
    }

    void DFSUtil(int u, vector<bool>& vis){
        vis[u] = true;
        cout << u << " ";
        for(int neigh : adj[u]){
            if(!vis[neigh]){
                DFSUtil(neigh, vis);
            }
        }
    }

    void DFS(int start){
        vector<bool> vis(V, false);
        DFSUtil(start, vis);
        cout << endl;
    }
};

int main(){
     Graph g(5);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    g.printGraph();

    cout << "BFS: ";
    g.BFS(0);

    // cout << "DFS: ";
    // g.DFS(0);

    return 0;
}
