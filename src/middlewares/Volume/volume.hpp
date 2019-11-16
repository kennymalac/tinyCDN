#pragma once

#include <vector>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <optional>

#include "../exceptions.hpp"
#include "../../utility.hpp"
#include "../../hashing.hpp"
#include "../FileStorage/filesystem.hpp"

namespace TinyCDN::Middleware::Volume {

using namespace TinyCDN::Utility::Exceptions;
using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::File;

using TinyCDN::Utility::Size;

using FileBucketId = Id<64>;
using VolumeId = Id<64>;

/*!
 * \brief Abstract, statically-sized, network-identified blob
*/
class Volume {
public:
  // TODO remove this? StorageVolumeManager needs this, not Volume itself
  const VolumeId id;
  //! The maximum allowed size
  Size size;

  // virtual void resize(t size) = 0;
  virtual void destroy() = 0;

  // bool hasBucketId(FileBucketId id);

  inline Volume(VolumeId id, Size size) : id(id), size(size) {};
};

/*!
 * \brief Volume capable of storing files.
 * FileBuckets are assigned volumes based on location, config, etc.
 A StorageVolume will be assigned many different buckets from a VirtualVolume, but if a bucket does not populate, it can "spill over" into another StorageVolume if necessary.
 At some point, every now and then the volume can reallocate space by reclaiming lost volume space. (?)
 A StorageVolume is not be tied to the storage requirements of its buckets.
 StorageVolumes are replicated at a lower level than buckets. Buckets with default replication settings will be replicated as part of a volume manager's replication strategy.
*/
template <typename storageType>
class StorageVolume : public Volume {
public:
  inline void resize() {
    throw new NotImplementedException();
  }

  inline void destroy() {
    this->storage->destroy();
  }

  //! A file storage driver that provides methods to retrieve, modify, and delete files
  std::unique_ptr<storageType> storage;

  inline StorageVolume(VolumeId id, Size size, fs::path location, bool preallocated)
    : Volume(id, size) {
    storage = std::make_unique<storageType>(size, location, preallocated);

  }
  // TODO delete default, volume constructors
};

//! All StorageVolume types (monostate might be required here b/c storagevolume not default constructible)
using AnyStorageVolume = std::variant<std::unique_ptr<StorageVolume<FileStorage::FilesystemStorage>>>;
// Could be any StorageVolume instance, or a non-existent value
// Could use std::optional, but wrapping variant would make std::visit less usable
using MaybeAnyStorageVolume = std::variant<std::monostate, std::unique_ptr<StorageVolume<FileStorage::FilesystemStorage>>>;

//class BackupVolume : Volume;

struct VirtualVolume;

/*!
 * \brief Responsible for allocating, replicating, and removing StorageVolume instances as necessary.
 */
class StorageVolumeManager {
public:
  inline uintmax_t getSize() {
    return size;
  };
  //! Checks if size is going DOWN, if so, TODO will replicate elsewhere?
  inline void setSize(uintmax_t size) {
    size = size;
  };
  uintmax_t defaultVolumeSize;

  std::optional<std::tuple<std::unique_lock<std::shared_mutex>, MaybeAnyStorageVolume>> getStorageVolume(VolumeId id);
  template <typename T>
  std::unique_ptr<StorageVolume<T>> createStorageVolume(fs::path location);
  template <typename T>
  std::unique_ptr<StorageVolume<T>> loadStorageVolume(VolumeId id, Size size, fs::path location);
  // replicateVolume
  //! Removes a StorageVolume from the StorageVolumeManager's records
  inline void removeStorageVolume(VolumeId id) {
    volumes.erase(id);
    volumeMutexes.erase(id);
  };

  std::unordered_map<VolumeId, MaybeAnyStorageVolume, IdHasher> volumes;

  inline StorageVolumeManager(uintmax_t size, uintmax_t defaultVolumeSize) : size(size), defaultVolumeSize(defaultVolumeSize) {}

private:
  IdFactory idFactory;
  uintmax_t size;

  std::unordered_map<VolumeId, std::shared_mutex, IdHasher> volumeMutexes;
};

/*!
 * \brief Volume that can be of a very large indeterminate size and manages many volumes within it.
 * The simple difference between a VirtualVolume and a StorageVolume is that a virtual volume is an alias to a single Volume that manages many volumes internally to efficently store large amounts of files.
 A FileBucket references a single virtual volume instead of the set of StorageVolumes that store its files.
*/
class VirtualVolume : public Volume {
public:
  fs::path location;
  //! Retrieves the list of VolumeIds associated with
  std::vector<VolumeId> getFileBucketVolumeIds(FileBucketId id);
  inline uintmax_t getSize() {
    return size;
  };
  //! Also modifies storage volume manager's size
  inline void setSize(uintmax_t size) {
    // Persist to config? IN which situation will this ever happen?
    storageVolumeManager.setSize(size);
    size = size;
  };
  StorageVolumeManager storageVolumeManager;

  /*
   * \brief Increments size by newly added volume size, adds to volDb.
   * In the future, committing different storage volume types may have different side-effects.
   * Care has to be taken for adding new volumes, because the lookup of a file id means each StorageVolume will have to be queried. Filebuckets are only aware of what file ids are assigned, not which volume they are stored in.
   * Future optimization: we could add the file id to a hash table of the file id mapped to its volumeid, so that the lookup is much faster
   * The hash table would expire file id entries that are older than a certain threshold, i.e. not accessed within a certain time period
  */
  template <typename T>
  uintmax_t commitStorageVolume(std::unique_ptr<StorageVolume<T>> volume);

  //! Adds volume to fbVolDb and asynchronously persists the mapping to the disk.
  void addFileBucketVolume(FileBucketId fbId, VolumeId volId);

  // void loadConfig();
  // TODO: how to split the db file up?
  //! Loads the FileBucket Volume Database file into memory
  void loadDb(std::ifstream persistedDb);

  //! Destroys the virtual volume. NOTE: will not destroy backup volumes or replicated volumes(?)
  void destroy();

  VirtualVolume(VolumeId id, uintmax_t size, uintmax_t defaultVolumeSize, fs::path location, fs::path fbVolDbLocation);

private:
  std::ofstream fbVolDbFile;
  // std::ofstream configFile;

  uintmax_t size;
  //! A mapping of FileBucketIds to a list of volumes containing their contents
  std::unordered_map<FileBucketId, std::vector<VolumeId>, IdHasher> fbVolDb;
};
}
