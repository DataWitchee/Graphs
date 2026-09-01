#include<iostream>
#include<vector>
#include<queue>
#include<stack>
#include<array>

using namespace std;

using ll = long long;


vector<ll> edges[100000]; //10^5
bool vis[100000];
bool col[100000]; // to keep track of colours 
bool possible = 1;

void dfs(ll v, bool c){
    if(vis[v]) return;
    vis[v] = 1;
    col[v] = c;

    for(ll x: edges[v]){
        if(!vis[x]){
            dfs(x , c ^ 1); // c ^ 1 == the other colour // it will just flip the colour 
        }
        else{
            if(col[v] == col[x]){
                possible = 0;
            }
        }
    }
}

void Split_Possible(int tc=0){
    ll n, m;
    cin >> n >>m;

    for(ll i=0; i<m; i++){
        ll u, v;
        cin >> u >>v;
        --u; --v;

        edges[u].push_back(v);
        edges[v].push_back(u);

    }

    for(ll i=0 ; i<n ; i++){
        if(!vis[i]){
            dfs(i, 0); // dfs will determine whether graph is bipartite or not 
        }
    }

    // case 1 it is not bipartite 
    if(!possible){
        cout<< -1 << "\n";
        return;
    }

    // case 2 it is partite now we have to print 0 for set A and 1 colour for set B
    vector<ll> A,B;

    
    for(ll i=0; i<n ; i++){
        if(edges[i].empty()) continue; //dont include isolated vertex 
        if(col[i]==1){
            A.push_back(i+1);
        }
        else{
            B.push_back(i+1);
        }
    }
    cout<<A.size()<<"\n";
    for(ll i : A) cout << i << " ";
    cout << "\n";

    cout<<B.size()<<"\n";
    for(ll i: B) cout<< i << " ";
    cout<< "\n";
}




int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Split_Possible();
    return 0;

}