#include <iostream>
#include <random>
#include <fstream>

#include <NTL/ZZX.h>
#include <helib/helib.h>
#include "rmfe.cpp"

using namespace std;

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

mt19937 mt(5);

/*
Macro for q-lineaization
*/
#define Q_LINEARIZE(ctxt) \
vector<helib::Ctxt> small_frob; \
small_frob.push_back(ctxt); \
for (int i = 1; i < 6; i++){ \
  small_frob.push_back(small_frob[i-1]); \
  small_frob[i].frobeniusAutomorph((long) 1); \
} \
cout << "Done getting small frobs\n"; \ 
helib::Ctxt tot = small_frob[0]; \
for (long j = 0; j < 6; j++){ \
  helib::Ctxt small_tot = small_frob[0]; \
  for (int i = 0; i < 6; i++){ \
    if (j * 6 + i >= q_linearized_coeff.size()) continue; \
    if (i == 0){ \
      small_tot.multByConstant(q_linearized_coeff[j*6+i]); \
      continue; \
    } \
    else{ \
      ctxt = small_frob[i]; \
      ctxt.multByConstant(q_linearized_coeff[j*6+i]); \
      small_tot.addCtxt(ctxt); \
    } \
  } \
  small_tot.frobeniusAutomorph(j); \
  if (j == 0) tot = small_tot; \
  else tot.addCtxt(small_tot); \
} \
ctxt = tot;

/*
Macro for FIMD ciphertext-plaintext multiplication
*/
#define FIMD_PTXT_MULTIPLY(ctxt, ptxt) \
ctxt.multByConstant(ptxt); \
cout << "Performing q-linearization:\n"; \
TIME(Q_LINEARIZE(ctxt)); 

/* 
Macro for FIMD ciphertext-ciphertext multiplication
*/
#define FIMD_CTXT_MULTIPLY(ctxt, other_ctxt) \
ctxt.multiplyBy(other_ctxt); \
cout << "Performing q-linearization:\n"; \
TIME(Q_LINEARIZE(ctxt)); 


vector<long> generate_bit_compare_polynomial(long d, long p){
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
  denom = modInverse(denom, p);

  cout << "Before invert\n";
  for (long x: ans) cout << x << " ";
  cout << "\n";

  for (long i = 0; i < ans.size(); i++) (ans[i] *= denom) %= p;
  return ans;
}

