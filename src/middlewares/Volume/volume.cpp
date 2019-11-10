#include "volume.hpp"

namespace TinyCDN::Middleware::Volume {

using storageVolumeResult = std::tuple<std::unique_lock<std::shared_mutex>, MaybeAnyStorageVolume>;

std::optional<storageVolumeResult> StorageVolumeManager::getStorageVolume(VolumeId id) {
  auto mtxFind = volumeMutexes.find(id);

  if (mtxFind == volumeMutexes.end()) {
    return std::make_optional<storageVolumeResult>();
  }

  std::unique_lock<std::shared_mutex> lock(mtxFind->second);

  auto volFind = volumes.find(id);
  if (volFind == volumes.end()) {
    // Should never happen if mutex is found, do some logging...
    return std::make_optional<storageVolumeResult>();
  }

  auto vol = std::move(volFind->second);
  // Since we have the mutex already at this point, the value should be available
  if (auto noVal = std::get_if<std::monostate>(&vol)) {
    // Should never happen...
    return std::make_optional<storageVolumeResult>();
  }

  // WARNING - callee is responsible for moving back volume to manager again
  // Any better way to do this? Perhaps we do a "visit" on the volume rather than a std::move?
  return std::optional<storageVolumeResult>{std::make_tuple(std::move(lock), std::move(vol))};
}

template<typename T>
std::unique_ptr<StorageVolume<T>> StorageVolumeManager::createStorageVolume(fs::path location) {
  return std::make_unique<StorageVolume<T>>(idFactory.generate<64>(), defaultVolumeSize, location, false);
}

template<typename T>
std::unique_ptr<StorageVolume<T>> StorageVolumeManager::loadStorageVolume(VolumeId id, Size size, fs::path location) {
  return std::make_unique<StorageVolume<T>>(id, size, location, true);
}

std::vector<VolumeId> VirtualVolume::getFileBucketVolumeIds(FileBucketId id) {
  auto volIdsFind = fbVolDb.find(id);

  if (volIdsFind == fbVolDb.end()) {
    return std::vector<VolumeId>{};
  }
  return volIdsFind->second;
}

template<typename T>
uintmax_t VirtualVolume::commitStorageVolume(std::unique_ptr<StorageVolume<T>> volume) {
  storageVolumeManager.volumes[volume.id] = volume;
  auto size = getSize() + volume.size;
  setSize(size);
  // TODO persist()

  return size;
}

void VirtualVolume::addFileBucketVolume(FileBucketId fbId, VolumeId volId) {
  /*  TYPICAL Operation
  store 2gig file in volume 1
  volume 1 -> no space!
  ok, add another volume with space to bucket
  */

  // NOTE - this could be problematic not to have a mutex, because many concurrent process might try to add a volume all at once. What would be better is if there was a need for more space, all requests for adding a new volume would halt, and wait for the new volume to be assigned. Once it is assigned, we can return the volume to each waiting process

  // if alreadyAddingFileBucketVolume(fbId)
  // wait for that to finish, return that new vol id
  auto volIdsFind = fbVolDb.find(id);

  std::vector<VolumeId> vec;

  if (volIdsFind != fbVolDb.end()) {
    vec = volIdsFind->second;
  }
  vec.push_back(volId);

  fbVolDb[fbId] = vec;
  // TODO persist()
}

void VirtualVolume::loadDb(std::ifstream persistedDb) {
  // TODO load file - parse into fbVolDbMap. Look at FileBucketRegistry
}

void VirtualVolume::destroy() {
  for (auto& kv : storageVolumeManager.volumes) {
    // TODO move below to getStorageVolume

    // if (auto volume = std::get_if<StorageVolume<FileStorage::FilesystemStorage>>(r.second)) {
    //	volume->destroy();
    // }
    // else {
    //	// This should never EVER happen! monostate is ruled out when mutex is acquired
    // }

    // // TODO improve complexity b/c removeStorageVolume will delete by key not by index
    // this->storageVolumeManager.removeStorageVolume(kv.first);
  }
  setSize(0);
}

}
