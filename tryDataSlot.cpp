#include <iostream>
#include <random>

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

  cout << "The Phi is: " << context.getPhiM() << "\n";

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

  helib::Ptxt<helib::BGV> ptxt(context);
  helib::PolyMod x = ptxt[0]; 

  cout << "Poly Mod: " << x << "\n";

  // cout << "G: " << x.getG() << "\n";

  NTL::ZZX u = x.getData();

  // cout << "u: " << u << "\n";

  SetCoeff(u, (long)2, (long)3);
  SetCoeff(u, (long)1, (long)5);

  // cout << "u: " << u << "\n";

  cout << "New ZZ[x] " << u << "\n";

  cout << "Before\n";
  cout << "ptxt[0]: " << ptxt[0] << "\n";
  cout << ptxt[0].getData() << "\n";

  ptxt[0] = u;

  cout << "ptxt[0]: " << ptxt[0] << "\n";
  cout << ptxt[0].getData() << "\n";

  cout << "Automorphism:\n";
  ptxt.frobeniusAutomorph(1);
  // cout << ptxt[0].getData() << "\n";

  ptxt.frobeniusAutomorph(1);
  // cout << ptxt[0].getData() << "\n";



    // Trying Frobenius Automorphism on Ciphertext

  for (int i = 0; i < nslots; i++) ptxt[i] = (long) (i % 16);
  helib::Ctxt ctxt(public_key);
  public_key.Encrypt(ctxt, ptxt);

  cout << "Frobenius automorphism:\n";
  TIME(
    for (int i = 0; i < 20; i++){
      ctxt.frobeniusAutomorph((long) 1);
      // cout << "done Automorph, can still decrypt? " << ctxt.isCorrect() << "\n";
    }
  )

  return 0;
}