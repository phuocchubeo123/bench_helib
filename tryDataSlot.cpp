#include <iostream>
#include <random>

#include <NTL/ZZX.h>
#include <helib/helib.h>

using namespace std;

int main(int argc, char* argv[])
{
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

  helib::Ptxt<helib::BGV> ptxt(context);
  helib::PolyMod x = ptxt[0]; 

  cout << "Poly Mod: " << x << "\n";

  cout << "G: " << x.getG() << "\n";

  NTL::ZZX u = x.getData();

  cout << "u: " << u << "\n";

  SetCoeff(u, (long)2, (long)3);
  SetCoeff(u, (long)1, (long)5);

  cout << "u: " << u << "\n";

  cout << "New ZZ[x] " << u << "\n";

    cout << "Before\n";
    cout << "ptxt[0]: " << ptxt[0] << "\n";
    cout << ptxt[0].getData() << "\n";

    ptxt[0] = u;

    cout << "ptxt[0]: " << ptxt[0] << "\n";
    cout << ptxt[0].getData() << "\n";

    cout << "Automorphism:\n";
    ptxt.frobeniusAutomorph(1);
    cout << ptxt[0].getData() << "\n";

    ptxt.frobeniusAutomorph(1);
    cout << ptxt[0].getData() << "\n";

    return 0;
}