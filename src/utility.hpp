#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <functional>

namespace TinyCDN::Utility {

extern "C" {
  //! Wrapper around an input stream that iterates over a buffer of size bufferSize.
  struct ChunkedCursor {
    std::size_t bufferSize;
    std::uintmax_t size;
    std::size_t seekPosition;
    std::ifstream handle;

    std::size_t numChunks = 0;
    bool isLastChunk = false;
    std::size_t forwardsAmount = 0;
    size_t currentChunkNum = 0;
    std::size_t lastChunkSize;

    inline ChunkedCursor(std::size_t bufferSize,
                         std::uintmax_t size,
                         std::size_t seekPosition)
      : bufferSize(bufferSize),
        size(size),
        seekPosition(seekPosition),
        numChunks(size / bufferSize + 1),
        lastChunkSize(size % bufferSize) {
    }

    inline ChunkedCursor(std::size_t bufferSize,
                         std::uintmax_t size,
                         std::size_t seekPosition,
                         std::function<void(std::ifstream&)> setupStream)
      : bufferSize(bufferSize),
        size(size),
        seekPosition(seekPosition),
        numChunks(size / bufferSize + 1),
        lastChunkSize(size % bufferSize) {
      setupStream(handle);
    }

    ~ChunkedCursor();

    void prevChunk(unsigned char* buffer);
    void nextChunk(unsigned char* buffer);
  };
}

typedef const uintmax_t Size;

constexpr std::size_t operator""_kB(unsigned long long v) {
  return 1024u * v;
}

constexpr std::size_t operator""_mB(unsigned long long v) {
  return 1048576u * v;
}

constexpr std::size_t operator""_gB(unsigned long long v) {
  return 1073741824 * v;
}

/*! Takes a container and converts to a comma-separated string
 * Treats a comma'd string value as a container of string-convertible values.
 * Overload for specific types to create k-tuples from stored text
 * This is used to convert structures into a format that can be created at run-time by reading from a file.
 */
template <typename t>
inline std::string asCSV(t container) {
  std::string csv;

  // TODO optimize
  if (container.size() == 1) {
    csv.append(container[0]);
    return csv;
  }

  for (auto elem : container) {
    // TODO don't cast here, use a "statusfield"
    csv.append(static_cast<std::string>(elem));
    csv.append(",");
  }
  return csv;
};

/*! Takes a comma-separated string and outputs a vector of the string's values
 */
std::vector<std::string> fromCSV(std::string csv);
}
