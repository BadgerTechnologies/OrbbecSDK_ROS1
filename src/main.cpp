#include <signal.h>

#include "ros/ros.h"
#include "orbbec_camera/ob_camera_node_driver.h"

void term_handler(int) {
  ros::requestShutdown();
}

int main(int argc, char** argv) {
  /*
   * handle SIGTERM and SIGINT so that we can shutdown cleanly. We have seen
   * cameras get into bad states when this is not shutdown cleanly.
   */
  signal(SIGTERM, term_handler);
  signal(SIGINT, term_handler);
  ros::init(argc, argv, "orbbec_camera");
  ros::NodeHandle nh;
  ros::NodeHandle nh_private("~");
  orbbec_camera::OBCameraNodeDriver ob_camera_node_factory(nh, nh_private);
  ros::spin();
  ros::shutdown();
  return 0;
}
