#include "filesystem.hpp"
#include "storedfile.hpp"

namespace TinyCDN::Middleware::FileStorage {

const fs::path FilesystemStorage::linkDirName = fs::path{"links/"};

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

// cdn-website.com/<bucket_id>/<file_id>/file.jpg
std::unique_ptr<StoredFile> FilesystemStorage::lookup(fileId id)
{
  return std::make_unique<StoredFile>(
      fs::read_symlink(this->linkDirName / std::to_string(id)), false);
}

std::unique_ptr<StoredFile> FilesystemStorage::add(std::unique_ptr<StoredFile> file)
{
  auto const assignedId = getUniqueFileId();
  file->id = assignedId;

  auto assignedLocation = this->location / file->location.filename();
  std::cout << "location: " << file->location << "\n";
  std::cout << "assignedLocation: " << assignedLocation << "\n";

  fs::copy(file->location, assignedLocation);
  fs::remove(file->location);

  // Create a link that points to this file
  fs::create_symlink(assignedLocation, this->location / this->linkDirName / std::to_string(assignedId));

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
  fs::create_directory(this->location / "links");
}

}
