#include <optional>

#include "storedfile.hpp"
#include "../../utility.hpp"
#include "../exceptions.hpp"

namespace TinyCDN::Middleware::FileStorage {
StoredFile::StoredFile(Size size, fs::path location, bool temporary)
  : size(size), temporary(temporary), location(location)
{

}
StoredFile::StoredFile(fs::path location, bool temporary)
  : size(Size{0_kB}), temporary(temporary), location(location)
{
}

//! Helper method for deducing a file size
Size StoredFile::getRealSize() {
  try {
    return Size{static_cast<uintmax_t>(fs::file_size(location))};
  }
  catch (fs::filesystem_error& e) {
    throw TinyCDN::Middleware::File::FileStorageException(0, e.what(), std::make_optional<StoredFile>(*this));
  }
}
}
