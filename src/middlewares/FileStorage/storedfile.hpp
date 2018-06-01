#pragma once

#include <optional>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "../../utility.hpp"

namespace TinyCDN::Middleware::FileStorage {

using fileId = uint_fast32_t;
struct StoredFile
{
  Size size;
  bool temporary;
  fs::path location;

  std::optional<fileId> id;
  // std::optional<std::pair<std::size_t, std::size_t>> position;

  StoredFile(Size size, fs::path location, bool temporary);
};

}
