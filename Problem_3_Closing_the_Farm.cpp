#include <iostream>
#include <vector>
#include <algorithm>
#define ll long long
#define ld long double

// find one single barn that is currently open 
// run dfs on that barn 
// count the number of barns dfs visited 
// if that number is equal to currently open barns return YES else return NO

void dfs(int node, std::vector<std::vector<int>> &nodes, std::vector<bool> &vis, std::vector<bool> &closed)
{
    if (vis[node]  || closed[node])
        return;
    vis[node] = true;

    for (auto v : nodes[node])
    {
        if (!vis[v])
        {
            dfs(v, nodes, vis, closed);
        }
    }
}

void solve()
{
    int m, n;
    std::cin >> n >> m;

    std::vector<std::vector<int>> adj(n + 8);
    std::vector<bool> closure(n + 8, false);
    std::vector<int> order(n+1, false);
    for (int i = 1; i <= m; i++)
    {
        int u, v;
        std::cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    std::vector<bool> vis(n + 4, false);
    int count = 0;
    for (int i = 1; i <= n; i++)
    {
        if (!vis[i])
        {
            count++;
            dfs(i, adj, vis, closure);
        }
    }
    if (count == 1)
    {
        std::cout << "YES" << "\n";
    }
    else
    {
        std::cout << "NO" << "\n";
    }

    
    for (int i = 1; i <= n - 1; i++)
    {
        int closed_node;
        std::cin>>closed_node;
        closure[closed_node] = true;

        // reset visited array that we make true above during checking connected components
        std::fill(vis.begin(), vis.end(), false);
        int connected_components = 0;
        for(int j=1;j<=n;j++){
            if(!vis[j] && !closure[j]){
                connected_components ++;
                dfs(j, adj, vis, closure);


            }
        }
        if(connected_components == 1) std::cout<<"YES"<<std::endl;
        else{
            std::cout<<"NO"<<std::endl;
        }

    }
    int last_node;
    std::cin>>last_node;
}

int main()
{   freopen("closing.in", "r", stdin);
    freopen("closing.out", "w", stdout);
    solve();
    return 0;
}