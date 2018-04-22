#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <iterator>

#include <optional>
#include <future>
#include <experimental/filesystem>

#include "../utility.hpp"

namespace fs = std::experimental::filesystem;


namespace TinyCDN::Middleware::FileStorage {

struct HaystackBlock {
  const Size size;
  std::string_view buffer;

  HaystackBlock() : size(256_kB) {}
  HaystackBlock(Size size) : size(size) {}
  HaystackBlock(Size size, std::string_view buffer) : size(size), buffer(buffer) {}

  HaystackBlock& operator*() { return *this; }
};

template <typename HaystackType, typename StreamType>
class HaystackCursor {
  //inline HaystackBlock forward();
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = HaystackBlock;

public:
  std::size_t endBlock;
  std::size_t seekPosition;

  StreamType handle;
  const Size haystackSize;
  std::optional<std::shared_future<HaystackType>>* futureBlock;

  virtual void operate(std::string_view buffer, const std::size_t forwardsAmount);

  HaystackCursor<HaystackType, StreamType> operator++() {
    std::packaged_task<HaystackType()> task([&]{
      // If a file is < 256kB, it will be sent as a single HaystackBlock
      // The final block will not have the fixed block size of 256kB.
      std::string_view buffer;

      auto const forwardsAmount = 256_kB > (haystackSize.size - seekPosition) ? 256_kB : (haystackSize.size - seekPosition);

      this->operate(buffer, forwardsAmount);

      seekPosition += forwardsAmount;

      return HaystackType{forwardsAmount, std::move(buffer)};
    });

    &futureBlock->emplace(task.get_future().share());
    return *this;
  }

  std::optional<std::shared_future<HaystackType>>& operator*() { return *futureBlock; }
  // TODO make this compare HayStacks...
  bool operator==(const HaystackCursor<HaystackType, StreamType> rhs) const { return seekPosition == rhs.seekPosition; }
  bool operator!=(const HaystackCursor<HaystackType, StreamType> rhs) const { return seekPosition != rhs.seekPosition; }

  HaystackCursor<HaystackType, StreamType>(StreamType handle, std::size_t seekPosition, Size haystackSize)
    : handle(handle), seekPosition(seekPosition), haystackSize(haystackSize)
  {}

  virtual ~HaystackCursor<HaystackType, StreamType>();

};

class HaystackReadCursor : public HaystackCursor<const HaystackBlock, std::ifstream&> {
  inline virtual void operate(std::string_view buffer, const std::size_t forwardsAmount) override {
    handle.seekg(static_cast<long>(seekPosition));
    handle.read(reinterpret_cast<char*>(&buffer), static_cast<std::streamsize>(seekPosition + forwardsAmount));
  }

  HaystackReadCursor cbegin() const { return HaystackReadCursor{this->handle, 0, this->haystackSize}; }
  HaystackReadCursor cend() const { return HaystackReadCursor{this->handle, this->haystackSize.size, this->haystackSize}; }

  HaystackReadCursor(std::ifstream& haystackHandle, std::size_t seekPosition, Size haystackSize)
    : HaystackCursor<const HaystackBlock, std::ifstream&>(haystackHandle, seekPosition, haystackSize)
  {}


};

class HaystackWriteCursor : public HaystackCursor<HaystackBlock, std::ofstream&> {
  std::ifstream& fileHandle;
  std::size_t fileSeekPosition = 0;

  inline virtual void operate(std::string_view buffer, const std::size_t forwardsAmount) override {
    fileHandle.seekg(static_cast<long>(fileSeekPosition));
    fileHandle.read(reinterpret_cast<char*>(&buffer), static_cast<std::streamsize>(forwardsAmount));
    fileSeekPosition += forwardsAmount;

    handle.seekp(static_cast<long>(seekPosition));
    handle << buffer;
  }

public:

  HaystackWriteCursor begin() const { return HaystackWriteCursor{this->fileHandle, this->handle, 0, this->haystackSize}; }
  HaystackWriteCursor end() const { return HaystackWriteCursor{this->fileHandle, this->handle, this->haystackSize.size, this->haystackSize}; }

  HaystackWriteCursor(std::ifstream& fileHandle, std::ofstream& haystackHandle, std::size_t seekPosition, Size haystackSize)
    : HaystackCursor<HaystackBlock, std::ofstream&>(haystackHandle, seekPosition, haystackSize), fileHandle(fileHandle)
  {}
};

// ReadWriteCursor - for video encoding, encryption etc.

struct Haystack
{
public:
  // static size
  Size allocatedSize;
  fs::path location;
  std::ifstream inputHandle;
  std::ofstream outputHandle;

  const bool preallocated = false;

  void allocate();


//  HaystackReadCursor cseek(std::size_t position) const {
//    return HaystackReadCursor{this->outputHandle, position};
//  }

  HaystackWriteCursor write(std::ifstream& fileHandle, std::size_t position) {
    return HaystackWriteCursor{fileHandle, this->outputHandle, position, this->allocatedSize};
  }

//  ReadWriteHaystackCursor begin;
//  ReadOnlyHaystackCursor cbegin;
//  ReadWriteHaystackCursor end;
//  ReadOnlyHaystackCursor cend;


  inline void addFile(std::size_t position, std::ifstream file) {
    // addedFile
    for (auto it = this->write(file, position); it != it.end(); ++it) {
      auto block = *it;
      block.value().get();
    }
  }


  Haystack(const Haystack&);

  inline Haystack(Size allocatedSize, fs::path location, bool preallocated)
    : allocatedSize(allocatedSize), location(location), preallocated(preallocated) {
    if (!preallocated) {
      allocate();
  }
  }

};
}
