#pragma once

#include "Frame.h"


template<typename T>
static std::vector<size_t> argsort(const std::vector<T> &array) {
    std::vector<size_t> indices(array.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&array](int left, int right) -> bool {
                  // sort indices according to corresponding array element
                  return array[left] < array[right];
              });

    return indices;
}

static double swap_distance(std::vector<size_t>& r1, std::vector<size_t>& r2) {
  double wrong_count = 0;

  for (size_t i1 = 0; i1 < r1.size(); i1++) {
    size_t i2;
    for (size_t k = 0; k < r2.size(); k++) {
      if (r2[k] == r1[i1]) {
        i2 = k;
        break;
      }
    }

    if (i1 != i2) {
      wrong_count++;
    }
  }

  return wrong_count / 2.0;
}

static bool distinct(std::vector<size_t>& v) {
  std::vector<size_t> t1(v);
  std::vector<size_t> t2;
  std::sort(t1.begin(), t1.end());
  for (size_t i = 0; i < t1.size(); i++) {
    if (std::find(t2.begin(), t2.end(), t1[i]) == t2.end()) {
      t2.push_back(t1[i]);
    } else {
      // Found a duplicate; return early
      return false;
    }
  }
  // Found no duplicates; return
  return true;
}


class ScoreSheet {
  private:
    size_t num_judges;
    size_t num_competitors;
    std::vector<std::vector<size_t>> table;

  public:
    ScoreSheet(size_t num_judges, size_t num_competitors) : num_judges(num_judges), num_competitors(num_competitors) {
      table = std::vector<std::vector<size_t>>(num_judges, std::vector<size_t>(num_competitors));
      for (size_t i = 0; i < num_judges; i++) {
        std::iota(table[i].begin(), table[i].end(), 0);
      }
    }

    ScoreSheet(size_t num_judges, size_t num_competitors, size_t num_mutations, std::minstd_rand& rng) : ScoreSheet(num_judges, num_competitors) {
      for (size_t i = 0; i < num_mutations; i++) {
        auto [j, i1i2] = generate_random_mutation(rng, true);
        auto [i1, i2] = i1i2;
        apply_mutation(j, i1, i2);
      }
    }

    std::string to_string() const {
      std::string s = "[";
      for (size_t i = 0; i < num_judges; i++) {
        if (i != 0) {
          s += " ";
        }

        s += "[";

        for (size_t j = 0; j < num_competitors; j++) {
          s += std::to_string(table[i][j]);
          if (j != num_competitors - 1) {
            s += " ";
          }
        }

        s += "]";
        if (i == num_judges - 1) {
          s += "]";
        } else {
          s += "],\n";
        }
      }

      return s;
    }

    std::pair<size_t, std::pair<size_t, size_t>> generate_random_mutation(std::minstd_rand& rng, bool local) {
      size_t j = rng() % num_judges;
      size_t i1 = 0;
      size_t i2 = 0;
      if (local) {
        i1 = rng() % (num_competitors - 2) + 1;
        i2 = (rng() % 2) ? (i1 - 1) : (i1 + 1);
      } else {
        i1 = rng() % num_competitors;
        while (i2 == i1) {
          i2 = rng() % num_competitors;
        }
      }

      return {j, {i1, i2}};
    }

    void apply_mutation(size_t j, size_t i1, size_t i2) {
      std::swap(table[j][i1], table[j][i2]);
    }

    void apply_mutation(size_t j, std::vector<size_t>& s) {
      std::vector<size_t> tmp(table[j]);
      for (size_t i = 0; i < num_competitors; i++) {
        tmp[i] = table[j][s[i]];
      }
      table[j] = tmp;
    }

    std::vector<size_t> break_ties_by_comparison(const std::vector<size_t>& inds) const {
      std::vector<int> best_scores(inds.size(), 0);

      for (size_t j = 0; j < num_judges; j++) {
        size_t min_ind = 0;
        size_t min_score = RAND_MAX;

        for (size_t i = 0; i < inds.size(); i++) {
          size_t score = table[j][inds[i]];
          if (score < min_score) {
            min_score = score;
            min_ind = i;
          }
        }

        best_scores[min_ind]--;
      }

      auto sorted_inds = argsort(best_scores);
      std::vector<size_t> to_return(inds.size());
      for (size_t i = 0; i < inds.size(); i++) {
        to_return[i] = sorted_inds[i];
      }

      return to_return;
    }

    std::vector<size_t> break_ties_by_sum(const std::vector<size_t>& inds, const std::vector<std::vector<size_t>>& ordinals, size_t i) const {
      std::vector<size_t> sums(inds.size(), 0);

      size_t placements_considered = i + 1;
      for (size_t j = 0; j < inds.size(); j++) {
        size_t ij = inds[j];
        for (size_t k = 0; k < placements_considered; k++) {
          sums[j] += ordinals[ij][k];
        }
      }

      // CHECK
      while (!distinct(sums) && placements_considered < num_competitors) {
        for (size_t j = 0; j < inds.size(); j++) {
          size_t ij = inds[j];
          sums[j] += ordinals[ij][placements_considered];
          placements_considered++;
        }
      }

      std::map<size_t, std::vector<size_t>> partition;
      for (size_t j = 0; j < inds.size(); j++) {
        size_t k = sums[j];
        if (!partition.contains(k)) {
          partition.emplace(k, std::vector<size_t>());
        }

        partition[k].push_back(inds[j]);
      }

      std::vector<size_t> placements;
      for (auto const& [k, v] : partition) {
        std::vector<size_t> to_place;
        if (v.size() == 1) {
          to_place = v;
        } else {
          to_place = break_ties_by_comparison(v);
        }

        for (auto c : to_place) {
          placements.push_back(c);
        }
      }
      return placements;
    }

