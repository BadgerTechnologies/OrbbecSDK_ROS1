#include <signal.h>

#include <memory>

#include <nodelet/nodelet.h>

#include "orbbec_camera/ob_camera_node_driver.h"

void term_handler(int) {
  ros::requestShutdown();
}

namespace orbbec_camera
{

class OrbbecCameraNodelet : public nodelet::Nodelet
{
public:
  OrbbecCameraNodelet()  {};

  ~OrbbecCameraNodelet() {}

private:
  virtual void onInit()
  {
    /*
     * handle SIGTERM and SIGINT so that we can shutdown cleanly. We have seen
     * cameras get into bad states when this is not shutdown cleanly.
     */
    signal(SIGTERM, term_handler);
    signal(SIGINT, term_handler);
    driver_ptr_ = std::make_unique<OBCameraNodeDriver>(getNodeHandle(), getPrivateNodeHandle());
  };

  std::unique_ptr<OBCameraNodeDriver> driver_ptr_;
};

}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(orbbec_camera::OrbbecCameraNodelet, nodelet::Nodelet)
