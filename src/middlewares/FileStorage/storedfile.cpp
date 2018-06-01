#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {
StoredFile::StoredFile(Size size, fs::path location, bool temporary)
  : size(size), temporary(temporary), location(location)
{

}
}
