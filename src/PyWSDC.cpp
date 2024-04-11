#include "ScoreSheetConfig.hpp"

#include <PyDataFrame.hpp>

NB_MODULE(xorsat_bindings, m) {
  INIT_CONFIG();
  EXPORT_CONFIG(ScoreSheetConfig);
}

