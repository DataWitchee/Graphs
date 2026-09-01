#include<iostream>
#include<unordered_map>
#include<vector>
#include<array>
#include<math.h>

using namespace std;

using ll = long long ;

int find(ll x,vector<ll> &labels){
    if(labels[x] == x) return x;
    else{
        return labels[x]= find(labels[x], labels);
    }
}

void unite(ll u, ll v, vector<ll> &labels, vector<ll> &esize){
    ll parU = find(u,labels);
    ll parV = find(v, labels);
    if(parU == parV){
        return;
    }
    if(esize[parU]>=esize[parV]){
        labels[parV] = parU;
        esize[parU] += esize[parV]; //adding both components together

    }
    else{
        labels[parU] = parV;
        esize[parV] += esize[parU] ;
    }

}

void NetworkPossible(int tc=0){
    ll n,m;
    cin>> n>> m;
    vector<pair<ll, ll>> Edges;
    vector<ll> labels(n+1);
    vector<ll> esize(n+1);
    
    for(ll i=1; i<=n; i++){
        labels[i] = i;
        esize[i] = 1;
   }
    ll u,v;
    for(ll i=1; i<=m; i++){
        cin>> u>> v;
        unite(u,v,labels,esize); //we united edges together now we know which componenets are tgt and which are disjoint
        Edges.push_back({u,v});
    }

   //Now we have to check if graph is complete or not 
   unordered_map<ll,ll> edgeCount;
   for(auto e:Edges){

    ll u = e.first;
    ll v = e.second;
    ll leader = find(u,labels);
    edgeCount[leader]++;
}
   for(ll i=1; i<=n; i++){
    if(labels[i]==i){
        // i is the leader 
        ll k = esize[i];
        ll required = (k *(k-1) )/2 ;
        ll actual = edgeCount[i];
        if(required != actual){
            cout<<"NO\n";
            return;
        }
    }
   }
   cout<<"YES\n";

}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    NetworkPossible();
    return 0;
}