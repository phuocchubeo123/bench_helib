#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

#define TIME(X) \
for (int __ = 0; __ < 1; __++){ \
auto start = high_resolution_clock::now(); \
X; \
auto stop = high_resolution_clock::now(); \
auto dur = duration_cast<microseconds>(stop - start); \
cout << "Time taken: " << dur.count() / 1000 << "ms" << "\n"; \
}