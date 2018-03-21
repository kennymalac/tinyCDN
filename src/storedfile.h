#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "src/utility.cpp"


struct StoredFile
{
  Size size;
  bool temporary;
  fs::path location;

  StoredFile(std::string location, Size size, bool temporary);
};
