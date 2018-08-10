#include "exceptions.hpp"
#include "file.hpp"

#include "FileStorage/storedfile.hpp"

namespace TinyCDN::Middleware::File {


FileStorageException::FileStorageException(int code, std::string explanation, const Storage::StoredFile storedFile)
  : std::runtime_error(explanation), code(code), storedFile(std::make_optional<Storage::StoredFile>(storedFile))
{}


FileStorageException::FileStorageException(int code, std::string explanation, const std::optional<Storage::StoredFile> storedFile)
  : std::runtime_error(explanation), code(code), storedFile(storedFile)
{}

FileStorageException::FileStorageException(int code, std::string explanation)
  : std::runtime_error(explanation), code(code), storedFile(std::optional<Storage::StoredFile>{})
{}

FileBucketException::FileBucketException(const FileBucket &fb, int code, std::string explanation)
  : std::runtime_error(explanation), code(code), fileBucket(fb)
{}

FileBucketRegistryItemException::FileBucketRegistryItemException(const FileBucketRegistryItem &fbri, int code, std::string explanation)
  : std::runtime_error(explanation), code(code), item(fbri)
{}


FileBucketRegistryException::FileBucketRegistryException(const FileBucketRegistry &fbr, int code, std::string explanation)
  : std::runtime_error(explanation), code(code), registry(fbr)
{}
}
