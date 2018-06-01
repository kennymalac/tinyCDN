#include "filesystem.hpp"
#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {

fileId FilesystemStorage::getUniqueFileId()
{
  auto const id = ++fileUniqueId;
  // Persist incremented id to META
  META.seekp(0);
  META << fileUniqueId;
  return id;
}

void FilesystemStorage::allocate()
{
  fileUniqueId = 0;
  std::ofstream META(this->location / "META");
  META << fileUniqueId;
}

void FilesystemStorage::destroy()
{
   fs::remove_all(this->location);
}

std::unique_ptr<StoredFile> FilesystemStorage::lookup(fileId id)
{

}

std::unique_ptr<StoredFile> FilesystemStorage::add(std::unique_ptr<StoredFile> file)
{
  auto const assignedId = getUniqueFileId();
  file->id = assignedId;

  auto assignedLocation = this->location / std::to_string(assignedId);
  fs::copy(file->location, assignedLocation);
  fs::remove(file->location);

  file->location = assignedLocation;
  file->temporary = false;

  return file;
}

void FilesystemStorage::remove(std::unique_ptr<StoredFile> file)
{

}

FilesystemStorage::~FilesystemStorage() {

}

FilesystemStorage::FilesystemStorage(Size allocatedSize, fs::path location, bool preallocated)
  : FileStorage(allocatedSize, location, preallocated) {
  if (!preallocated) {
    allocate();
  }
}

}
