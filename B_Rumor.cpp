#include<iostream>
#include<vector>
#include<list>

using namespace std;

using ll = long long;

vector<ll> edges[100005];

bool vis[100005];



ll dfs(ll v, vector<ll> &c){
    ll mn = c[v];
    vis[v] = 1;
    for(ll x : edges[v]){
        if(!vis[x]){
            mn = min(mn,dfs(x,c));
        }
    }
    return mn;
}

void Bribe(int tc=0){
    ll n;
    ll m;
    cin>> n >> m;
    vector<ll> c(n);
    for(ll i=0 ; i<n ; i++){
        cin >> c[i];
}
    
    

    for(ll i=0 ; i<m ; i++){
        ll u;
        ll v;
        cin >> u >> v;
        --u ; --v;
        edges[u].push_back(v);
        edges[v].push_back(u);
        
    }
    ll ans = 0;

    for(ll i=0;i<n;i++){
        if(!vis[i]){
            ans += dfs(i,c);
        }
    }

    cout<< ans << "\n";

}

int main(){
    Bribe();
    return 0;
}

