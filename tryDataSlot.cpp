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

  ////////////////////////////////////


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

  /////////////////////////

  /*
  Set one dummy plaintext, dummy Slot for the whole file
  */
  helib::Ptxt<helib::BGV> ptxt(context);
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

  return 0;
}