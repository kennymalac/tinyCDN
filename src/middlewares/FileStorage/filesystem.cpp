#include "filesystem.hpp"
#include "storedfile.hpp"
#include <cinttypes>

namespace TinyCDN::Middleware::FileStorage {

const fs::path FilesystemStorage::linkDirName = fs::path{"links/"};

fileId FilesystemStorage::getUniqueFileId()
{
  auto const id = ++fileUniqueId;
  // Persist incremented id to META
  META.seekp(0);
  META << fileUniqueId << ";" << allocatedSize->size;
  return id;
}

void FilesystemStorage::allocate()
{
  fileUniqueId = 0;
  META << fileUniqueId << ";" << allocatedSize->size;

  fs::create_directory(this->location / "links");
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

FilesystemStorage::FilesystemStorage(Size size, fs::path location, bool preallocated)
  : FileStorage(size, location, preallocated) {

  if (!preallocated) {
    META = std::ofstream(this->location / "META");
    allocate();
  }
  else {
    std::ifstream _meta(this->location / "META");
    std::string idAndSize((std::istreambuf_iterator<char>(_meta)),
                           std::istreambuf_iterator<char>());
    // TODO fileUniqueId = META

    auto const delim = idAndSize.find(";");
    fileUniqueId = std::stoul(idAndSize.substr(0, delim));

    char* nptr;
    allocatedSize = std::make_unique<Size>(std::strtoumax(idAndSize.substr(delim+1).c_str(), &nptr, 10));

    META = std::ofstream(this->location / "META");
  }

}

}
