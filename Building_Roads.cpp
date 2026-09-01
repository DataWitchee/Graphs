#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#define ll long long
#define ld long double

const int N = 1e5 + 8;
std::vector<bool> vis(N);
std::vector<std::vector<int>> path(N + 5);
std::vector<int> bridges;

void dfs(std::vector<std::vector<int>> &path, int node, std::vector<bool> &vis)
{
    vis[node] = true;

    // dfs will return number of connected components
    for (int v : path[node])
    {
        if(!vis[v]){
        dfs(path, v, vis);}
    }
}

void solve()
{
    int n, m;
    std::cin >> n >> m;

    for (int i = 0; i < m; i++)
    {
        int u, v;
        std::cin >> u >> v;
        path[u].push_back(v);
        path[v].push_back(u);
    }
    int c = 0; // connected components 
    
    for (int i = 1; i <= n; i++)
    {
        if (!vis[i])
        {
            c++;
            bridges.push_back(i);

            dfs(path, i, vis);
            
        }
    }
    std::cout<< c-1 << std::endl;
    for(int i=0 ; i<bridges.size() - 1 ; i++ ){
        std::cout << bridges[i] <<" "<< bridges[i+1] << std::endl;
    }
    

}

int main()
{
    solve();
    return 0;
}