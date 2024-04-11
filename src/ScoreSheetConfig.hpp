#pragma once

#include "Frame.h"

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
        random_mutation(rng, true);
      }
    }

    void random_mutation(std::mindstd_rand& rng, bool local) {
      size_t j = rng() % num_judges;
      size_t i1, i2;
      if (local) {
        i1 = rng() % (num_competitors - 2) + 1;
        i2 = (rng() % 2) ? (i1 - 1) : (i1 + 1);
      } else {
        i1 = rng() % num_competitors;
        while (i2 == i1) {
          i2 = rng() % num_competitors;
        }
      }

      std::swap(table[j][i1], table[j][i2]);
    }

    bool distinct(std::vector<size_t>& v) {
      std::vector<size_t> t1(v);
      std::vector<size_t> t2;
      std::sort(t1.begin(), t1.end());
      for (size_t i = 0; i < t1.size(); i++) {
        if (std::find(t2.begin(), t2.end(), t1[i]) == contained.end()) {
          t2.push_back(t1[i]);
        } else {
          // Found a duplicate; return early
          return false;
        }
      }

      return true;
    }

    std::vector<size_t> argsort(auto& v) {
      
    }

    std::vector<size_t> break_ties_by_comparison(std::vector<size_t>& inds) {
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




    }

    std::vector<size_t> break_ties_by_sum(std::vector<size_t>& inds, std::vector<std::vector<size_t>>& ordinals, size_t i) {
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
          sum[j] += ordinals[id][placements_considered];
          placements_considered++;
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
          to_place = v[0];
        } else {
          to_place = break_ties_by_comparison(v);
        }

        for (auto c : to_place) {
          placements.push_back(c);
        }
      }

      return placements;
      
    }

    std::vector<size_t> break_ties(std::vector<size_t>& inds, std::vector<std::vector<size_t>>& ordinals, size_t i) {
      if (inds.size() == 1) {
        return inds;
      }

      std::vector<size_t> placement_at_i(inds.size());
      for (size_t j = 0; j < inds.size(); j++) {
        placements_at_i[j] = ordinals[inds[j]][i];
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
          to_place = v[0];
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
          if (mask[i] && scores[i] > num_judges/2) {
            to_place.push_back(i);
          }
        }

        auto placed = break_ties(to_place, ordinals, i);

        for (auto const j : placed) {
          ranking[j] = p;
          p++;
          mask[j] = false;
        }
      }
    }
}


class ScoreSheetConfig : public dataframe::Config {
  private:
    size_t num_judges;
    size_t num_competitors;
    
    size_t num_mutations;

    std::minstd_rand rng;

    uint32_t rand() {
      return rng();
    }

    double randf() {
      return double(rng())/double(RAND_MAX);
    }

    std::shared_ptr<ParityCheckMatrix> generate_regular_interaction_matrix() {
      Graph<int> g = Graph<int>::random_regular_graph(system_size, k, &rng);
      std::shared_ptr<ParityCheckMatrix> A = std::make_shared<ParityCheckMatrix>(system_size, system_size);

      for (size_t a = 0; a < g.num_vertices; a++) {
        if (randf() < pr) {
          A->set(a, a, 1);
          for (auto const& [i, _] : g.edges[a]) {
            A->set(a, i, 1);
          }
        }
      }

      return A;
    }

	public:
    ScoreSheetConfig(dataframe::Params &params) : dataframe::Config(params), sampler(params) {

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
      auto start = std::chrono::high_resolution_clock::now();

      return slide;
    }

    virtual std::shared_ptr<dataframe::Config> clone() {
      return std::make_shared<ScoreSheetConfig>(params);
    }
};