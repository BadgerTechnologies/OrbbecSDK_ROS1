#include <ros/ros.h>
#include <orbbec_camera/types.h>

int main() {
  auto context = std::make_shared<ob::Context>();
  context->setLoggerSeverity(OBLogSeverity::OB_LOG_SEVERITY_NONE);
  auto list = context->queryDeviceList();
  for (size_t i = 0; i < list->deviceCount(); i++) {
    try {
      auto serial = list->getDevice(i)->getDeviceInfo()->serialNumber();
      ROS_INFO_STREAM("serial: " << serial);
      // Badger specific format, parsed by depthcam ID scripts, based on legacy
      // ros_astra_camera format. The parsing scripts need the formatted text to
      // not have ROS log headers, so output directly to stdout.
      auto device_info = list->getDevice(i)->getDeviceInfo();
      std::cout << "Device #" << i << ":" << std::endl;
      std::cout << "Name: " << device_info->name() << std::endl;
      std::cout << "Firmware version: " << device_info->firmwareVersion() << std::endl;
      std::cout << "UID: " << device_info->uid() <<
        " (Vendor ID: " << std::hex << device_info->vid() <<
        ", Product ID: " << std::hex << device_info->pid() << ")" << std::endl;
      std::cout << "Serial number: " << serial << std::endl;
    } catch(const ob::Error& e) {
      ROS_WARN_STREAM("Failed to enumerate device number " << i);
    }
  }
  return 0;
}
