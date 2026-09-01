#include<iostream>
#include<algorithm>
#include<vector>
#include<list>
#define ll long long 

const int N = 1e5 + 5;
std::vector<std::vector<int>> path(N);
std::vector<bool> vis(N);



// in the question we can clearly see that n pupils and m friendship 
// we can use bi partite ness taking colour 1 or 2 
// we will run dfs with colour as an arguement in it with value 1 or 2 
// with each node i have to print out the colour also 
bool dfs(int node, std::vector<std::vector<int>> &path, int colour, std::vector<bool> &vis, std::vector<int> &ans){
    vis[node] = true;
    ans[node] = colour + 1;
    for(auto v:path[node]){
        if(!vis[v]){
            if(!dfs(v, path, 1 - colour, vis, ans)) return false;;
        }
        else if(ans[v] == ans[node]) return false;
    }
    return true;

}

void solve(){
    int n, m;
    std::cin>> n>> m;
    for(int i=0;i<m;i++){
        int u,v;
        std::cin>>u>> v;
        path[u].push_back(v);
        path[v].push_back(u);
    }
    int color = 0;
    std::vector<int> ans(n+1);
    bool possible = true;
    for(int i=1;i<=n;i++){
        if(!vis[i]) if(!dfs(i, path, color, vis, ans)){ 
            possible = false;
            break;
        }
    }
    if(!possible) {
        std::cout<<"IMPOSSIBLE"<<std::endl;
        return ;}

    for(int i=1;i<=n;i++){
        std::cout<<ans[i]<<" ";
    }

}

int main(){
    solve();
    return 0;
}