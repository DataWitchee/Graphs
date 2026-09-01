#include <iostream>
#include <vector>
#include <algorithm>
#define ll long long
#define ld long double

// i am thinking of this questons in connected components terms
// run dfs so we can visit all nodes , if we cant visitand dfs loop finished this implies company has to spend money
// so our answer = (connected components - 1) * 1 (1 is the amount of money company spends )

void dfs(int node, const std::vector<std::vector<int>> &nodes, std::vector<bool> &vis)
{
    if (vis[node] == true)
        return;
    vis[node] = true;
    for (auto v : nodes[node])
    {
        if (!vis[v])
        {
            dfs(v, nodes, vis);
        }
    }
}

void solve()
{
    int n, m;
    std::cin >> n >> m;
    std::vector<std::vector<int>> languages(m+1);
    

    int known_languages = 0;
    for (int i = 0; i < n; i++)
    {
        int k;
        std::cin >> k;
        known_languages += k;
        for (int j = 0; j < k; j++)
        {
            int u;
            std::cin >> u;
            languages[u].push_back(i);
        }
    }
    if (known_languages == 0)
    {
        std::cout << n << std::endl;
        return;
    }
    std::vector<std::vector<int>> adj(n + 1);
    for (int i = 1; i <= m; i++)
    {
        if (languages[i].size() >= 2)
        {
            int first_person = languages[i][0];
            // connect this person to everyone who speaks this language
            for (int j = 1; j < languages[i].size(); j++)
            {
                int other_person = languages[i][j];
                adj[first_person].push_back(other_person);
                adj[other_person].push_back(first_person);
            }
        }
    }
    int count = 0;
    std::vector<bool> vis(n + 1, false);
    for (int i = 0; i < n; i++)
    {
        if (!vis[i])
        {
            count++;
            dfs(i, adj, vis);
        }
    }
    std::cout << count - 1 << std::endl; // answers = connected components - 1
}

int main()
{
    solve();
    return 0;
}