#include<iostream>
#include<vector>
#include<list>
#include<queue>

using namespace std;

using ll = long long;

vector<ll> edges[500005];//5 * 10^5
bool vis[500005];


void num_user(int tc=0){
    ll n ; ll m;
    cin >> n >> m;

    vector<ll> ans(n+1);

    for(ll i=0; i<m; i++){
        ll k;
        cin >> k;
        vector<ll> v(k);

        for(ll j=0; j<k; j++){
            cin >> v[j];
            
        }

        for(ll j=0; j+1<k; j++){
            edges[v[j]].push_back(v[j+1]);
            edges[v[j+1]].push_back(v[j]);
        }
    }

    for(ll i=1; i<=n; i++){
        if(!vis[i]){
            vector<ll> component;
            queue<ll> q;
            q.push(i);

            while(!q.empty()){
                ll x = q.front();
                q.pop(); // pop the beginning 
                

                if(vis[x]) continue;

                vis[x] = 1;
                component.push_back(x);

                for(ll y : edges[x]){
                    if(!vis[y]){
                        q.push(y);
                    }
                }
             }

             for(ll x:component){
                ans[x] = component.size();
             }
            }
         }

    for(ll i=1; i<=n; i++){
        cout<<ans[i]<<" ";
    }


}


int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    num_user();

    return 0;
}