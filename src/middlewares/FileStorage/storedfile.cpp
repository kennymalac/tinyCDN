#include <optional>

#include "storedfile.hpp"
#include "../../utility.hpp"
#include "../exceptions.hpp"

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
  try {
    return Size{static_cast<uintmax_t>(fs::file_size(location))};
  }
  catch (fs::filesystem_error& e) {
    throw TinyCDN::Middleware::File::FileStorageException(0, e.what(), std::make_optional<StoredFile>(*this));
  }
}

template<typename StreamType>
StreamType StoredFile::getStream() {}

template<>
std::ifstream StoredFile::getStream() {
  std::ifstream stream(this->location, std::ios::in | std::ios::binary);
  if (!stream.is_open() || stream.bad()) {
    std::string explanation{"Reading input stream failed in getStream()"};
    throw File::FileStorageException(0, explanation, *this);
  }
  return stream;
}

template<>
std::ofstream StoredFile::getStream() {
  try {
    std::get<std::unique_ptr<std::unique_lock<std::shared_mutex>>>(lock);
  }
  catch (const std::bad_variant_access&) {
    std::string explanation{"This StoredFile was opened in a Read-only mode and cannot be written to"};
    throw File::FileStorageException(4, explanation, *this);
  }

  std::ofstream stream(this->location);

  if (!stream.is_open() || stream.bad()) {
    std::string explanation{"Reading output stream failed in getStream()"};
    throw File::FileStorageException(0, explanation, *this);
  }

  return stream;
}
}
