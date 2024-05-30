#include<iostream>
#include <vector>

using namespace std;

vector<vector<long>> lagrange_basis;

// Function to compute the modular inverse of a under modulo p
// Uses Fermat's little theorem: a^(p-1) ≡ 1 (mod p), hence a^(p-2) ≡ a^(-1) (mod p)
long modInverse(long a, long p) {
    long result = 1;
    long exponent = p - 2; // Fermat's little theorem exponent

    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = (result * a) % p;
        }
        a = (a * a) % p;
        exponent /= 2;
    }
    return result;
}

void generate_lagrange_basis(vector<long> x, long p){
    lagrange_basis.clear();
    for (int i = 0; i < x.size(); i++){
        vector<long> component;
        component.push_back(1);
        for (int j = 0; j < x.size(); j++){
            if (j == i) continue;
            component.push_back(component.back());
            for (int k = component.size() - 2; k > 0; k--) component[k] = (component[k-1] + (p - component[k]) * x[j]) % p;
            component[0] = ((p - component[0]) * x[j]) % p;
        }

        long inv = 1;
        for (int j = 0; j < x.size(); j++){
            if (i == j) continue;
            (inv *= modInverse((p + x[i] - x[j]) % p, p)) %= p;
        }

        for (int j = 0; j < component.size(); j++) (component[j] *= inv) %= p;
        lagrange_basis.push_back(component);
    }
}

// Lagrange Interpolation
vector<long> lagrange_interpolation(vector<long> f, long p){
    vector<long> coeff(f.size(), 0);
    for (int i = 0; i < f.size(); i++){
        for (int j = 0; j < lagrange_basis[i].size(); j++) (coeff[j] += lagrange_basis[i][j] * f[i]) %= p;
    }
    return coeff;
}

// int main(){
//     vector<long> f = {4, 0, 5};
//     vector<long> x = {1, 2, 3};
//     long p = 7;
    
//     generate_lagrange_basis(x, p);
//     vector<long> coeff = lagrange_interpolation(f, p);

//     for (int u: coeff) cout << u << " ";
//     cout << "\n";

//     return 0;
// }