#include <iostream>
#include <vector>
#include <climits>
#include <gmpxx.h>
#include "gyt-common-gmp.hpp"

using namespace std;

void read_input (polynomial &p, mpz_class &B) {
  cout << header << endl;
  cout << underline << endl;
  cout << endl;
  
  cerr << "+++ Input B: ";
  cin >> B;
  cout << endl << "*** B = " << B << endl;
  cerr << "+++ Number of variables: ";
  cin >> p.k;
  cout << endl << "*** k = " << p.k << endl;
  cerr << "+++ Monomials in the form 'coefficient exponent ... exponent', ";
  cerr << "one per line" << endl;
  cerr << "+++ Terminate with CTRL-D" << endl;
  
  mpz_class temp;
  while (cin >> temp) {
    p.coeffs.push_back(temp);
    val_tuple exponents;
    mpz_class exp;
    for (unsigned int i = 0; i < p.k; ++i) {
      cin >> exp;
      exponents.push_back(exp);
    }
    p.monomials.push_back(exponents);
    cerr << "*** monomial = " << temp;
    for (unsigned int i = 0; i < p.k; ++i)
      if (exponents[i] > 0)
	cerr << " (x_" << i+1 << ")^" << exponents[i];
    cerr << endl;
  }

  cout << "*** equation: ";
  bool plus = false;
  for (unsigned int i = 0; i < p.coeffs.size(); ++i) {
    if (plus)
      cout << " + ";
    else
      plus = true;
    cout << p.coeffs[i];
    for (unsigned int j = 0; j < p.k; ++j)
      if (p.monomials[i][j] > 0)
	cout << " (x_" << j+1 << ")^" << p.monomials[i][j];
  }
  cout <<  " = " << B << endl;
}

template <typename T>
T power(T x, mpz_class n) {
  if (n == 0)
    return 1;
  mpz_class y = 1;
  while (n > 1)
    if (n % 2 == 0) {
      x *= x;
      n >>= 1;
    } else {
      y *= x;
      x *= x;
      n = (n-1)/2;
    }
  return x * y;
}

mpz_class eval(const polynomial &p, const val_tuple &val) {
  mpz_class add = 0;
  for (unsigned int i = 0; i < p.coeffs.size(); ++i) {
    mpz_class mult = p.coeffs[i];
    for (unsigned j = 0; j < p.k; ++j)
      mult *= power(val[j], p.monomials[i][j]);
    add += mult;
  }
  return add;
}

mpf_class nthroot (const mpf_class A, const mpz_class n) {
  mpf_class oldx, x = A/n;
  do {
    oldx = x;
    x = ((n-1)*oldx + A/power(oldx, n-1))/n;
  } while (abs(oldx-x) > 0.01);
  return x;
}

val_tuple get_bounds (const polynomial &p, const mpz_class &B) {
  val_tuple bound(p.k);;
  vector<unsigned long long> minpos(p.k);
  val_tuple minexp(p.k, UINT_MAX);
  for (unsigned int j = 0; j < p.monomials.size(); ++j)
    for (unsigned int i = 0; i < p.k; ++i)
      if (p.monomials[j][i] > 0 && p.monomials[j][i] < minexp[i]) {
	minexp[i] = p.monomials[j][i];
	minpos[i] = j;
      }
  for (unsigned int i = 0; i < p.k; ++i) {
    mpf_class bpc = B/p.coeffs[minpos[i]];
    bound[i] = nthroot(bpc, minexp[i]) + 1;
  }

  for (unsigned int i = 0; i < bound.size(); ++i) {
    val_tuple val(p.k, 0);
    val[i] = bound[i];
    while (val[i] >= 0 && eval(p, val) > B)
      val[i]--;
    bound[i] = val[i];
  }

  return bound;
}

bool test_bound (const val_tuple &val, const val_tuple &bound, int k) {
  for (unsigned int i = 0; i < k; ++i)
    if (val[i] > bound[i])
      return false;
  return true;
}

void statistics (mpz_class msize,
		 const string what,
		 const mpz_class &maxstack,
		 const mpz_class &split,
		 const mpz_class &nback,
		 const mpz_class &dbl) {
  cout << "*** memo size       = " << msize;
  const string kmg = " KMG";
  unsigned int idx = 0;
  while (msize > 1024 && idx < 3) {
    msize /= 1024;
    idx++;
  }
  if (idx > 0)
    cout << " (" << msize << kmg[idx] << ")";
  cout << endl;
  cout << "    max " << what << " size  = " << maxstack << endl;
  cout << "    # of splits     = " << split << endl;
  cout << "    # of backtracks = " << nback << endl;
  cout << "    doubles reached = " << dbl << endl;
}
