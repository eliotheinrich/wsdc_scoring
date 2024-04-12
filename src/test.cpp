#include "ScoreSheetConfig.hpp"
#include <iostream>


using namespace dataframe;
using namespace dataframe::utils;

int main() {
  Params p;
  p["num_judges"] = 1.0;
  p["num_competitors"] = 3.0;
  p["num_samples"] = 1.0;
  p["num_tables"] = 1.0;
  p["num_mutations"] = 1000.0;

  ScoreSheetConfig c(p);

  auto slide = c.compute(1);
  std::cout << slide.to_string() << std::endl;
}
