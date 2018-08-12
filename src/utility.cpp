#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include "utility.hpp"

namespace TinyCDN {
  extern "C" {
    ChunkedCursor::~ChunkedCursor() {
      handle.close();
    }

    void ChunkedCursor::prevChunk(unsigned char *buffer) {
      // TODO
    }

    void ChunkedCursor::nextChunk(unsigned char *buffer) {
      // std::ios::sync_with_stdio();
      std::cout << "numChunks: " << numChunks <<
        " currentChunkNum: " << currentChunkNum <<
        " bufferSize: " << bufferSize <<
        " lastChunkSize: " << lastChunkSize <<
        std::endl;
      isLastChunk = numChunks <= ++currentChunkNum;
      std::cout << "numChunks: " << numChunks <<
        " currentChunkNum: " << currentChunkNum <<
        std::endl;
      forwardsAmount = isLastChunk ? lastChunkSize : bufferSize;

      // for some reason the first chunk doesn't seem to be any data whatsoever,
      // even though the first chunk is BIGGER than the last chunk. HOW?
      handle.seekg(seekPosition);

      handle.read(reinterpret_cast<char*>(buffer), forwardsAmount * sizeof(char));
      std::cout << "Read buffer... | forwardsAmount: " << forwardsAmount << std::endl;
      seekPosition += forwardsAmount;
    }
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
