#include <optional>

#include "storedfile.hpp"
#include "../../utility.hpp"

using TinyCDN::Utility::operator""_kB;

namespace TinyCDN::Middleware::FileStorage {
StoredFile::StoredFile(Size size, fs::path location, bool temporary, std::unique_ptr<std::unique_lock<std::shared_mutex>> lock)
  : size(size), temporary(temporary), location(location), lock(std::move(lock)) {}

StoredFile::StoredFile(fs::path location, bool temporary, std::unique_ptr<std::unique_lock<std::shared_mutex>> lock)
  : size(Size{0_kB}), temporary(temporary), location(location), lock(std::move(lock)) {}

StoredFile::StoredFile(Size size, fs::path location, bool temporary, std::unique_ptr<std::shared_lock<std::shared_mutex>> lock)
  : size(size), temporary(temporary), location(location), lock(std::move(lock)) {}

StoredFile::StoredFile(fs::path location, bool temporary, std::unique_ptr<std::shared_lock<std::shared_mutex>> lock)
  : size(Size{0_kB}), temporary(temporary), location(location), lock(std::move(lock)) {}


//! Helper method for deducing a file size
Size StoredFile::getRealSize() {
  return Size{static_cast<uintmax_t>(fs::file_size(location))};
}

template<typename StreamType>
StreamType StoredFile::getStream() {}

template<>
std::ifstream StoredFile::getStream() {
  std::ifstream stream(this->location, std::ios::in | std::ios::binary);
  if (!stream.is_open() || stream.bad()) return;
  return stream;
}

template<>
std::ofstream StoredFile::getStream() {
  std::get<std::unique_ptr<std::unique_lock<std::shared_mutex>>>(lock);

  return stream(this->location);
}
}
