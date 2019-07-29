#pragma once

#include <mutex>
#include <shared_mutex>
#include <optional>
#include <variant>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "../../utility.hpp"

using TinyCDN::Utility::Size;
namespace TinyCDN::Middleware::FileStorage {

using fileId = uint_fast32_t;
struct StoredFile
{
  Size size;
  bool temporary;
  fs::path location;

  //! StoredFile can be opened in a Read-only or Read-Write mode
  std::variant<std::unique_ptr<std::unique_lock<std::shared_mutex>>, std::unique_ptr<std::shared_lock<std::shared_mutex>>> lock;

  std::optional<fileId> id;
  // std::optional<std::pair<std::size_t, std::size_t>> position;
  Size getRealSize();

  //! Safely returns a stream handle for the StoredFile
  template <typename StreamType>
  StreamType getStream();

  inline StoredFile(const StoredFile& f)
    : size(f.size),
      temporary(f.temporary),
      location(f.location),
      id(f.id)
  {};

  StoredFile(Size size, fs::path location, bool temporary, std::unique_ptr<std::unique_lock<std::shared_mutex>> lock);
  StoredFile(fs::path location, bool temporary, std::unique_ptr<std::unique_lock<std::shared_mutex>> lock);

  StoredFile(Size size, fs::path location, bool temporary, std::unique_ptr<std::shared_lock<std::shared_mutex>> lock);
  StoredFile(fs::path location, bool temporary, std::unique_ptr<std::shared_lock<std::shared_mutex>> lock);
};
}
