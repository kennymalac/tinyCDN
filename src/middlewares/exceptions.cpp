#include "exceptions.hpp"
#include "file.hpp"

#include "FileStorage/storedfile.hpp"

namespace TinyCDN::Middleware::File {

FileStorageException::FileStorageException(int code, std::string explanation, const std::optional<Storage::StoredFile> storedFile)
  : std::runtime_error(explanation), code(code), storedFile(storedFile)
{}

FileBucketException::FileBucketException(const FileBucket &fb, int code, std::string explanation)
  : std::runtime_error(explanation), code(code), fileBucket(fb)
{}

FileBucketRegistryException::FileBucketRegistryException(const FileBucketRegistry &fbr, int code, std::string explanation)
  : std::runtime_error(explanation), code(code), registry(fbr)
{}
}
