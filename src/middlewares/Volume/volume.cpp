namespace TinyCDN::Middleware::Volume {

Volume::Volume(VolumeId id, TinyCDN::Utility::Size size)
{

}

template<typename T>
std::unique_ptr<StorageVolume<TinyCDN::Middleware::Volume::StorageVolumeManager::T> > StorageVolumeManager::createStorageVolume()
{

}

void StorageVolumeManager::removeVolume(VolumeId id)
{

}

std::optional<std::vector<VolumeId> > TinyCDN::Middleware::Volume::VirtualVolume::getVolumeIds()
{

}

void TinyCDN::Middleware::Volume::VirtualVolume::setSize(uintmax_t size)
{

}

}
