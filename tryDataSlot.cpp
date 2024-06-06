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
    helib::addFrbMatrices(secret_key);

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

    // In degree and out degree for RMFE
    long in_degree = 22;

    // The number of bits in an element
    long d = 18;

    /////////////////////////

    /*
    Set one dummy plaintext, dummy Slot for the whole file
    */
    helib::Ptxt<helib::BGV> dummy_ptxt(context);
    helib::Ctxt dummy_ctxt(public_key);
    NTL::ZZX poly = dummy_ptxt[0].getData();

    for (int i = 0; i < nslots; i++) {
        cout << "The modulus for plaintext slot " << i << " is: " << dummy_ptxt[i].getG() << "\n";
    }

    ////////////////////////////////////

    /*
    Generating two sets with size equals to the number of slots (nslots * packing in one slot)
    For ease, I will put the first 10 packs of both sets to have one common value, while the rest are random.
    */

    long sender[nslots][in_degree];
    long receiver[nslots][in_degree];

    for (int i = 0; i < 10; i++) {
        sender[i][0] = receiver[i][0] = mt();
        for (int j = 1; j < in_degree; j++)
            sender[i][j] = mt();
        for (int j = 1; j < in_degree; j++)
            receiver[i][j] = receiver[i][0];
    }

    for (int i = 10; i < nslots; i++) {
        receiver[i][0] = mt();
        for (int j = 0; j < in_degree; j++) {
            sender[i][j] = mt();
            receiver[i][j] = receiver[i][0];
        }
    }

    //////////////////////////

    /*
    Read the q-linearized polynomial coefficients from the params file
    */
    ifstream param;
    param.open("params/257_4096_8.txt");

    FIMD rmfe_operator(p, in_degree);

    vector<helib::Ptxt<helib::BGV>> q_linearized_coeff1;
    for (int i = 0; i < 2 * in_degree - 1; i++) {
        for (long j = 0; j < one_slot; j++) {
            long coeff;
            param >> coeff;
            SetCoeff(poly, j, coeff);
        }

        for (int j = 0; j < nslots; j++)
            dummy_ptxt[j] = poly;
        q_linearized_coeff1.push_back(dummy_ptxt);
    }
    rmfe_operator.q_linearized_coeff.push_back(q_linearized_coeff1);

    vector<helib::Ptxt<helib::BGV>> q_linearized_coeff2;
    for (int i = 0; i < 3 * in_degree - 2; i++) {
        for (long j = 0; j < one_slot; j++) {
            long coeff;
            param >> coeff;
            SetCoeff(poly, j, coeff);
        }

        for (int j = 0; j < nslots; j++)
            dummy_ptxt[j] = poly;
        q_linearized_coeff2.push_back(dummy_ptxt);
    }
    rmfe_operator.q_linearized_coeff.push_back(q_linearized_coeff2);

    /*
    Find evaluation points
    */

    vector<long> evaluation_points;
    for (long i = 0; i < in_degree; i++)
        evaluation_points.push_back(i);
    rmfe_operator.set_evaluation_points(evaluation_points);

    ////////////////////////////

    /*
    Packing receiver's elements into plaintexts and ciphertexts
    */

    vector<helib::Ctxt> receiver_ciphertexts;

    cout << "Packing receiver's elements into ciphertexts...\n";
    TIME(for (int bit = 0; bit < d; bit++) {
        for (int i = 0; i < nslots; i++) {
            vector<long> bits_to_pack;
            for (int j = 0; j < in_degree; j++) {
                bits_to_pack.push_back((receiver[i][j] >> bit) & 1);
            }
            rmfe_operator.encode(dummy_ptxt, bits_to_pack, i);
        }
        public_key.Encrypt(dummy_ctxt, dummy_ptxt);
        receiver_ciphertexts.push_back(dummy_ctxt);
    })

    /*
    Packing sender's elements into plaintexts
    */
    vector<helib::Ptxt<helib::BGV>> sender_plaintexts;

    cout << "Packing sender's elements into plaintexts...\n";
    TIME(for (int bit = 0; bit < d; bit++) {
        for (int i = 0; i < nslots; i++) {
            vector<long> bits_to_pack;
            for (int j = 0; j < in_degree; j++) {
                bits_to_pack.push_back((sender[i][j] >> bit) & 1);
            }
            rmfe_operator.encode(dummy_ptxt, bits_to_pack, i);
        }
        sender_plaintexts.push_back(dummy_ptxt);
    })

    ////////////////////////

    // Preparing all zeros and all ones plaintexts
    helib::Ptxt<helib::BGV> all_ones_ptxt(context);
    vector<long> all_one_slot;
    for (int i = 0; i < in_degree; i++)
        all_one_slot.push_back(1);
    for (int i = 0; i < all_ones_ptxt.size(); i++) {
        rmfe_operator.encode(all_ones_ptxt, all_one_slot, i);
    }
    helib::Ctxt all_ones_ctxt(public_key);
    public_key.Encrypt(all_ones_ctxt, all_ones_ptxt);

    helib::Ptxt<helib::BGV> all_zeros_ptxt(context);
    vector<long> all_zero_slot;
    for (int i = 0; i < in_degree; i++)
        all_zero_slot.push_back(0);
    for (int i = 0; i < all_zeros_ptxt.size(); i++) {
        rmfe_operator.encode(all_zeros_ptxt, all_zero_slot, i);
    }

    ////////////////////////

    helib::Ctxt bit_sum(public_key);

    cout << "Time to do all XNORs is:\n";
    TIME(for (int bit = 0; bit < d; bit++) {
        cout << "XNORing one plaintext with one ciphertext in bit " << bit << "...\n";
        TIME(dummy_ptxt = sender_plaintexts[bit]; dummy_ptxt += dummy_ptxt; all_ones_ptxt *= (long)(p - 1); dummy_ptxt += all_ones_ptxt;
             dummy_ctxt = receiver_ciphertexts[bit];
             dummy_ctxt.multByConstant(dummy_ptxt);
             dummy_ptxt = sender_plaintexts[bit];
             dummy_ptxt *= (long)(p - 1);
             dummy_ptxt += all_ones_ptxt;
             dummy_ctxt.addConstant(dummy_ptxt);
             all_ones_ptxt *= (long)(p - 1);)

        if (bit == 0)
            bit_sum = dummy_ctxt;
        else
            bit_sum.addCtxt(dummy_ctxt);
    } rmfe_operator.q_linearize(bit_sum, 0);)

    cout << "\n";

    TIME(
        vector<helib::Ctxt> small_powers; vector<helib::Ctxt> big_powers; cout << "Adding power 0\n"; small_powers.push_back(all_ones_ctxt);
        cout << "Adding power 1\n";
        small_powers.push_back(bit_sum);
        for (int i = 2; i < 5; i++) {
            small_powers.push_back(small_powers[i / 2]);
            cout << "Taking small power  " << i << "\n";
            TIME(small_powers[i].multiplyBy(small_powers[i - i / 2]));
            cout << "q-linearize small power " << i << "\n";
            TIME(rmfe_operator.q_linearize(small_powers[i], 0));
        }

        big_powers.push_back(all_ones_ctxt);
        big_powers.push_back(small_powers[4]);
        for (int i = 2; i < 5; i++) {
            big_powers.push_back(big_powers[i / 2]);
            cout << "Taking big powers " << i * 4 << "\n";
            TIME(big_powers[i].multiplyBy(big_powers[i - i / 2]));
            if (i == 2)
                rmfe_operator.q_linearize(big_powers[i], 0);
        }

        vector<long> bit_compare_coefficients = generate_bit_compare_polynomial(d, p);

        helib::Ctxt big_res(public_key);
        for (int i = 0; i < 5; i++) {
            helib::Ctxt small_res(public_key);
            for (int j = 0; j < 4; j++) {
                if (i * 4 + j > 18)
                    dummy_ctxt = small_powers[j];
                dummy_ctxt.multByConstant(bit_compare_coefficients[i * 4 + j]);
                if (j == 0)
                    small_res = dummy_ctxt;
                else
                    small_res.addCtxt(dummy_ctxt);
            }
            TIME(small_res.multiplyBy(big_powers[i]));
            if (i == 0)
                big_res = small_res;
            else
                big_res.addCtxt(small_res);
        }

        TIME(rmfe_operator.q_linearize(big_res, 1));

    )

    return 0;
}