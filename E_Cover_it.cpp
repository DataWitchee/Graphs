#include <iostream>
#include <vector>
#include <algorithm>
#define ll long long
#define ld long double

const int N = 2e5 + 6;
std::vector<std::vector<int>> adj(N);
std::vector<bool> vis(N);
std::vector<int> col_0(0);
std::vector<int> col_1(0);
// colour 0 or colour 1
// we can use bi partite ness
// dfs always forms spanning tree

void dfs(int u, int col)
{
    vis[u] = true;
    if (col == 0)
    {
        col_0.push_back(u);
    }
    else
    {
        col_1.push_back(u);
    }

    for (int v : adj[u])
    {
        if (!vis[v])
        {
            

            dfs(v, 1 - col);
        }
    }
}

void solve()
{
    int n, m;
    std::cin >> n >> m;
    col_0.clear();
    col_1.clear();
    for (int i = 0; i <= n; i++)
    {
        adj[i].clear();
        vis[i] = false;
    }

    for (int i = 1; i <= m; i++)
    {
        int u, v;
        std::cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // select vertices such that the selected ones are adjacent to unchosen ones
    int k = n / 2;
    // i have to choose such that with each iteration of traversal we traverse entire graph
    
    for (int i = 1; i <= n; i++){
        if(!vis[i]){
            dfs(i, 0);
        }
    }
    /// i got the colour of each vertex

    if (col_0.size() <= n / 2)
    {
        std::cout << col_0.size() << "\n";
        for (auto v : col_0)
        {
            std::cout << v << " ";
        }
    }
    else
    {
        std::cout << col_1.size() << "\n";
        for (auto v : col_1)
        {
            std::cout << v << " ";
        }
    }
    std::cout<<"\n";
}

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    int t;
    std::cin >> t;
    while (t--)
    {
        solve();
    }
    return 0;
}