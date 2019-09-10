#pragma once

#include <memory>
#include <string>

#include "volume.hpp"

namespace TinyCDN::Middleware::Volume {

//! A Volume CSV gets converted into this POD and subsequently this data is assigned to a Volume instance
struct VolumeParams {
  VolumeId id;
  uintmax_t size;

  VolumeParams() : size(0) {};
  VolumeParams(VolumeId id, uintmax_t size) : id(id), size(size) {};
};

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
struct VirtualVolumeJsonMarshaller  {
  // TODO decide what JSON library to use
};
}
