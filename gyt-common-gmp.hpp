#include <vector>

using namespace std;

typedef vector<mpz_class> val_tuple;
struct polynomial {
  unsigned int k;	       	// number of variables
  val_tuple coeffs;		// coefficients
  vector<val_tuple> monomials;	// exponents
};

extern const string header;
extern const string underline;

void read_input (polynomial &p, mpz_class &B);

template <typename T>
T power(T x, mpz_class n);

mpz_class eval(const polynomial &p, const val_tuple &val);
long double nthroot (const long double A, const mpz_class n);
val_tuple get_bounds (const polynomial &p, const mpz_class &B);
bool test_bound (const val_tuple &val, const val_tuple &bound, int k);
void statistics (mpz_class msize,
		 const string what,
		 const mpz_class &maxstack,
		 const mpz_class &split,
		 const mpz_class &nback,
		 const mpz_class &dbl);
