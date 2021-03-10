typedef pair<val_tuple, mpz_class> val_res;
struct cmp_vr {
  bool operator()(const val_res &vr1, const val_res &vr2)
  {
    return vr1.second >= vr2.second;
  }
};
