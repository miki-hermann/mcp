/**************************************************************************
 *                                                                        *
 *                                                                        *
 *        Multiple Characterization Problem (MCP)                         *
 *                                                                        *
 * Author:   Miki Hermann                                                 *
 * e-mail:   hermann@lix.polytechnique.fr                                 *
 * Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France             *
 *                                                                        *
 * Author:   Gernot Salzer                                                *
 * e-mail:   gernot.salzer@tuwien.ac.at                                   *
 * Address:  Technische Universitaet Wien, Vienna, Austria                *
 *                                                                        *
 * Author:   César Sagaert                                                *
 * e-mail:   cesar.sagaert@ensta-paris.fr                                 *
 * Address:  ENSTA Paris, Palaiseau, France                               *
 *                                                                        *
 * Version: all                                                           *
 *     File:    src-mekong/mcp-bucket.hpp                                 *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <array>

#include "mcp-matrix+formula.hpp"

namespace bucket {

  struct Pattern {
    Sign lsign;    // left sign
    Sign rsign;    // right sign
    size_t lcoord; // left coordinate
    size_t rcoord; // right coordinate

    Pattern() = default;
    ~Pattern() = default;
    constexpr Pattern(Sign ls, size_t lc, Sign rs, size_t rc) noexcept
      : lsign(ls), rsign(rs), lcoord(lc), rcoord(rc) {}

    constexpr bool operator==(const Pattern &other) const noexcept {
      return lsign == other.lsign && rsign == other.rsign &&
	lcoord == other.lcoord && rcoord == other.rcoord;
    }
  };

  typedef std::array<integer, 2> Point;
  constexpr size_t X = 0, Y = 1;

  struct Clause {
    Sign lsign, rsign;
    integer lval, rval;
    size_t lcoord, rcoord;
  };

  constexpr bool valid(const Clause &c) noexcept {
    return c.lsign != lnone && c.rsign != lnone;
  }

  //------------------------------------------------------------------------------

  typedef std::unordered_map<Pattern, std::set<Point>> Bucket;

  void insert(const Clause &, Bucket &);
  Formula get_formula(const Bucket &, const size_t &);
  bool sat_bucket(const Row &, const Bucket &);
  void print_bucket(const Bucket &);

} // namespace bucket

template <> struct std::hash<bucket::Pattern> {
  using argument_type = bucket::Pattern;
  using result_type = std::size_t;

  result_type operator()(const argument_type &pattern) const {
    size_t left_hash = hash_combine(std::hash<Sign>{}(pattern.lsign),
                                    std::hash<size_t>{}(pattern.lcoord));
    size_t right_hash = hash_combine(std::hash<Sign>{}(pattern.rsign),
                                     std::hash<size_t>{}(pattern.rcoord));
    return hash_combine(left_hash, right_hash);
    // string strg = to_string(pattern.lsign) + ":"
    //   + to_string(pattern.lcoord) + ":"
    //   + to_string(pattern.rsign) + ":"
    //   + to_string(pattern.rcoord);
    // return hash<string>()(strg);
  }
};
