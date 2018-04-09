#include "storedfile.h"

namespace TinyCDN {
StoredFile::StoredFile(std::string location, Size size, bool temporary)
  : location(location), size(size), temporary(temporary)
{

}
}
