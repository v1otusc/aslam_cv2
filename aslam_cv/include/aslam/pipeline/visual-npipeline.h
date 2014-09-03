#ifndef VISUAL_PIPELINE_H_
#define VISUAL_PIPELINE_H_

#include <memory>

#include <aslam/common/macros.h>

namespace cv { class Mat; }

namespace aslam {

class VisualNFrame;
class NCameras;

/// \class VisualNPipeline
/// \brief An interface for pipelines that turn images into VisualNFrames
///
/// This is the abstract interface for visual pipelines that turn raw images
/// into VisualNFrame data. The underlying pipeline may include undistortion
/// or rectification, image contrast enhancement, feature detection and
/// descriptor computation, or other operations.
///
/// The class has two NCameras calibration structs that represent the
/// intrinsic and extrinsic calibration of the camera system.
/// The "input" calibration (getInputNCameras()) represents the calibration of
/// raw camera system, before any image processing, resizing, or undistortion
/// has taken place. The "output" calibration (getOutputNCameras())
/// represents the calibration parameters of the images and keypoints that get
/// set in the VisualNFrames struct. These are the camera parameters after
/// image processing, resizing, undistortion, etc.
///
/// The class should synchronize images with nearby timestamps and handle
/// out-of-order images. When all frames of a VisualNFrame are complete,
/// they are added to a list of output frames in the order that they are
/// completed. This list should be sorted by time (oldest first) and the number
/// of elements can be queried by numVisualNFramesComplete(). The getNext()
/// function retrieves the oldest complete VisualNFrames and leaves the remaining.
/// The getLatestAndClear() function gets the newest VisualNFrames and discards
/// anything older.
class VisualNPipeline {
public:
  ASLAM_POINTER_TYPEDEFS(VisualNPipeline);
  ASLAM_DISALLOW_EVIL_CONSTRUCTORS(VisualNPipeline);

  VisualNPipeline();
  virtual ~VisualNPipeline();

  /// \brief Add an image to the visual pipeline
  ///
  /// This function is called by a user when an image is received.
  /// The pipeline then processes the images and constructs VisualNFrames
  /// call numVisualNFramesComplete() to find out how many VisualNFrames are
  /// completed.
  ///
  /// \param[in] cameraIndex The index of the camera that this image corresponds to
  /// \param[in] image the image data
  /// \param[in] systemStamp the host time in integer nanoseconds since epoch
  /// \param[in] hardwareStamp the camera's hardware timestamp. Can be set to "invalid".
  virtual void processImage(int cameraIndex,
                            const cv::Mat& image,
                            int64_t systemStamp,
                            int64_t hardwareStamp) = 0;


  /// \brief How many completed VisualNFrames are waiting to be retrieved?
  virtual size_t numVisualNFramesComplete() const = 0;

  /// \brief  Get the next available set of processed frames
  ///
  /// This may not be the latest data, it is simply the next in a FIFO queue.
  /// If there are no VisualNFrames waiting, this returns a NULL pointer.
  virtual std::shared_ptr<VisualNFrame> getNext() = 0;

  /// \brief Get the latest available data and clear anything older.
  ///
  /// If there are no VisualNFrames waiting, this returns a NULL pointer.
  virtual std::shared_ptr<VisualNFrame> getLatestAndClear() = 0;

  /// \brief Get the input camera system that corresponds to the images
  ///        passed in to processImage().
  ///
  /// Because this pipeline may do things like image undistortion or
  /// rectification, the input and output camera systems may not be the same.
  virtual std::shared_ptr<NCameras> getInputNCameras() const = 0;

  /// \brief Get the output camera system that corresponds to the VisualNFrame
  ///        data that comes out.
  ///
  /// Because this pipeline may do things like image undistortion or
  /// rectification, the input and output camera systems may not be the same.
  virtual std::shared_ptr<NCameras> getOutputNCameras() const = 0;
};
}  // namespace aslam
#endif // VISUAL_PIPELINE_H_ 