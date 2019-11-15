#pragma once

#include <memory>
#include <string>

#include "volume.hpp"

namespace TinyCDN::Middleware::Volume {

//! A Volume CSV gets converted into this POD and subsequently this data is assigned to a Volume instance
struct VolumeParams {
  VolumeId id;
  uintmax_t size;

  VolumeParams() : size(0) {}
  VolumeParams(VolumeId id, uintmax_t size) : id(id), size(size) {}
};

struct VirtualVolumeParams : VolumeParams {
  fs::path location;
  //! Location of the FileBucket Volume DB file
  fs::path fbVolDbLocation;
  //! The default size of the created StorageVolumes
  uintmax_t defaultVolumeSize;
  //! The limit to the number of StorageVolumes to be created
  int volumeLimit;

  VirtualVolumeParams(VolumeId id, fs::path location, fs::path fbVolDbLocation, uintmax_t defaultVolumeSize, int volumeLimit) : VolumeParams(id, volumeLimit * defaultVolumeSize), fbVolDbLocation(fbVolDbLocation), volumeLimit(volumeLimit) {};
};


// id=AFTAWAFT#2233;location=/blahblah/blah;filebuckets=ID1,ID2,ID3,ID4,ID5
//! Converts volumes from a CSV into a POD into an object instance
struct VolumeCSVMarshaller {
  std::unique_ptr<VolumeParams> params;

  //! Takes a Volume field and assigns it to its deduced conversion value
  auto deserializeField(std::string field, std::string value);

  //! Creates a Volume instance by taking params and creating a Volume instance from it
  template <typename T>
  std::unique_ptr<T> deserialize();

  //! Convenience helper method for generating fieldName=
  inline std::string assignmentToken(std::string fieldName) {
    return fieldName + "=";
  }

  //! Resets the converter by emptying the current VolumeParams
  inline void reset() {
    params = std::make_unique<VolumeParams>();
  }

  inline VolumeCSVMarshaller() {
    params = std::make_unique<VolumeParams>();
  }
};

// JSON because this may be read by other programs and will not grow very large
struct VirtualVolumeMarshaller {
  inline std::unique_ptr<VirtualVolume> getInstance(VirtualVolumeParams params) {
    return std::make_unique<VirtualVolume>(params.id, params.size, params.defaultVolumeSize, params.location, params.fbVolDbLocation);
  }
};
}