    std::vector<size_t> break_ties(const std::vector<size_t>& inds, const std::vector<std::vector<size_t>>& ordinals, size_t i) const {
      if (inds.size() == 1) {
        return inds;
      }

      std::vector<size_t> placement_at_i(inds.size());
      for (size_t j = 0; j < inds.size(); j++) {
        placement_at_i[j] = ordinals[inds[j]][i];
      }

      std::map<size_t, std::vector<size_t>> partition;
      for (size_t j = 0; j < inds.size(); j++) {
        size_t k = placement_at_i[j];
        if (!partition.contains(k)) {
          partition.emplace(k, std::vector<size_t>());
        }

        partition[k].push_back(inds[j]);
      }

      std::vector<size_t> placements;
      for (auto const& [k, v] : partition) {
        std::vector<size_t> to_place;
        if (v.size() == 1) {
          to_place = v;
        } else {
          to_place = break_ties_by_sum(v, ordinals, i);
        }

        for (auto c : to_place) {
          placements.push_back(c);
        }
      }

      return placements;
    }

    std::vector<std::vector<size_t>> get_ordinals() const {
      std::vector<std::vector<size_t>> ordinals(num_competitors, std::vector<size_t>(num_competitors));
      for (size_t i = 0; i < num_competitors; i++) {
        for (size_t j = 0; j < num_competitors; j++) {
          size_t k = 0;
          for (size_t p = 0; p < num_judges; p++) {
            if (table[p][i] <= j) {
              k++;
            }
          }
          ordinals[i][j] = k;
        }
      }

      return ordinals;
    }

    std::vector<size_t> placements() {
      auto ordinals = get_ordinals();

      std::vector<bool> mask(num_competitors, true);
      std::vector<size_t> ranking(num_competitors);

      size_t p = 0;

      for (size_t i = 0; i < num_competitors; i++) {
        std::vector<size_t> scores(num_competitors);
        for (size_t j = 0; j < num_competitors; j++) {
          scores[j] = ordinals[j][i];
        }

        std::vector<size_t> to_place;
        for (size_t j = 0; j < num_competitors; j++) {
          if (mask[j] && scores[j] > num_judges/2) {
            to_place.push_back(j);
          }
        }

        auto placed = break_ties(to_place, ordinals, i);

        for (auto const j : placed) {
          ranking[j] = p;
          p++;
          mask[j] = false;
        }
      }

      return ranking;
    }

    double mutation_distance(size_t j, size_t i1, size_t i2) {
      std::vector<size_t> r1 = placements();
      std::cout << "before mutation: \n" << to_string() << "\n";
      std::cout << "[ "; for (auto r : r1) { std::cout << r << " "; } std::cout << "]\n";
      apply_mutation(j, i1, i2);
      std::vector<size_t> r2 = placements();
      std::cout << "after mutation: \n" << to_string() << "\n";
      std::cout << "[ "; for (auto r : r2) { std::cout << r << " "; } std::cout << "]\n";
      apply_mutation(j, i1, i2);
      return swap_distance(r1, r2);
    }

    std::vector<double> mutation_distance(size_t num_samples, std::minstd_rand& rng) {
      std::vector<double> samples(num_samples);
      for (size_t i = 0; i < num_samples; i++) {
        auto [j, inds] = generate_random_mutation(rng, false);
        auto [i1, i2] = inds;
        samples[i] = mutation_distance(j, i1, i2);
      }

      return samples;
    }
};


class ScoreSheetConfig : public dataframe::Config {
  private:
    size_t num_judges;
    size_t num_competitors;
    
    size_t num_mutations;
    size_t num_samples;
    size_t num_tables;

    std::minstd_rand rng;

    uint32_t rand() {
      return rng();
    }

    double randf() {
      return double(rng())/double(RAND_MAX);
    }

	public:
    ScoreSheetConfig(dataframe::Params &params) : dataframe::Config(params) {
      num_judges = dataframe::utils::get<int>(params, "num_judges");
      num_competitors = dataframe::utils::get<int>(params, "num_competitors");
      num_mutations = dataframe::utils::get<int>(params, "num_mutations");
      num_samples = dataframe::utils::get<int>(params, "num_samples", 1);
      num_tables = dataframe::utils::get<int>(params, "num_tables", 1);

      int seed = dataframe::utils::get<int>(params, "seed", 0);
      if (seed == 0) {
        thread_local std::random_device rd;
        seed = rd();
      }

      rng.seed(seed);
    }

    ScoreSheetConfig()=default;

    ~ScoreSheetConfig()=default;

    virtual dataframe::DataSlide compute(uint32_t num_threads) {
      dataframe::DataSlide slide;

      auto start = std::chrono::high_resolution_clock::now();

      slide.add_data("instability");
      for (size_t i = 0; i < num_tables; i++) {
        ScoreSheet T(num_judges, num_competitors, num_mutations, rng);
        std::cout << T.to_string() << std::endl;

        std::vector<double> instability = T.mutation_distance(num_samples, rng);
        slide.push_data("instability", instability);
      }

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::micro> duration = end - start;

      slide.add_data("time");
      slide.push_data("time", duration.count());


      return slide;
    }

    virtual std::shared_ptr<dataframe::Config> clone() {
      return std::make_shared<ScoreSheetConfig>(params);
    }
};
