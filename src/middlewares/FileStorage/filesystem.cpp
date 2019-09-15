#include "filesystem.hpp"
#include "storedfile.hpp"
#include <cinttypes>

namespace TinyCDN::Middleware::FileStorage {

const fs::path FilesystemStorage::linkDirName = fs::path{"links/"};
const int FilesystemStorage::storeFileThreshold = 1000;

fileId FilesystemStorage::getUniqueFileId()
{
  auto const id = ++fileUniqueId;

  // Persist incremented id to META
  persist();
  return id;
}

fileId FilesystemStorage::getUniqueStoreId()
{
  std::cout << "getUniqueStoreId " << storeUniqueId << std::endl;
  auto const id = ++storeUniqueId;

  // Create the store's directory
  fs::create_directory(this->location / "store" / std::to_string(id));

  // Persist incremented id to META
  persist();
  return id;
}

void FilesystemStorage::persist() {
  META.seekp(0);
  META << storeUniqueId << ";" << fileUniqueId << ';' << getAllocatedSize();
  META.flush();
}

void FilesystemStorage::allocate()
{
  fileUniqueId = 0;
  storeUniqueId = 1;

  fs::create_directory(this->location / "links");
  fs::create_directory(this->location / "store");
  fs::create_directory(this->location / "store" / std::to_string(1));

  persist();
}

void FilesystemStorage::destroy()
{
   fs::remove_all(this->location);
}

// cdn-website.com/<bucket_id>/<file_id>/file.jpg
std::unique_ptr<StoredFile> FilesystemStorage::lookup(fileId id)
{
  // auto search = fileMutexes.find(id);
  auto lock = std::make_unique<std::shared_lock<std::shared_mutex>>(fileMutexes[id]);

  auto stFile = std::make_unique<StoredFile>(
    fs::read_symlink(this->location / this->linkDirName / std::to_string(id)), false, std::move(lock));

  stFile->id = id;
  return stFile;
}

std::unique_ptr<StoredFile> FilesystemStorage::add(std::unique_ptr<StoredFile> file)
{
  std::unique_lock<std::mutex> storageLock(mutex);
  allocatedSize = std::make_unique<Size>(getAllocatedSize() + file->size);

  auto const assignedId = getUniqueFileId();

  file->id = assignedId;

  auto assignedStoreId = storeUniqueId.load();

  std::unique_lock<std::shared_mutex> storeLock(storeMutexes[assignedStoreId]);

  auto assignedLocation = this->location / "store" / fs::path(std::to_string(assignedStoreId)) / file->location.filename();

  std::ios::sync_with_stdio();
  std::cout << "FileSystemStorage::add assignedLocation: " << assignedLocation << std::endl;
  // TODO increment store's amount of files stored
  if (fs::exists(assignedLocation)) {
    // The likelihood that a file in a separate would have the same filename is very low, so just create a new store folder for now
    // TODO reuse existing stores, don't always create new ones in this situation
    assignedStoreId = getUniqueStoreId();

    assignedLocation = this->location / "store" / fs::path(std::to_string(assignedStoreId)) / file->location.filename();
  }

  // No mutations on the storage instance's properties will be made following this
  storageLock.unlock();

  {
    std::ofstream stream(assignedLocation);
    if (!stream.is_open() || stream.bad()) {
      storageLock.lock();
      allocatedSize = std::make_unique<Size>(getAllocatedSize() - file->size);
      storageLock.unlock();
      storeLock.unlock();
      return;
    }
  }
  // Uniqueness of filename in this directory is now guaranteed, so unlock this
  storeLock.unlock();

  // std::cout << "location: " << file->location << "\n";
  // std::cout << "assignedLocation: " << assignedLocation << "\n";

  fs::copy(file->location, assignedLocation, fs::copy_options::overwrite_existing);
  fs::remove(file->location);

  // Create a link that points to this file
  fs::create_symlink(assignedLocation, this->location / this->linkDirName / std::to_string(assignedId));

  file->location = assignedLocation;
  file->temporary = false;

  return file;
}

void FilesystemStorage::remove(std::unique_ptr<StoredFile> file)
{
  if (!file->id.has_value()) return;

  std::unique_lock<std::mutex> storageLock(mutex);

  fs::remove(this->location / this->linkDirName / std::to_string(file->id.value()));

  allocatedSize = file->size != 0
    ? std::make_unique<Size>(getAllocatedSize() - file->size)
    : std::make_unique<Size>(getAllocatedSize() - file->getRealSize());

  fs::remove(file->location);

  persist();
}

FilesystemStorage::~FilesystemStorage() {
}

FilesystemStorage::FilesystemStorage(Size size, fs::path location, bool preallocated)
  : FileStorage(size, location, preallocated) {

  std::ios::sync_with_stdio();

  std::cout << "Creating new FilesystemStorage...\n"
            << "size: " << size
            << " location: " << location
            << " preallocated: " << preallocated << std::endl;

  if (!preallocated) {
    META = std::ofstream(this->location / "META");

    if (!META.is_open() || META.bad()) return;

    allocatedSize = std::make_unique<Size>(0);
    allocate();
  }
  else {
    std::ifstream _meta(this->location / "META");
    if (!_meta.is_open() || _meta.bad()) return;

    std::string idsAndSize((std::istreambuf_iterator<char>(_meta)),
                           std::istreambuf_iterator<char>());
    _meta.close();
    std::cout << "idsAndSize " << idsAndSize << std::endl;

    auto delim = idsAndSize.find(";");
    storeUniqueId = std::stoul(idsAndSize.substr(0, delim));
    std::cout << "storeUniqueId: " << storeUniqueId << std::endl;
    auto nextDelim = idsAndSize.find(";", delim+1);

    fileUniqueId = std::stoul(idsAndSize.substr(delim+1, nextDelim));
    std::cout << "fileUniqueId: " << fileUniqueId << std::endl;
    delim = nextDelim;

    char* nptr;
    auto justSize = idsAndSize.substr(delim+1);

    allocatedSize = std::make_unique<Size>(std::strtoumax(justSize.c_str(), &nptr, 10));

    META = std::ofstream(this->location / "META");
    persist();
  }

}

}
