#include <aslam/cameras/camera.h>
#include <aslam/cameras/ncamera.h>
#include <aslam/cameras/yaml/camera-yaml-serialization.h>
#include <aslam/cameras/yaml/ncamera-yaml-serialization.h>
#include <aslam/common/memory.h>
#include <aslam/common/yaml-serialization.h>

namespace YAML {

bool convert<std::shared_ptr<aslam::NCamera> >::decode(const Node& node,
                                                       aslam::NCamera::Ptr& ncamera) {
  ncamera.reset();
  try {
    if (!node.IsMap()) {
      LOG(ERROR) << "Unable to parse the ncamera because the node is not a map.";
      return true;
    }

    // Parse the label.
    std::string label = "";
    if (!YAML::safeGet<std::string>(node, "label", &label)) {
      LOG(ERROR) << "Unable to get the label for the ncamera.";
      return true;
    }

    // Parse the id.
    aslam::NCameraId ncam_id;
    std::string id_string;
    if (!node["id"] || !YAML::safeGet<std::string>(node, "id", &id_string)) {
      LOG(WARNING) << "Unable to get the id for the ncamera. Generating new random id.";
      ncam_id.randomize();
    } else {
      ncam_id.fromHexString(id_string);
    }

    // Parse the cameras.
    const Node& cameras_node = node["cameras"];
    if (!cameras_node.IsSequence()) {
      LOG(ERROR) << "Unable to parse the cameras because the camera node is not a sequence.";
      return true;
    }

    size_t num_cameras = cameras_node.size();
    if (num_cameras == 0) {
      LOG(ERROR) << "Number of cameras is 0.";
      return true;
    }

    aslam::TransformationVector T_Ci_B;
    std::vector<aslam::Camera::Ptr> cameras;
    for (size_t camera_index = 0; camera_index < num_cameras; ++camera_index) {
      // Decode the camera
      const Node& camera_node = cameras_node[camera_index];
      if (!camera_node) {
        LOG(ERROR) << "Unable to get camera node for camera " << camera_index;
        return true;
      }
      if (!camera_node.IsMap()) {
        LOG(ERROR) << "Camera node for camera " << camera_index << " is not a map.";
        return true;
      }

      aslam::Camera::Ptr camera;
      if (!YAML::safeGet(camera_node, "camera", &camera)) {
        LOG(ERROR) << "Unable to retrieve camera " << camera_index;
        return true;
      }

      const Node& extrinsics_node = camera_node["extrinsics"];
      if (!extrinsics_node) {
        LOG(ERROR) << "No extrinsics node for camera " << camera_index;
        return true;
      }
      if (!extrinsics_node.IsMap()) {
        LOG(ERROR) << "Extrinsics node for camera " << camera_index << " is not a map.";
        return true;
      }

      // The vector from the origin of B to the origin of C expressed in B
      Eigen::Vector3d p_B_C;
      if (!YAML::safeGet(extrinsics_node, "p_B_C", &p_B_C)) {
        LOG(ERROR) << "Unable to get extrinsic position p_B_C for camera " << camera_index;
        return true;
      }

      // Get the quaternion. Hamiltonian, scalar first.
      Eigen::Matrix3d R_B_C_raw;
      if (!YAML::safeGet(extrinsics_node, "R_B_C", &R_B_C_raw)) {
        LOG(ERROR) << "Unable to get extrinsic rotation R_B_C for camera " << camera_index;
        return true;
      }
      aslam::Quaternion q_B_C = aslam::Quaternion::constructAndRenormalize(R_B_C_raw);

      aslam::Transformation T_B_C(q_B_C, p_B_C);

      // Fill in the data in the ncamera.
      cameras.push_back(camera);
      T_Ci_B.push_back(T_B_C.inverted());
    }

    // Create the ncamera and fill in all the data.
    ncamera.reset(new aslam::NCamera(ncam_id, T_Ci_B, cameras, label));

  } catch(const std::exception& e) {
    LOG(ERROR) << "Yaml exception during parsing: " << e.what();
    ncamera.reset();
    return true;
  }
  return true;
}

Node convert<std::shared_ptr<aslam::NCamera> >::encode(
    const std::shared_ptr<aslam::NCamera>& ncamera) {
  return convert<aslam::NCamera>::encode(*CHECK_NOTNULL(ncamera.get()));
}

bool convert<aslam::NCamera>::decode(const Node& /*node*/, aslam::NCamera& /*ncamera*/) {
  LOG(FATAL) << "Not implemented!";
  return false;
}

Node convert<aslam::NCamera>::encode(const aslam::NCamera& ncamera) {
  Node ncamera_node;

  ncamera_node["label"] = ncamera.getLabel();
  if(ncamera.getId().isValid()) {
    ncamera_node["id"] = ncamera.getId().hexString();
  }

  Node cameras_node;
  size_t num_cameras = ncamera.numCameras();
  for (size_t camera_index = 0; camera_index < num_cameras; ++camera_index) {
    Node camera_node;
    camera_node["camera"] = ncamera.getCamera(camera_index);

    Eigen::Vector3d p_B_C = ncamera.get_T_C_B(camera_index).inverted().getPosition();
    Eigen::Matrix3d R_B_C = ncamera.get_T_C_B(camera_index).inverted().getRotationMatrix();

    // The vector from the origin of B to the origin of C expressed in B
    Node extrinsics;
    extrinsics["p_B_C"] = p_B_C;
    extrinsics["R_B_C"] = R_B_C;
    camera_node["extrinsics"] = extrinsics;

    cameras_node.push_back(camera_node);
  }

  ncamera_node["cameras"] = cameras_node;

  return ncamera_node;
}

}  // namespace YAML