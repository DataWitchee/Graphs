#include<iostream>
#include<array>
#include<vector>

typedef long long ll; //using ll = long loong 



using namespace std;

void solve(int tc = 0){
    ll t;
    ll n;
   

    cin>> n >> t;
    vector<int> a(n-1);
    

    for(ll i=0 ; i<n-1 ; i++) cin >> a[i];

    ll curr = 1;

    do{
        curr = curr + a[curr-1];
        if(curr == t ){
            cout<<"YES\n";
            return;

        }

    }while(curr < n);

    cout<<"NO\n";


}

int main(){
    solve();
    return 0;
}