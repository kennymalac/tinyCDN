#pragma once

#include <memory>
#include <string>

#include "../../utility.hpp"
#include "../Volume/marshaller.hpp"
#include "storagecluster.hpp"

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility;
using namespace TinyCDN::Middleware::Volume;

struct StorageClusterNode;
struct StorageClusterNodeSingleton;

struct StorageClusterParams {
  UUID4 id;
  std::string name;
  fs::path location;
  bool existing;
  VirtualVolumeParams virtualVolumeParams;

  inline StorageClusterParams(UUID4 id, std::string name, fs::path location, bool existing, VirtualVolumeParams volParams)
    : id(id), name(name), location(location), existing(existing), virtualVolumeParams(volParams)
    {};
};

struct StorageClusterNodeJsonMarshaller {
  inline StorageClusterParams deserialize(std::string input) {
    // TODO make this actually work as intended
    auto location = fs::current_path();

    // vVolId = 
    // fbVolDbLocation = vParams.location / "volumes.db";
    // defaultVolumeSize = 1_gB;
    // volumeLimit = 4;
    VirtualVolumeParams vParams{VolumeId{std::string{""}}, location / "volume", location / "volume" / "volumes.db", 1_gB, 4};
    StorageClusterParams params{UUID4{std::string{""}}, "master-test", location, false, vParams};

    return params;
  };
  inline StorageClusterNodeSingleton getSingleton(StorageClusterParams params) {
    StorageClusterNodeSingleton singleton;

    // Initialize the virtual volume marshaller to convert the params into a VirtualVolume instance
    VirtualVolumeMarshaller virtualVolumeMarshaller;
    singleton.initInstance(singleton, params.id, params.name, params.location, params.existing, virtualVolumeMarshaller.getInstance(params.virtualVolumeParams));
    return singleton;
  }
//  VirtualVolumeParams volParams;
};
}