int main(int argc, char* argv[])
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
  helib::addFrbMatrices(secret_key);

  // Public key management
  // Set the secret key (upcast: SecKey is a subclass of PubKey)
  const helib::PubKey& public_key = secret_key;

  // Get the EncryptedArray of the context
  const helib::EncryptedArray& ea = context.getEA();

  // Get the number of slot (phi(m))
  long nslots = ea.size();
  std::cout << "Number of slots: " << nslots << std::endl;

  long phi = context.getPhiM();
  cout << "Phi of m: " << phi << "\n";

  long one_slot = phi / nslots;
  cout << "Degree in one slot: " << one_slot << "\n";

  /* Notable for generalization later*/
  long one_packing = 15;

  ////////////////////////////////////


  /* 
  Generating two sets with size equals to the number of slots (nslots * packing in one slot)
  For ease, I will put the first 10 packs of both sets to have one common value, while the rest are random.
  */

  long sender[nslots][one_packing];
  long receiver[nslots][one_packing];

  for (int i = 0; i < 10; i++){
    sender[i][0] = receiver[i][0] = mt();
    for (int j = 1; j < one_packing; j++) sender[i][j] = mt();
    for (int j = 1; j < one_packing; j++) receiver[i][j] = receiver[i][0];
  }

  for (int i = 10; i < nslots; i++){
    receiver[i][0] = mt();
    for (int j = 0; j < one_packing; j++){
      sender[i][j] = mt();
      receiver[i][j] = receiver[i][0];
    }
  }

  // The number of bits in an element
  long d = 18;


  //////////////////////////



  /*
  Find 15 evaluation points mod 31 such that x^15 - 1 = 0
  */

  cout << "Evaluation points such that x^15 - 1 = 0\n";
  int cnt = 0;
  vector<long> evaluation_points;
  for (int i = 0; i < 31; i++){
    int u = 1; 
    for (int j = 0; j < 15; j++) (u *= i) %= p;
    if (u == 1){
      cnt++;
      cout << i << " ";
      evaluation_points.push_back(i);
    }
  }

  cout << "\n";
  cout << "Total number of evaluation points: " << cnt << "\n";


  /*
  Generate Lagrange basis from evaluation points
  This modifies the lagrange_basis variable in rmfe.cpp
  */

  generate_lagrange_basis(evaluation_points, p);

  /////////////////////////

  /*
  Set one dummy plaintext, dummy Slot for the whole file
  */
  helib::Ptxt<helib::BGV> ptxt(context);
  helib::Ctxt ctxt(public_key);
  NTL::ZZX poly = ptxt[0].getData(); 

  //////////////////////////

  /*
  Read the q-linearized polynomial coefficients from the params file
  */
  ifstream param;
  param.open("params/31_1261_15.txt");

  vector<helib::Ptxt<helib::BGV>> q_linearized_coeff;

  for (int i = 0; i < 29; i++){
    for (long j = 0; j < one_slot; j++){
      long coeff; param >> coeff;
      SetCoeff(poly, j, coeff);
    }

    for (int j = 0; j < nslots; j++) ptxt[j] = poly;
    q_linearized_coeff.push_back(ptxt);
  }

  for (int i = 0; i < 29; i++){
    cout << q_linearized_coeff[i][0] << "\n";
  }


  ////////////////////////////


  /* 
  Packing receiver's elements into plaintexts and ciphertexts
  */

  vector<helib::Ctxt> receiver_ciphertexts;

  cout << "Packing receiver's elements into ciphertexts...\n";
  TIME(
  for (int bit = 0; bit < d; bit++){
    for (int i = 0; i < nslots; i++){
      vector<long> bits_to_pack;
      for (int j = 0; j < one_packing; j++){
        bits_to_pack.push_back((receiver[i][j] >> bit) & 1);
      }
      vector<long> corresponding_field_element = lagrange_interpolation(bits_to_pack, p);
      for (long j = 0; j < corresponding_field_element.size(); j++){
        SetCoeff(poly, j, corresponding_field_element[j]);
      }
      ptxt[i] = poly;
    }
    public_key.Encrypt(ctxt, ptxt);
    receiver_ciphertexts.push_back(ctxt);
  }
  )


  /*
  Packing sender's elements into plaintexts
  */
  vector<helib::Ptxt<helib::BGV>> sender_plaintexts;

  cout << "Packing sender's elements into plaintexts...\n";
  TIME(
  for (int bit = 0; bit < d; bit++){
    for (int i = 0; i < nslots; i++){
      vector<long> bits_to_pack;
      for (int j = 0; j < one_packing; j++){
        bits_to_pack.push_back((sender[i][j] >> bit) & 1);
      }
      vector<long> corresponding_field_element = lagrange_interpolation(bits_to_pack, p);
      for (long j = 0; j < corresponding_field_element.size(); j++) SetCoeff(poly, j, corresponding_field_element[j]);
      ptxt[i] = poly;
    }
    sender_plaintexts.push_back(ptxt);
  }
  )

  ////////////////////////


  // Preparing all zeros and all ones plaintexts
  helib::Ptxt<helib::BGV> all_ones_ptxt(context);
  vector<long> all_one_slot;
  for (int i = 0; i < one_packing; i++) all_one_slot.push_back(1);
  vector<long> all_one_coeff = lagrange_interpolation(all_one_slot, p);
  for (long i = 0; i < one_packing; i++) SetCoeff(poly, i, all_one_coeff[i]);
  for (int i = 0; i < all_ones_ptxt.size(); i++){
    all_ones_ptxt[i] = poly;
  }
  helib::Ctxt all_ones_ctxt(public_key);
  public_key.Encrypt(all_ones_ctxt, all_ones_ptxt);

  helib::Ptxt<helib::BGV> all_zeros_ptxt(context);
  vector<long> all_zero_slot;
  for (int i = 0; i < one_packing; i++) all_zero_slot.push_back(0);
  vector<long> all_zero_coeff = lagrange_interpolation(all_zero_slot, p);
  for (long i = 0; i < one_packing; i++) SetCoeff(poly, i, all_zero_coeff[i]);
  for (int i = 0; i < all_zeros_ptxt.size(); i++){
    all_zeros_ptxt[i] = poly;
  }

  ////////////////////////

  helib::Ctxt bit_sum(public_key);

  for (int bit = 0; bit < d; bit++){
    cout << "XNORing one plaintext with one ciphertext in bit " << bit << "...\n";
    TIME(
      ptxt = sender_plaintexts[bit];
      ptxt += ptxt;
      all_ones_ptxt *= (long) (p-1);
      ptxt += all_ones_ptxt;
      ctxt = receiver_ciphertexts[bit];
      FIMD_PTXT_MULTIPLY(ctxt, ptxt);
      ptxt = sender_plaintexts[bit];
      ptxt *= (long) (p-1);
      ptxt += all_ones_ptxt;
      ctxt.addConstant(ptxt);
      all_ones_ptxt *= (long) (p-1);
    )

    if (bit == 0) bit_sum = ctxt;
    else bit_sum.addCtxt(ctxt);
  }

  vector<helib::Ctxt> small_powers;
  vector<helib::Ctxt> big_powers;
  cout << "Adding power 0\n";
  small_powers.push_back(all_ones_ctxt);
  cout << "Adding power 1\n";
  small_powers.push_back(bit_sum);
  for (int i = 2; i < 5; i++){
    small_powers.push_back(small_powers[i / 2]);
    cout << "Taking powers " << i << "\n"; 
    TIME(FIMD_CTXT_MULTIPLY(small_powers[i], small_powers[i - i / 2]));
  }

  vector<long> bit_compare_coefficients = generate_bit_compare_polynomial(d, p);

  return 0;
}