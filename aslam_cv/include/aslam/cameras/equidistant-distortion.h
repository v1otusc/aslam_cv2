#ifndef ASLAM_EQUIDISTANT_DISTORTION_H_
#define ASLAM_EQUIDISTANT_DISTORTION_H_

#include <Eigen/Core>
#include <glog/logging.h>
#include "distortion.h"

namespace aslam {

/// \class EquidistantDistortion
/// \brief An implementation of the equidistant distortion model for pinhole cameras.
///        See "A Generic Camera Model and Calibration Method for Conventional, Wide-Angle, and
///        Fish-Eye Lenses" by Juho Kannala and Sami S. Brandt for further information.
///        The ordering of the parameter vector is: k1 k2 k3 k4
///        NOTE: The inverse transformation (undistort) in this case is not available in
///        closed form and so it is computed iteratively!
class EquidistantDistortion : public aslam::Distortion {

 private:
  /** \brief Number of parameters used for this distortion model. */
  enum { kNumOfParams = 4 };

 public:
  enum { CLASS_SERIALIZATION_VERSION = 1 };

  //////////////////////////////////////////////////////////////
  /// \name Constructors/destructors and operators
  /// @{

  /// \brief EquidistantDistortion Ctor.
  /// @param[in] distortionParams Vector containing the distortion parameter. (dim=1)
  explicit EquidistantDistortion(const Eigen::VectorXd& distortionParams);

  /// @}

  //////////////////////////////////////////////////////////////
  /// \name Distort methods: applies the distortion model to a point.
  /// @{
  /// \brief Apply distortion to a point in the normalized image plane using provided distortion
  ///        coefficients. External distortion coefficients can be specified using this function.
  ///        Ignores the internally stored parameters.
  /// @param[in]     dist_coeffs  Vector containing the coefficients for the distortion model.
  /// @param[in,out] point        The point in the normalized image plane. After the function,
  ///                             this point is distorted.
  /// @param[out]    out_jacobian The Jacobian of the distortion function with respect to small
  ///                             changes in the input point. If NULL is passed, the Jacobian
  ///                             calculation is skipped.
  virtual void distortUsingExternalCoefficients(const Eigen::VectorXd& dist_coeffs,
                                                Eigen::Vector2d* point,
                                                Eigen::Matrix2d* out_jacobian) const;

  /// \brief Templated version of the distortExternalCoeffs function.
  /// @param[in]  dist_coeffs Vector containing the coefficients for the distortion model.
  /// @param[in]  point       The point in the normalized image plane. After the function, this
  ///                         point is distorted.
  /// @param[out] out_point   The distorted point.
  template <typename ScalarType>
  void distortUsingExternalCoefficients(const Eigen::Map<Eigen::Matrix<ScalarType,
                                        Eigen::Dynamic,1>>& dist_coeffs,
                                        const Eigen::Matrix<ScalarType, 2, 1>& point,
                                        Eigen::Matrix<ScalarType, 2, 1>* out_point) const;

  /// \brief Apply distortion to the point and provide the Jacobian of the distortion with respect
  ///        to small changes in the distortion parameters.
  /// @param[in]  dist_coeffs  Vector containing the coefficients for the distortion model.
  /// @param[in]  point        The point in the normalized image plane. After the function,
  ///                          this point is distorted.
  /// @param[out] out_jacobian The Jacobian of the distortion with respect to small changes in
  ///                          the distortion parameters.
  virtual void distortParameterJacobian(const Eigen::VectorXd& dist_coeffs,
                                        const Eigen::Vector2d& point,
                                        Eigen::Matrix<double, 2, Eigen::Dynamic>* out_jacobian) const;

  /// @}

  //////////////////////////////////////////////////////////////
  /// \name Undistort methods: Removes the modeled distortion effects from a point.
  /// @{

  /// \brief Apply undistortion to recover a point in the normalized image plane using provided
  ///        distortion coefficients. External distortion coefficients can be specified using this
  ///        function. Ignores the internally  stored parameters.
  /// @param[in]      dist_coeffs  Vector containing the coefficients for the distortion model.
  /// @param[in,out]  point        The distorted point. After the function, this point is in the
  ///                              normalized image plane.
  virtual void undistortUsingExternalCoefficients(const Eigen::VectorXd& dist_coeffs,
                                                  Eigen::Vector2d* point) const;

  /// @}

  ///////////////////////////////////////////////////////////////////////////////
  /// \name Methods to set/get distortion parameters
  /// @{

  /// \brief Check the validity of distortion parameters.
  /// @param[in] dist_coeffs Vector containing the coefficients.
  ///            Parameters will NOT be stored.
  /// @return If the distortion parameters are valid.
  virtual bool distortionParametersValid(const Eigen::VectorXd& dist_coeffs) const;

  /// \brief Returns the number of parameters used in this distortion model.
  inline static constexpr size_t parameterCount() {
      return kNumOfParams;
  }

  /// \brief Print the internal parameters of the distortion in a human-readable form
  /// Print to the ostream that is passed in. The text is extra
  /// text used by the calling function to distinguish cameras.
  virtual void printParameters(std::ostream& out, const std::string& text) const;

  /// @}

};

} // namespace aslam

#endif /* ASLAM_EQUIDISTANT_DISTORTION_H_ */
