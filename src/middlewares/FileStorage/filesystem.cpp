#include "filesystem.hpp"
#include "storedfile.hpp"
#include <cinttypes>

namespace TinyCDN::Middleware::FileStorage {

const fs::path FilesystemStorage::linkDirName = fs::path{"links/"};

fileId FilesystemStorage::getUniqueFileId()
{
  auto const id = ++fileUniqueId;
  // Persist incremented id to META
  persist();
  return id;
}

void FilesystemStorage::persist() {
  META.seekp(0);
  META << fileUniqueId << ';' << allocatedSize->size;
}

void FilesystemStorage::allocate()
{
  fileUniqueId = 0;
  META.seekp(0);
  META << fileUniqueId << ';' << allocatedSize->size;

  fs::create_directory(this->location / "links");
}

void FilesystemStorage::destroy()
{
   fs::remove_all(this->location);
}

// cdn-website.com/<bucket_id>/<file_id>/file.jpg
std::unique_ptr<StoredFile> FilesystemStorage::lookup(fileId id)
{
  try {
    auto stFile = std::make_unique<StoredFile>(
          fs::read_symlink(this->location / this->linkDirName / std::to_string(id)), false);

    stFile->id = id;
    return stFile;
  }
  catch (fs::filesystem_error& e) {
    throw File::FileStorageException(0, e.what(), std::optional<StoredFile>{});
  }
}

std::unique_ptr<StoredFile> FilesystemStorage::add(std::unique_ptr<StoredFile> file)
{
  allocatedSize = std::make_unique<Size>(allocatedSize->size + file->size.size);
  auto const assignedId = getUniqueFileId();
  file->id = assignedId;

  auto assignedLocation = this->location / file->location.filename();
  // std::cout << "location: " << file->location << "\n";
  // std::cout << "assignedLocation: " << assignedLocation << "\n";

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
  if (!file->id.has_value()) {
    throw File::FileStorageException(0, "StoredFile file has no id", std::make_optional<StoredFile>(*file));
  }

  fs::remove(this->location / this->linkDirName / std::to_string(file->id.value()));

  allocatedSize = file->size.size != 0
      ? std::make_unique<Size>(allocatedSize->size - file->size.size)
      : std::make_unique<Size>(allocatedSize->size - assignStoredFileSize(*file).size);

  fs::remove(file->location);

  persist();
}

FilesystemStorage::~FilesystemStorage() {

}

FilesystemStorage::FilesystemStorage(Size size, fs::path location, bool preallocated)
  : FileStorage(size, location, preallocated) {

  if (!preallocated) {
    META = std::ofstream(this->location / "META");
    allocatedSize = std::make_unique<Size>(0);
    allocate();
  }
  else {
    std::ifstream _meta(this->location / "META");
    std::string idAndSize((std::istreambuf_iterator<char>(_meta)),
                           std::istreambuf_iterator<char>());
    _meta.close();

    // TODO check is_open etc.

    auto const delim = idAndSize.find(";");
    // std::cout  << "idAndSize: " << idAndSize << "\n";
    // std::cout << "id: " << idAndSize.substr(0, delim) << "\n";
    fileUniqueId = std::stoul(idAndSize.substr(0, delim));

    char* nptr;
    auto justSize = idAndSize.substr(delim+1);

    allocatedSize = std::make_unique<Size>(std::strtoumax(justSize.c_str(), &nptr, 10));

    META = std::ofstream(this->location / "META");
  }

}

}
