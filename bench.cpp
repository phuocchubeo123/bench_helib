/* Copyright (C) 2019-2021 IBM Corp.
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */

// This is a sample program for education purposes only.
// It attempts to show the various basic mathematical
// operations that can be performed on both ciphertexts
// and plaintexts.

#include <iostream>
#include <random>

#include <helib/helib.h>
#include "rmfe.cpp"

using namespace std::chrono;
auto start = high_resolution_clock::now();
auto stop = high_resolution_clock::now();
microseconds dur;

#define TIME(X) \
start = high_resolution_clock::now(); \
X; \
stop = high_resolution_clock::now(); \
dur = duration_cast<microseconds>(stop - start); \
cout << "Time taken: " << dur.count() / 1000 << "ms" << "\n";


using namespace std;

mt19937 mt(5);


long power(long x, long i, long p){
  /* Compute power in mod*/
  if (i == 0) return 1;
  int y = power(x, i/2, p);
  return (y * y * ((i & 1) ? x : 1)) % p;
}

long invert(long x, long p){
  /* Inverting in mod*/
  return power(x, p-2, p);
}

vector<long> interpolate(long d, long p){
  /* 
  Generating the evaluate polynomial
  */

  vector<long> ans;
  ans.push_back(1);

  for (int i = 0; i < d; i++){
    vector<long> tmp = ans;
    for (long j = 0; j < ans.size(); j++){
      ans[j] = p - (ans[j] * i) % p;
      if (ans[j] == p) ans[j] = 0;
    }
    ans.push_back(0);
    for (long j = 0; j < tmp.size(); j++){
      (ans[j+1] += tmp[j]) %= p;
    }
  }

  int denom = 1;
  for (long i = 1; i <= d; i++) (denom *= i) %= p;
  denom = invert(denom, p);

  cout << "Before invert\n";
  for (long x: ans) cout << x << " ";
  cout << "\n";

  for (long i = 0; i < ans.size(); i++) (ans[i] *= denom) %= p;
  return ans;
}

