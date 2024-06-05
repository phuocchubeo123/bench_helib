#include <bits/stdc++.h>

using namespace std;

const int N = 1e5;

int is_prime[N];
vector<int> primes;

void find_prime()
{
    for (int i = 0; i < N; i++)
        is_prime[i] = 1;
    is_prime[0] = is_prime[1] = 0;
    for (int i = 2; i < N; i++) {
        if (is_prime[i] == 1) {
            primes.push_back(i);
            for (int j = 2; j * i < N; j++)
                is_prime[i * j] = 0;
        }
    }
}

int phi[N];

void find_phi()
{
    phi[0] = phi[1] = 1;
    for (int i = 2; i < N; i++) {
        if (is_prime[i] == 1)
            phi[i] = i - 1;
        else {
            for (int p : primes) {
                if (i % p == 0) {
                    int j = i;
                    int small_phi = p - 1;
                    int prime_power = p;
                    j /= p;
                    while (j % p == 0) {
                        small_phi *= p;
                        prime_power *= p;
                        j /= p;
                    }
                    phi[i] = phi[i / prime_power] * small_phi;
                    break;
                }
            }
        }
    }
}

int main()
{
    find_prime();
    find_phi();

    for (int i = 1; i < 15; i++) {
        cout << i << ": " << is_prime[i] << " " << phi[i] << "\n";
    }

    int POWER_OF_TWO = (1 << 12);
    int PHI_POWER_OF_TWO = POWER_OF_TWO / 2;
    int desired_batch = PHI_POWER_OF_TWO / 2;

    for (int p : primes) {
        if (p < 31)
            continue;
        if (p > 600)
            break;
        cout << "Currently investigating prime: " << p << "\n";

        int cnt = 0;
        for (int i = POWER_OF_TWO; i <= POWER_OF_TWO; i++) {
            int curr = p;
            for (int j = 1; j < i; j *= 2) {
                if (curr % i == 1) {
                    int packing = (PHI_POWER_OF_TWO / j) * min(j / 2, p);
                    if (packing >= desired_batch) {
                        cout << "Found good packing parameter: "
                             << "m = " << i << ", pack = " << packing << ", one slot = " << j << ", no of slots: " << PHI_POWER_OF_TWO / j << "\n";
                        cnt++;
                    }
                    break;
                }
                (curr *= curr) %= i;
            }

            if (cnt > 5)
                break;
        }
    }
}