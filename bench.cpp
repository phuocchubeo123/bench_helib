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

#include <helib/helib.h>

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

long power(long x, long i, long p){
  if (i == 0) return 1;
  int y = power(x, i/2, p);
  return (y * y * ((i & 1) ? x : 1)) % p;
}

int invert(int x, int p){
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

  // Create a vector of long with nslots elements
  helib::Ptxt<helib::BGV> ptxt(context);
  // Set it with numbers 0..nslots - 1
  // ptxt = [0] [1] [2] ... [nslots-2] [nslots-1]
  for (int i = 0; i < ptxt.size(); ++i) {
    ptxt[i] = (i % 19);
  }

  // Print the plaintext
  std::cout << "Initial Plaintext: " << ptxt << std::endl;


  /*
  Test level
  */
  // for (int i = 1; i < 10; i++){
  //     // Create a ciphertext object
  //     helib::Ctxt ctxt(public_key);
  //     // Encrypt the plaintext using the public_key
  //     public_key.Encrypt(ctxt, ptxt);

  //     /********** Operations **********/
  //     // Ciphertext and plaintext operations are performed
  //     // "entry-wise".

  //     // Create a plaintext for decryption
  //     helib::Ptxt<helib::BGV> plaintext_result(context);

  //     // Square the ciphertext
  //     // [0] [1] [2] [3] [4] ... [nslots-1]
  //     // -> [0] [1] [4] [9] [16] ... [(nslots-1)*(nslots-1)]

  //     cout << "First square: " << "\n";
  //     TIME(ctxt.multiplyBy(ctxt));
  //     // Decrypt the modified ciphertext
  //     secret_key.Decrypt(plaintext_result, ctxt);
  //     cout << "Second square: " << "\n";
  //     TIME(ctxt.multiplyBy(ctxt));
  //     // Decrypt the modified ciphertext
  //     secret_key.Decrypt(plaintext_result, ctxt);
  //     cout << "Third square: " << "\n";
  //     TIME(ctxt.multiplyBy(ctxt));
  //     // Decrypt the modified ciphertext
  //     secret_key.Decrypt(plaintext_result, ctxt);
  //     cout << "Fourth square: " << "\n";
  //     TIME(ctxt.multiplyBy(ctxt));
  //     // Decrypt the modified ciphertext
  //     secret_key.Decrypt(plaintext_result, ctxt);
  //     cout << "Fifth square: " << "\n";
  //     TIME(ctxt.multiplyBy(ctxt));
  //     // Decrypt the modified ciphertext
  //     secret_key.Decrypt(plaintext_result, ctxt);

  //     cout << "Can we decrypt?" << ctxt.isCorrect() << "\n";
  // }

  /*
  Test for the protocol's circuit
  */

  // We test whether a bit sum is equal to 18, which is the point-to-permute hashed number of bits of 32-bit values
  long d = 18;
  vector<long> coefficients = interpolate(d, p);

  cout << "Interpolated coefficients:\n";
  for (int i = 0; i < coefficients.size(); i++){
    cout << coefficients[i] << " ";
  }
  cout << "\n";

  helib::Ctxt power1(public_key);
  public_key.Encrypt(power1, ptxt);

  helib::Ptxt<helib::BGV> all_ones_ptxt(context);
  for (int i = 0; i < all_ones_ptxt.size(); i++){
    all_ones_ptxt[i] = 1;
  }

  helib::Ctxt all_ones_ctxt(public_key);
  public_key.Encrypt(all_ones_ctxt, all_ones_ptxt);

  vector<helib::Ctxt> small_powers;
  small_powers.push_back(all_ones_ctxt);
  small_powers.push_back(power1);


  cout << "Get power 2:\n";
  small_powers.push_back(power1);
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
  return 0;
}