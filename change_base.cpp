#include <NTL/ZZX.h>
#include <fstream>
#include <iostream>
#include <random>
#include "fimd/fimd.h"
#include "utils/common.h"
#include "utils/timeUtil.h"

using namespace std;

mt19937 mt(5);

int main(int argc, char *argv[])
{
    // I want to dedicate this experiment solely on Modulo p = 31 first.
    // What's next to do:
    // 1. Find 15 evaluation points that satisfy x^15 - 1 = 0 (done)
    // 2. Find out the appropriate RMFE
    // 3. Pack them into slot

    // Plaintext prime modulus
    unsigned long p = stoi(argv[1]);

    // Cyclotomic polynomial - defines phi(m)
    unsigned long m = stoi(argv[2]);
    // Hensel lifting (default = 1)
    unsigned long r = stoi(argv[3]);
    // Number of bits of the modulus chain
    unsigned long bits = stoi(argv[4]);
    // Number of columns of Key-Switching matrix (default = 2 or 3)
    unsigned long c = 2;

    cout << "The ciphertext bits set in this example is: " << bits << "\n";

    std::cout << "Initialising context object..." << std::endl;
    // Initialize context
    // This object will hold information about the algebra created from the
    // previously set parameters
    helib::Context context = helib::ContextBuilder<helib::BGV>().m(m).p(p).r(r).bits(bits).c(c).build();

    // Print the context
    context.printout();
    std::cout << std::endl;

    cout << "Estimated ciphertext size: " << estimateCtxtSize(context, 1) << "\n";

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;

    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    helib::SecKey secret_key(context);
    // Generate the secret key
    secret_key.GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;
    // Compute key-switching matrices that we need
    helib::addSome1DMatrices(secret_key);

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    const helib::PubKey &public_key = secret_key;

    // Get the EncryptedArray of the context
    const helib::EncryptedArray &ea = context.getEA();

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;

    long phi = context.getPhiM();
    cout << "Phi of m: " << phi << "\n";

    long one_slot = phi / nslots;
    cout << "Degree in one slot: " << one_slot << "\n";


    /*
    Set one dummy plaintext, dummy Slot for the whole file
    */
    helib::Ptxt<helib::BGV> dummy_ptxt(context);
    helib::Ctxt dummy_ctxt(public_key);

    /*
    Benchmarking generating plaintext first
    */
    vector<helib::Ptxt<helib::BGV>> ptxt_coeffs;

    // std::cout << "Timing to create 100 plaintexts\n";
    // TIME(
    // for (int i = 0; i < 100; i++){
    //     for (int j = 0; j < nslots; j++){
    //         dummy_ptxt[j] = (i + j + 5) % p;
    //     }
    //     ptxt_coeffs.push_back(dummy_ptxt);
    // }
    // )


    /*
    Benchmarking generating all powers
    */

    // First, we need to compute postage stamp base

    vector<long> bases = {1, 8, 11, 64, 102};
    vector<vector<long>> paths;
    for (long i = 1; i < 345; i++)
    {
        if (i == 1){
            vector<long> path = {1};
            paths.push_back(path);
            continue;
        }

        long best_length = 100;
        long best_j = 100;
        for (long j: bases)
        {
            if (j >= i) break;
            if (paths[i-j].size() < best_length){
                best_length = paths[i-j].size();
                best_j = j;
            }
        }


        std::cout << "length and best j" << best_length << " " << best_j << "\n";

        vector<long> path = paths[i - best_j];
        // path.push_back(best_j);
        // paths.push_back(path);
    }

    std::cout << "Printing out extremal postage stamp bases for check\n";
    // for (long i = 1; i < 10; i++){
    //     std::cout << i << ": ";
    //     for (long j: paths[i]){
    //         std::cout << j << " ";
    //     }
    //     std::cout << "\n";
    // }
}