#ifndef UTILITIES_H
#define UTILITIES_H

#include <unordered_set>
#include <string>
#include <functional>
#include <algorithm>

#include "base/errors.h"
#include "hardware/EventDrivenGP.h"
#include "tools/BitSet.h"
#include "tools/math.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/map_utils.h"

template<size_t TAG_WIDTH>
emp::BitSet<TAG_WIDTH> GenRandTag(emp::Random & rnd, const emp::vector<emp::BitSet<TAG_WIDTH>> & unique_from=emp::vector<emp::BitSet<TAG_WIDTH>>()) {
  using tag_t = emp::BitSet<TAG_WIDTH>;
  emp_assert(unique_from.size() < emp::Pow2(TAG_WIDTH), "Tag width is not large enough to be able to guarantee requested number of unique tags");
  tag_t new_tag(rnd, 0.5); // Make a random tag.
  bool guarantee_unique = (bool)unique_from.size();
  while (guarantee_unique) {
    guarantee_unique = false;
    for (size_t i = 0; i < unique_from.size(); ++i) {
      if (unique_from[i] == new_tag) {
        guarantee_unique = true;
        new_tag.Randomize(rnd);
        break;
      }
    }
  }
  return new_tag;
}

template<size_t TAG_WIDTH>
emp::vector<emp::BitSet<TAG_WIDTH>> GenRandTags(emp::Random & rnd, size_t count, bool guarantee_unique=false,
                                           const emp::vector<emp::BitSet<TAG_WIDTH>> & unique_from=emp::vector<emp::BitSet<TAG_WIDTH>>()) {
  using tag_t = emp::BitSet<TAG_WIDTH>;
  emp_assert(!guarantee_unique || (unique_from.size()+count <= emp::Pow2(TAG_WIDTH)), "Tag width is not large enough to be able to guarantee requested number of unique tags");

  std::unordered_set<uint32_t> uset; // Used to ensure all generated tags are unique.
  emp::vector<tag_t> new_tags;
  for (size_t i = 0; i < unique_from.size(); ++i) uset.emplace(unique_from[i].GetUInt(0));
  for (size_t i = 0; i < count; ++i) {
    new_tags.emplace_back(tag_t());
    new_tags[i].Randomize(rnd);
    if (guarantee_unique) {
      uint32_t tag_int = new_tags[i].GetUInt(0);
      while (true) {
        if (!emp::Has(uset, tag_int)) {
          uset.emplace(tag_int);
          break;
        } else {
          new_tags[i].Randomize(rnd);
          tag_int = new_tags[i].GetUInt(0);
        }
      }
    }
  }
  return new_tags;
}

template<size_t TAG_WIDTH>
emp::vector<emp::BitSet<TAG_WIDTH>> GenHadamardMatrix() {
  emp_assert((TAG_WIDTH & (TAG_WIDTH - 1)) == 0, "Bit set width must be power of 2", TAG_WIDTH);
  // todo - assert n power of two
  emp::vector<emp::BitSet<TAG_WIDTH>> matrix(TAG_WIDTH);
  size_t n = TAG_WIDTH;
  matrix[0][0] = true;
  for (size_t k = 1; k < n; k += k) {
    for (size_t i = 0; i < k; ++i) {
      for (size_t j = 0; j < k; ++j) {
        matrix[i+k].Set(j, matrix[i][j]);
        matrix[i].Set(j+k, matrix[i][j]);
        matrix[i+k].Set(j+k, !matrix[i][j]);
      }
    }
  }
  return matrix;
}

/// Computes simple matching coefficient (https://en.wikipedia.org/wiki/Simple_matching_coefficient).
template <size_t NUM_BITS>
size_t HammingDist(const emp::BitSet<NUM_BITS> & in1, const emp::BitSet<NUM_BITS> & in2) {
  emp_assert(NUM_BITS > 0);
  // return (double)((in1 & in2).CountOnes() + (~in1 & ~in2).CountOnes()) / (double)NUM_BITS;
  return (in1^in2).CountOnes();
}

/// Function copied over (and modified) from Emily Dolson's memic_model branch of
/// her Empirical fork
static inline std::string to_titlecase(std::string value) {
  constexpr int char_shift = 'a' - 'A';
  bool next_upper = true;
  for (size_t i = 0; i < value.size(); i++) {
    if (next_upper && value[i] >= 'a' && value[i] <= 'z') {
      value[i] = (char) (value[i] - char_shift);
    } else if (!next_upper && value[i] >= 'A' && value[i] <= 'Z') {
      value[i] = (char) (value[i] + char_shift);
    }
    if (value[i] == ' ') {
      next_upper = true;
    } else {
      next_upper = false;
    }
  }
  return value;
}

/// Function copied over from Emily Dolson's memic_model branch of
/// her Empirical fork
template <typename T>
inline std::string join(const emp::vector<T> & v, std::string join_str) {
  if (v.size() == 0) {
    return "";
  } else if (v.size() == 1) {
    return emp::to_string(v[0]);
  } else {
    std::stringstream res;
    res << v[0];
    for (size_t i = 1; i < v.size(); i++) {
      res << join_str;
      res << emp::to_string(v[i]);
    }
    return res.str();
  }
}

#endif