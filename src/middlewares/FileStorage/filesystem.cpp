#include "filesystem.hpp"
#include "storedfile.hpp"
#include <cinttypes>

namespace TinyCDN::Middleware::FileStorage {

const fs::path FilesystemStorage::linkDirName = fs::path{"links/"};
const int FilesystemStorage::storeFileThreshold = 1000;

FileId FilesystemStorage::getUniqueFileId() {
  return idFactory.generate<64>();
}

StoreId FilesystemStorage::getUniqueStoreId()
{
  auto id = idFactory.generate<32>();

  std::cout << "getUniqueStoreId " << id << std::endl;

  // Create the store's directory
  fs::create_directory(this->location / "store" / id.str());

  // Persist incremented id to META
  // persist();
  return id;
}

void FilesystemStorage::persist() {
  META.seekp(0);
  META << currentStoreId;
  META << ";";
  META << getAllocatedSize();
  META.flush();
}

void FilesystemStorage::allocate()
{
  fs::create_directory(this->location / "links");
  fs::create_directory(this->location / "store");
  currentStoreId = getUniqueStoreId();

  persist();
}

void FilesystemStorage::destroy()
{
   fs::remove_all(this->location);
}

// cdn-website.com/<bucket_id>/<file_id>/file.jpg
std::unique_ptr<StoredFile> FilesystemStorage::lookup(FileId id)
{
  // auto search = fileMutexes.find(id);
  auto lock = std::make_unique<std::shared_lock<std::shared_mutex>>(fileMutexes[id]);

  try {
    auto stFile = std::make_unique<StoredFile>(
      fs::read_symlink(this->location / this->linkDirName / id.str()), false, std::move(lock));

    stFile->id = id;
    return stFile;
  }
  catch (fs::filesystem_error& e) {
    throw File::FileStorageException(0, e.what());
  }
}

std::unique_ptr<StoredFile> FilesystemStorage::add(std::unique_ptr<StoredFile> file)
{
  std::unique_lock<std::mutex> storageLock(mutex);
  // Increment the allocatedSize
  allocatedSize = std::make_unique<Size>(getAllocatedSize() + file->size);
  storageLock.unlock();

  auto const assignedId = getUniqueFileId();
  file->id = assignedId;

  // TODO Have a list of the store ids that are the most empty and populate those
  auto assignedStoreId = currentStoreId;

  // TODO - do we need to EVER lock the store? perhaps for incrementing file count?
  // std::unique_lock<std::shared_mutex> storeLock(storeMutexes[assignedStoreId]);

  auto stem = file->location.stem();
  auto ext = file->location.extension();
  std::string assignedFileName{""};
  // TODO: Truncate stem?
  assignedFileName.append(stem);
  assignedFileName.append("_");
  assignedFileName.append(file->id->str());
  assignedFileName.append(ext);

  auto assignedLocation = this->location / "store" / fs::path(assignedStoreId.str()) / fs::path(assignedFileName);

  std::ios::sync_with_stdio();
  std::cout << "FileSystemStorage::add assignedLocation: " << assignedLocation << std::endl;

  // TODO increment store's amount of files stored
  // Chance of hash collision is miniscule
  // if (fs::exists(assignedLocation)) {

  // No mutations on the storage instance's properties will be made following this

  {
    std::ofstream stream(assignedLocation);
    if (!stream.is_open() || stream.bad()) {
      storageLock.lock();
      allocatedSize = std::make_unique<Size>(getAllocatedSize() - file->size);
      storageLock.unlock();
      throw File::FileStorageException(0, "New file could not be created for StoredFile in specified location", *file);
    }
  }
  // std::cout << "location: " << file->location << "\n";
  // std::cout << "assignedLocation: " << assignedLocation << "\n";

  fs::copy(file->location, assignedLocation, fs::copy_options::overwrite_existing);
  fs::remove(file->location);

  // Create a symlink that points to this file
  fs::create_symlink(assignedLocation, this->location / this->linkDirName / assignedId.str());

  file->location = assignedLocation;
  file->temporary = false;

  return file;
}

void FilesystemStorage::remove(std::unique_ptr<StoredFile> file)
{
  if (!file->id.has_value()) {
    throw File::FileStorageException(0, "StoredFile file has no id", std::make_optional<StoredFile>(*file));
  }

  std::unique_lock<std::mutex> storageLock(mutex);

  fs::remove(this->location / this->linkDirName / file->id.value().str());

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

    if (!META.is_open() || META.bad()) {
      throw File::FileStorageException(3, "META file cannot be created");
    }

    allocatedSize = std::make_unique<Size>(0);
    allocate();
  }
  else {
    std::ifstream _meta(this->location / "META");
    if (!_meta.is_open() || _meta.bad()) {
      throw File::FileStorageException(2, "META file does not exist or is not available");
    }

    std::string metaContents((std::istreambuf_iterator<char>(_meta)),
			   std::istreambuf_iterator<char>());
    _meta.close();
    std::cout << "FileSystemStorage META contents: " << metaContents << std::endl;

    try {
      auto delim = metaContents.find(";");
      currentStoreId = metaContents.substr(0, delim);
      std::cout << "currentStoreId: " << currentStoreId << std::endl;
      // auto nextDelim = idsAndSize.find(";", delim+1);

      // fileUniqueId = std::stoul(idsAndSize.substr(delim+1, nextDelim));
      // std::cout << "fileUniqueId: " << fileUniqueId << std::endl;
      // delim = nextDelim;

      char* nptr;
      auto justSize = metaContents.substr(delim+1); // delim+1

      allocatedSize = std::make_unique<Size>(std::strtoumax(justSize.c_str(), &nptr, 10));
    }
    catch (std::invalid_argument e) {
      throw File::FileStorageException(-1, std::string{"ERROR converting META file: "}.append(e.what()));
    }

    META = std::ofstream(this->location / "META");
    persist();

    if (!META.is_open() || META.bad()) {
      throw File::FileStorageException(-1, "META file cannot be opened or is corrupted");
    }
  }

}

}
