#include "common.h"

using namespace std;

long estimateCtxtSize(const helib::Context &context, long offset)
{
  // Return in bytes.

  // We assume that the size of each element in the DCRT is BINIO_64BIT

  // sizeof(BINIO_EYE_CTXT_BEGIN) = 4;
  // BINIO_32BIT = 4
  // sizeof(long) = BINIO_64BIT = 8
  // xdouble = s * sizeof(long) = 2 * BINIO_64BIT = 16

  // We assume that primeSet after encryption is context.ctxtPrimes
  // We assume we have exactly 2 parts after encryption
  // We assume that the DCRT prime set is the same as the ctxt one

  long size = 0;

  // Header metadata
  size += 24;

  // Begin eye-catcher
  size += 4;

  // Begin Ctxt metadata
  // 64 = header_size = ptxtSpace (long) + intFactor (long) + ptxtMag (xdouble)
  //                    + ratFactor (xdouble) + noiseBound (xdouble)
  size += 64;

  // primeSet.write(str);
  // size of set (long) + each prime (long)
  size += 8 + context.getCtxtPrimes().card() * 8;

  // Begin Ctxt content size
  // write_raw_vector(str, parts);
  // Size of the parts vector (long)
  size += 8;

  long part_size = 0;
  // Begin CtxtPart size

  // skHandle.write(str);
  // powerOfS (long) + powerOfX (long) + secretKeyID (long)
  part_size += 24;

  // Begin DCRT size computation

  // this->DoubleCRT::write(str);
  // map.getIndexSet().write(str);
  // size of set (long) + each prime (long)
  part_size += 8 + context.getCtxtPrimes().card() * 8;

  // DCRT data write as write_ntl_vec_long(str, map[i]);
  // For each prime in the ctxt modulus chain
  //    size of DCRT column (long) + size of each element (long) +
  //    size of all the slots (column in DCRT) (PhiM long elements)
  long dcrt_size = (8 + 8 * context.getPhiM()) * context.getCtxtPrimes().card();

  part_size += dcrt_size;

  // End DCRT size
  // End CtxtPart size

  size += 2 * part_size; // 2 * because we assumed 2 parts
  // End Ctxt content size

  // End eye-catcher
  size += 4;

  return size + offset;
}

// Function to compute the modular inverse of a under modulo p
// Uses Fermat's little theorem: a^(p-1) ≡ 1 (mod p), hence a^(p-2) ≡ a^(-1) (mod p)
long modInverse(long a, long p)
{
  long result = 1;
  long exponent = p - 2; // Fermat's little theorem exponent

  while (exponent > 0)
  {
    if (exponent % 2 == 1)
    {
      result = (result * a) % p;
    }
    a = (a * a) % p;
    exponent /= 2;
  }
  return result;
}

vector<long> generate_bit_compare_polynomial(long d, long p)
{
  /*
  Generating the evaluate polynomial
  */

  vector<long> ans;
  ans.push_back(1);

  for (int i = 0; i < d; i++)
  {
    vector<long> tmp = ans;
    for (long j = 0; j < ans.size(); j++)
    {
      ans[j] = p - (ans[j] * i) % p;
      if (ans[j] == p)
        ans[j] = 0;
    }
    ans.push_back(0);
    for (long j = 0; j < tmp.size(); j++)
    {
      (ans[j + 1] += tmp[j]) %= p;
    }
  }

  int denom = 1;
  for (long i = 1; i <= d; i++)
    (denom *= i) %= p;
  denom = modInverse(denom, p);

  cout << "Before invert\n";
  for (long x : ans)
    cout << x << " ";
  cout << "\n";

  for (long i = 0; i < ans.size(); i++)
    (ans[i] *= denom) %= p;
  return ans;
}
