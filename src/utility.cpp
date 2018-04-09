#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include "utility.hpp"

namespace TinyCDN {

template <typename t>
std::string asCSV(t container) {
  std::string csv;
  for (auto elem : container) {
    // TODO don't cast here, use a "statusfield"
    csv.append(static_cast<std::string>(elem));
    csv.append(",");
  }
  return csv;
}

std::vector<std::string> fromCSV(std::string csv) {
  std::vector<std::string> values;

  // Check if there is only one value, or no value at all
  if (csv.length() != 0) {
    auto nextComma = csv.find(',');

    if (nextComma == std::string::npos) {
      // One value case
      values.push_back(csv);
    }
    else {
      // append head of CSV first
      values.push_back(csv.substr(0, nextComma));
      // initial value for next CSV
      auto nextCSV = csv.substr(nextComma+1);
      nextComma = nextCSV.find(',');

      while(nextComma != std::string::npos) {
        values.push_back(nextCSV.substr(nextComma));

        nextCSV = nextCSV.substr(nextComma+1);
        nextComma = nextCSV.find(',');
      };

      // append tail of CSV
      values.push_back(nextCSV);
    }
  }

  return values;
}
}

// struct StringConverter {
//   virtual  convertInput(std::string input) = 0;

//   // Converter(input) : input(input) {}
// };
