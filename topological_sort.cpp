#include<iostream>
#include<stack>
#include<algorithm>
using namespace std;

void dfs(int node,vector<bool> &vis,stack<int> &st,vector<vector<int>> &adj){
    vis[node] = true;

    for(auto i:adj[node]){
        if(!vis[i]){
            dfs(i,vis,st,adj);
        }
    }
    st.push(node);
}

vector<int> topoSort(int V, vector<vector<int>> &adj){
    vector<bool> vis(V,false);;
    stack<int> st;
    for(int i=0;i<V;i++){
        if(!vis[i]){
            dfs(i,vis,st,adj);
        }
    }
    vector<int> result;
    while(!st.empty()){
        result.push_back(st.top());
        st.pop();
    }

    return result;
}

int main(){
    int V = 4;
    vector<vector<int>> adj(V);

    adj[0].push_back(1);
    adj[0].push_back(2);
    adj[1].push_back(3);
    adj[2].push_back(3);

    vector<int> topo = topoSort(V,adj);

    for(int i=0;i<V;i++){
        cout<<topo[i];
        if (i != topo.size() - 1) cout << ",";
    }
    return 0;
}