int main(int argc, char* argv[])
{
    /*
    Arguments: p, m, r, bits*/

  /*  Example of BGV scheme  */

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

  std::cout << "\n*********************************************************";
  std::cout << "\n*         Basic Mathematical Operations Example         *";
  std::cout << "\n*         =====================================         *";
  std::cout << "\n*                                                       *";
  std::cout << "\n* This is a sample program for education purposes only. *";
  std::cout << "\n* It attempts to show the various basic mathematical    *";
  std::cout << "\n* operations that can be performed on both ciphertexts  *";
  std::cout << "\n* and plaintexts.                                       *";
  std::cout << "\n*                                                       *";
  std::cout << "\n*********************************************************";
  std::cout << std::endl;

  cout << "The ciphertext bits set in this example is: " << bits << "\n";

  std::cout << "Initialising context object..." << std::endl;
  // Initialize context
  // This object will hold information about the algebra created from the
  // previously set parameters
  helib::Context context = helib::ContextBuilder<helib::BGV>()
                               .m(m)
                               .p(p)
                               .r(r)
                               .bits(bits)
                               .c(c)
                               .build();

  // Print the context
  context.printout();
  std::cout << std::endl;

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
  const helib::PubKey& public_key = secret_key;

  // Get the EncryptedArray of the context
  const helib::EncryptedArray& ea = context.getEA();

  // Get the number of slot (phi(m))
  long nslots = ea.size();
  std::cout << "Number of slots: " << nslots << std::endl;

  /* 
  Generating two sets with size equals to the number of slots
  For ease, I will put the first 10 elements of both sets to be equal, while the rest are random.
  */

  long sender[nslots];
  long receiver[nslots];

  for (int i = 0; i < 10; i++){
    sender[i] = receiver[i] = mt();
  }

  for (int i = 10; i < nslots; i++){
    sender[i] = mt();
    receiver[i] = mt();
  }

  cout << "First 30 elements of sender\n";
  for (int i = 0; i < 50; i++) cout << sender[i] << " ";
  cout << "\n";

  cout << "First 30 elements of receiver\n";
  for (int i = 0; i < 50; i++) cout << receiver[i] << " ";
  cout << "\n";

  // The number of bits in an element
  long d = 18;


  // Here we generate the corresponding 18 plaintexts for Sender, and 18 ciphertexts for Receiver
  helib::Ptxt<helib::BGV> ptxt(context);

  vector<helib::Ptxt<helib::BGV>> sender_plaintexts;
  for (int bit = 0; bit < d; bit++){
    for (int i = 0; i < nslots; i++){
      ptxt[i] = (sender[i] >> bit) & 1;
    }
    sender_plaintexts.push_back(ptxt);
  }

  helib::Ctxt ctxt(public_key);

  vector<helib::Ctxt> receiver_ciphertexts;
  for (int bit = 0; bit < d; bit++){
    for (int i = 0; i < nslots; i++){
      ptxt[i] = (receiver[i] >> bit) & 1;
    }
    public_key.Encrypt(ctxt, ptxt);
    receiver_ciphertexts.push_back(ctxt);
  }


  // Preparing all zeros and all ones plaintexts
  helib::Ptxt<helib::BGV> all_ones_ptxt(context);
  for (int i = 0; i < all_ones_ptxt.size(); i++){
    all_ones_ptxt[i] = 1;
  }

  helib::Ptxt<helib::BGV> all_zeros_ptxt(context);
  for (int i = 0; i < all_zeros_ptxt.size(); i++){
    all_zeros_ptxt[i] = 0;
  }

  /*
  Test for the protocol's circuit
  */

  // First step is to xnor all corresponding bits, then sum them up.
  helib::Ctxt bit_sum(public_key);
  public_key.Encrypt(bit_sum, all_zeros_ptxt);

  for (int bit = 0; bit < d; bit++){
    cout << "XNORing ciphertext with plaintext\n";
    TIME(
    ctxt = receiver_ciphertexts[bit];
    ctxt.multByConstant(sender_plaintexts[bit]);
    ctxt.multByConstant((long) 2);
    ctxt.negate();
    ctxt.addConstant(sender_plaintexts[bit]);
    ctxt.addCtxt(receiver_ciphertexts[bit]);
    ctxt.negate();
    ctxt.addConstant((long) 1);
    )

    bit_sum.addCtxt(ctxt);

    // secret_key.Decrypt(ptxt, ctxt);
    // cout << ptxt << "\n\n\n\n";
  }

  secret_key.Decrypt(ptxt, bit_sum);
  cout << "Bit sum: " << ptxt << "\n\n\n\n";

  // We test whether a bit sum is equal to 18, which is the point-to-permute hashed number of bits of 32-bit values
  vector<long> coefficients = interpolate(d, p);

  cout << "Interpolated coefficients:\n";
  for (int i = 0; i < coefficients.size(); i++){
    cout << coefficients[i] << " ";
  }
  cout << "\n";

  for (int i = 0; i <= 18; i++){
    long tot = 0;
    long x = 1;
    for (int j = 0; j <= 18; j++){
      tot += (x * coefficients[j]) % p;
      (x *= i) %= p;
    }
    tot %= p;
    cout << "Interpolated polynomial for x = " << i << ": " << tot << "\n";
  }

  helib::Ctxt all_ones_ctxt(public_key);
  public_key.Encrypt(all_ones_ctxt, all_ones_ptxt);

  vector<helib::Ctxt> small_powers;
  small_powers.push_back(all_ones_ctxt);
  small_powers.push_back(bit_sum);


  cout << "Get power 2:\n";
  small_powers.push_back(bit_sum);
  TIME(small_powers[2].multiplyBy(small_powers[1]));

  cout << "Get power 3:\n";
  small_powers.push_back(small_powers[2]);
  TIME(small_powers[3].multiplyBy(small_powers[1]));

  cout << "Get power 4:\n";
  small_powers.push_back(small_powers[2]);
  TIME(small_powers[4].multiplyBy(small_powers[2]));

  cout << "Get power 5:\n";
  small_powers.push_back(small_powers[4]);
  TIME(small_powers[5].multiplyBy(small_powers[1]));


  vector<helib::Ctxt> big_powers;
  big_powers.push_back(all_ones_ctxt);
  big_powers.push_back(small_powers[4]);

  cout << "Get power 8:\n";
  big_powers.push_back(big_powers[1]);
  TIME(big_powers[2].multiplyBy(big_powers[1]));

  cout << "Get power 12:\n";
  big_powers.push_back(big_powers[2]);
  TIME(big_powers[3].multiplyBy(big_powers[1]));

  cout << "Get power 16:\n";
  big_powers.push_back(big_powers[2]);
  TIME(big_powers[4].multiplyBy(big_powers[2]));


  // Setting the total to be zero first
  helib::Ctxt total(public_key);
  public_key.Encrypt(total, all_zeros_ptxt);

  helib::Ctxt tmp(public_key); // dummy

  for (int i = 0; i <= 4; i++){ // big powers
    helib::Ctxt small_total(public_key);
    public_key.Encrypt(small_total, all_zeros_ptxt);   

    // compute smaller brackets
    for (int j = 0; j <= 3; j++){ // small powes
      tmp = small_powers[j];
      if (i * 4 + j > d) break;
      cout << "Multiplying with constant coefficients";
      TIME(tmp.multByConstant(coefficients[i*4+j]));
      small_total.addCtxt(tmp);
    }

    // adding it to the larger sum, something like x^12 (x^0 + 3x^1 + x^2 + 7x^3)
    cout << "Multiplying with big powers\n";
    TIME(small_total.multiplyBy(big_powers[i]));
    total.addCtxt(small_total);
  }

  cout << "\n\n\nCan we decrypt? " << total.isCorrect() << "\n\n\n";


  // Decrypt the modified ciphertext into a new plaintext
  helib::Ptxt<helib::BGV> new_plaintext_result(context);
  secret_key.Decrypt(new_plaintext_result, total);

  std::cout << "Decrypted Result: " << new_plaintext_result << std::endl;

  

  return 0;
}