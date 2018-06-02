#include "storedfile.hpp"
#include "../../utility.hpp"

namespace TinyCDN::Middleware::FileStorage {
StoredFile::StoredFile(Size size, fs::path location, bool temporary)
  : size(size), temporary(temporary), location(location)
{

}
StoredFile::StoredFile(fs::path location, bool temporary)
  : size(Size{0_kB}), temporary(temporary), location(location)
{
}
}
