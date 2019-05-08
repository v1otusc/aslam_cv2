#ifndef ASLAM_NCAMERA_H_
#define ASLAM_NCAMERA_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <aslam/common/macros.h>
#include <aslam/common/pose-types.h>
#include <aslam/common/sensor.h>
#include <aslam/common/unique-id.h>
#include <aslam/common/yaml-serialization.h>
#include <gtest/gtest_prod.h>

namespace sm {
class PropertyTree;
}
namespace aslam {
class Camera;
}

namespace aslam {

/// \class NCameras
/// \brief A class representing a calibrated multi-camera system
///
/// Coordinate frames involved:
/// - B  : The body frame of the camera rig
/// - Ci : A coordinate frame attached to camera i.
///
class NCamera : public Sensor {
 public:
  ASLAM_POINTER_TYPEDEFS(NCamera);
  enum {CLASS_SERIALIZATION_VERSION = 1};
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

//protected:
  /// Default constructor builds an empty camera rig.
  NCamera();

//public:
  /// \brief initialize from a list of transformations and a list of cameras
  ///
  /// The two lists must be parallel arrays (same size). The transformation
  /// at T_C_B[i] corresponds to the camera at cameras[i].
  ///
  /// @param id unique id for this camera rig
  /// @param T_C_B a list of transformations that take points from B to Ci
  /// @param cameras a list cameras
  /// @param description a human-readable description of this camera rig
  NCamera(const NCameraId& id,
      const TransformationVector& T_C_B,
      const std::vector<std::shared_ptr<Camera>>& cameras,
      const std::string& description);

  /// Initialize from a property tree.
  NCamera(const sm::PropertyTree& propertyTree);
  virtual ~NCamera() = default;

  /// Copy constructor for clone.
  NCamera(const NCamera&);
  void operator=(const NCamera&) = delete;
  bool operator==(const NCamera& other) const;

  /// Methods to clone this instance. All contained camera objects are cloned.
  /// (Make sure the Camera and NCamera ID's are set to your requirement after cloning!)
  NCamera* clone() const {
    return new NCamera(static_cast<NCamera const&>(*this));
  };

  NCamera::Ptr cloneToShared() const {
    return aligned_shared<NCamera>(*this);
  }

  Sensor::Ptr cloneAsSensor() const override {
    return std::dynamic_pointer_cast<Sensor>(cloneToShared());
  }

  /// Get sensor type as an integer or as a string
  int getSensorType() const override {
    return SensorType::kNCamera;
  }

  std::string getSensorTypeString() const override {
    return static_cast<std::string>(kNCameraIdentifier);
  }

  /// Get the number of cameras.
  size_t getNumCameras() const;

  /// Get the pose of body frame with respect to the camera i.
  const Transformation& get_T_C_B(size_t camera_index) const;

  /// Get the pose of body frame with respect to the camera i.
  Transformation& get_T_C_B_Mutable(size_t camera_index);

  /// Get the pose of body frame with respect to the camera with a camera id.
  /// The method will assert that the camera is not in the rig!
  const Transformation& get_T_C_B(const CameraId& camera_id) const;

  /// Get the pose of body frame with respect to the camera with a camera id.
  /// The method will assert that the camera is not in the rig!
  Transformation& get_T_C_B_Mutable(const CameraId& camera_id);

  /// Set the pose of body frame with respect to the camera i.
  void set_T_C_B(size_t camera_index, const Transformation& T_Ci_B);

  /// Get all transformations.
  const TransformationVector& getTransformationVector() const;

  /// Get the geometry object for camera i.
  const Camera& getCamera(size_t camera_index) const;

  /// Get the geometry object for camera i.
  Camera& getCameraMutable(size_t camera_index);

  /// Get the geometry object for camera i.
  std::shared_ptr<Camera> getCameraShared(size_t camera_index);

  /// Get the geometry object for camera i.
  std::shared_ptr<const Camera> getCameraShared(size_t camera_index) const;

  /// Get the geometry object for camera i.
  void setCamera(size_t camera_index, std::shared_ptr<Camera> camera);

  /// How many cameras does this system have?
  size_t numCameras() const;

  /// Get all cameras.
  const std::vector<std::shared_ptr<Camera>>& getCameraVector() const;

  /// Gets the id for the camera at index i.
  const CameraId& getCameraId(size_t camera_index) const;

  /// Does this rig have a camera with this id.
  bool hasCameraWithId(const CameraId& id) const;

  /// \brief Get the index of the camera with the id.
  /// @returns -1 if the rig doesn't have a camera with this id.
  int getCameraIndex(const CameraId& id) const;

  /// Create a copy of this NCamera with all distortion models removed. All internal cameras
  /// get cloned and new IDs will be assigned to the cloned NCamera and all contained cameras.
  aslam::NCamera::Ptr cloneRigWithoutDistortion() const;

  std::string getComparisonString(const NCamera& other) const;

 private:
  bool isValidImpl() const override;

  void setRandomImpl() override;

  bool isEqualImpl(const Sensor& other) const override;

  virtual bool loadFromYamlNodeImpl(const YAML::Node&) override;
  virtual void saveToYamlNodeImpl(YAML::Node*) const override;

  /// Internal consistency checks and initialization.
  void initInternal();

  /// The mounting transformations.
  TransformationVector T_C_B_;

  /// The camera geometries.
  std::vector<std::shared_ptr<Camera>> cameras_;

  /// Map from camera id to index.
  std::unordered_map<CameraId, size_t> id_to_index_;
};

} // namespace aslam

#endif /* ASLAM_NCAMERA_H_ */
