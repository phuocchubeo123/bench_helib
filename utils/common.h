#include <helib/helib.h>
#include <vector>

long estimateCtxtSize(const helib::Context &context, long offset);
long modInverse(long a, long p);
std::vector<long> generate_bit_compare_polynomial(long d, long p);