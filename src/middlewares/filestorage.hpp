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

//! A chunk of a Haystack's contents.
/*!

*/
struct HaystackBlock {
  //! The size of the buffer this block contains
  const Size size;
  //! Contents of the block
  std::string_view buffer;

  HaystackBlock() : size(256_kB) {}
  HaystackBlock(Size size) : size(size) {}
  HaystackBlock(Size size, std::string_view buffer) : size(size), buffer(buffer) {}

  HaystackBlock& operator*() { return *this; }
};

/*!
  \brief A generic iterator class for operating on a Haystack
  HaystackBlockType is the type used to deduce the return type of the iterator.
  StreamType is supposed to be a stream-like interface such as std::ifstream or std::ofstream.
  The iteration procedure returns a std::shared_future which guarantees an overloaded operation to execute on the current 256kB block.
  The intention of the HaystackCursor is to allow multithreaded operation on the same Haystack.
  However, currently there is little protection as in a lock or guard on the Haystack's contents.
*/
template <typename HaystackBlockType, typename StreamType>
class HaystackCursor {
  //inline HaystackBlock forward();
  using iterator_category = std::forward_iterator_tag;
  using iterator_type = HaystackBlock;

public:
  std::size_t endBlock;
  //! The current seek position of this cursor on the provided handle
  std::size_t seekPosition;

  //! The input or output stream that operate() will use as its input or output
  StreamType handle;
  //! The absolute size of the Haystack, needed to prevent a potential buffer overflow error
  const Size haystackSize;
  //! The unexecuted haystack block operation of type HaystackBlockType
  std::optional<std::shared_future<HaystackBlockType>>* futureBlock;

  /*!
  * \brief The operation that gets executed on the current block
  * \param buffer An empty value that should be used as the output container
  * \param forwardsAmount This will either be 256kB or smaller.
  * A forwardsAmount <256kB would indicate this is either the only block, or the last of a series of blocks.
  */
  virtual void operate(std::string_view buffer, const std::size_t forwardsAmount);

  //! Returns an operation that will update the seekPosition after running an operation on the current block.
  HaystackCursor<HaystackBlockType, StreamType> operator++() {
    std::packaged_task<HaystackBlockType()> task([&]{
      // If a file is < 256kB, it will be sent as a single HaystackBlock
      // The final block will not have the fixed block size of 256kB.
      std::string_view buffer;

      // haystack limit OR file limit?
      // TODO specialize this per read/write cursor
      auto const forwardsAmount = 256_kB > (haystackSize.size - seekPosition) ? 256_kB : (haystackSize.size - seekPosition);

      this->operate(buffer, forwardsAmount);

      seekPosition += forwardsAmount;

      return HaystackBlockType{forwardsAmount, std::move(buffer)};
    });

    &futureBlock->emplace(task.get_future().share());
    return *this;
  }

  std::optional<std::shared_future<HaystackBlockType>>& operator*() { return *futureBlock; }
  // TODO make this compare HayStacks...
  bool operator==(const HaystackCursor<HaystackBlockType, StreamType> rhs) const { return seekPosition == rhs.seekPosition; }
  bool operator!=(const HaystackCursor<HaystackBlockType, StreamType> rhs) const { return seekPosition != rhs.seekPosition; }
  bool operator<=(const HaystackCursor<HaystackBlockType, StreamType> rhs) const { return seekPosition <= rhs.seekPosition; }

  HaystackCursor<HaystackBlockType, StreamType>(StreamType handle, std::size_t seekPosition, Size haystackSize)
    : handle(handle), seekPosition(seekPosition), haystackSize(haystackSize)
  {}

  virtual ~HaystackCursor<HaystackBlockType, StreamType>();

};

class HaystackReadCursor : public HaystackCursor<const HaystackBlock, std::ifstream&> {
  inline virtual void operate(std::string_view buffer, const std::size_t forwardsAmount) override {
    handle.seekg(static_cast<long>(seekPosition));
    handle.read(reinterpret_cast<char*>(&buffer), static_cast<std::streamsize>(seekPosition + forwardsAmount));
  }

public:

  HaystackReadCursor cbegin() const { return HaystackReadCursor{this->handle, 0, this->haystackSize}; }
  HaystackReadCursor cend(std::size_t endPosition) const { return HaystackReadCursor{this->handle, endPosition, this->haystackSize}; }

  HaystackReadCursor(std::ifstream& haystackHandle, std::size_t seekPosition, Size haystackSize)
    : HaystackCursor<const HaystackBlock, std::ifstream&>(haystackHandle, seekPosition, haystackSize)
  {}


};

//! A HaystackCursor that takes an input stream and writes its contents to the Haystack
class HaystackWriteCursor : public HaystackCursor<HaystackBlock, std::ofstream&> {
  //! The input handle that will be written to the Haystack
  std::ifstream& fileHandle;
  //! The seekPosition equivalent for the current file. Starts at 0 to copy the entire file.
  std::size_t fileSeekPosition = 0;

  /*!
   * \brief operate Copies a file's contents to the provided HaystackBlock buffer.
   * \param buffer The future HaystackBlock's buffer, i.e. its contents
   * \param forwardsAmount How far to seek in the file's contents forwards after
   */
  inline virtual void operate(std::string_view buffer, const std::size_t forwardsAmount) override {
    fileHandle.seekg(static_cast<long>(fileSeekPosition));
    fileHandle.read(reinterpret_cast<char*>(&buffer), static_cast<std::streamsize>(forwardsAmount));
    fileSeekPosition += forwardsAmount;

    handle.seekp(static_cast<long>(seekPosition));
    handle << buffer;
  }

public:

  HaystackWriteCursor begin() const { return HaystackWriteCursor{this->fileHandle, this->handle, 0, this->haystackSize}; }
  HaystackWriteCursor end(std::size_t endPosition) const { return HaystackWriteCursor{this->fileHandle, this->handle, endPosition, this->haystackSize}; }

  HaystackWriteCursor(std::ifstream& fileHandle, std::ofstream& haystackHandle, std::size_t seekPosition, Size haystackSize)
    : HaystackCursor<HaystackBlock, std::ofstream&>(haystackHandle, seekPosition, haystackSize), fileHandle(fileHandle)
  {}
};

// ReadWriteCursor - for video encoding, encryption etc.
/*!
 * \brief The Haystack, a stucture of a fixed Size that allows storing multiple files within the same file.
 */
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

  // TODO lock the input/output handles
  HaystackWriteCursor write(std::ifstream& fileHandle, std::size_t position) {
    return HaystackWriteCursor{fileHandle, this->outputHandle, position, this->allocatedSize};
  }

//  ReadWriteHaystackCursor begin;
//  ReadOnlyHaystackCursor cbegin;
//  ReadWriteHaystackCursor end;
//  ReadOnlyHaystackCursor cend;

  HaystackReadCursor read(std::size_t position) {
    return HaystackReadCursor{this->inputHandle, position, this->allocatedSize};
  }

  inline void addFile(std::size_t position, std::ifstream file, Size fileSize) {
    // addedFile
    for (auto it = this->write(file, position); it <= it.end(position + fileSize.size); ++it) {
      auto block = *it;
      block.value().get();
      // TODO verify hashed chunk
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
