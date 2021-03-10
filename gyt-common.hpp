#include <vector>

using namespace std;

typedef unsigned long long bigint;
typedef vector<bigint> val_tuple;
struct polynomial {
  bigint k;			// number of variables
  val_tuple coeffs;		// coefficients
  vector<val_tuple> monomials;	// exponents
};

extern const string header;
extern const string underline;

void read_input (polynomial &p, bigint &B);

template <typename T>
T power(T x, bigint n);

bigint eval(const polynomial &p, const val_tuple &val);
long double nthroot (const long double A, const bigint n);
val_tuple get_bounds (const polynomial &p, const bigint &B);
bool test_bound (const val_tuple &val, const val_tuple &bound, int k);
void statistics (bigint msize,
		 const string what,
		 const bigint &maxstack,
		 const bigint &split,
		 const bigint &nback,
		 const bigint &dbl);
