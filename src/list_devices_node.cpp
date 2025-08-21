/*******************************************************************************
 * Copyright (c) 2023 Orbbec 3D Technology, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#include <ros/ros.h>
#include <orbbec_camera/types.h>
#include <orbbec_camera/utils.h>
#include <string>
#include <regex>
#include <thread>
#include <vector>
#include <systemd/sd-bus.h>
std::string parseUsbPort(const std::string &line) {
  std::string port_id;
  std::regex self_regex("(?:[^ ]+/usb[0-9]+[0-9./-]*/){0,1}([0-9.-]+)(:){0,1}[^ ]*",
                        std::regex_constants::ECMAScript);
  std::smatch base_match;
  bool found = std::regex_match(line, base_match, self_regex);
  if (found) {
    port_id = base_match[1].str();
    if (base_match[2].str().empty())  // This is libuvc string. Remove counter is exists.
    {
      std::regex end_regex = std::regex(".+(-[0-9]+$)", std::regex_constants::ECMAScript);
      bool found_end = std::regex_match(port_id, base_match, end_regex);
      if (found_end) {
        port_id = port_id.substr(0, port_id.size() - base_match[1].str().size());
      }
    }
  }
  return port_id;
}
bool checkSystemdServices(){
  // Badger: Check if any of the depthcam systemd services are running,
  // return false and print the offending unit name(s).
  bool success = true;
  sd_bus *bus = nullptr;

  int ret = sd_bus_default_system(&bus);
  if (ret < 0) {
    ROS_ERROR_STREAM("Cannot verify state of systemd services");
    success = false;
  }
  else {
    const char* unit_path = nullptr;
    sd_bus_message* reply = nullptr;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;

    // List all units
    r = sd_bus_call_method(
      bus,
      "org.freedesktop.systemd1",
      "/org/freedesktop/systemd1",
      "org.freedesktop.systemd1.Manager",
      "ListUnits",
      NULL,
      &reply,
      NULL
    );

    if (r < 0) {
      ROS_ERROR_STREAM("Failed to ListUnits");
      success = false;
    }

    r = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
    if (r < 0) {
      ROS_ERROR_STREAM("Failed to enter array of structs");
      success = false;
    }

    // Itterate through all units, look for depthcam units.
    // If a depthcam unit is active, flip boolean and log error for it.
    while (sd_bus_message_enter_container(reply, 'r', "ssssssouso") > 0) {
      const char *unit_name, *active_state;
      r = sd_bus_message_read(reply, "ssss", &unit_name, NULL, NULL, &active_state);
      if (r < 0) {
        ROS_ERROR_STREAM("Failed to parse unit struct: " << strerror(-r));
        success = false;
      }
      else if (strncmp(unit_name, "ros.depthcam", strlen("ros.depthcam")) == 0) {
        if (strcmp(active_state, "active") == 0) {
          ROS_ERROR_STREAM("Unit is active: " << unit_name);
          success = false;
        }
      }
      sd_bus_message_skip(reply, "ssouso");
      sd_bus_message_exit_container(reply);
    }
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);
  }
  sd_bus_unref(bus);
  return success;
}
int main() {
  if (!checkSystemdServices()) {
    ROS_ERROR_STREAM("Aborting; Please stop all depthcam systemd services.");
    return 0;
  }
  auto context = std::make_shared<ob::Context>();
  context->setLoggerSeverity(OBLogSeverity::OB_LOG_SEVERITY_OFF);
  auto list = context->queryDeviceList();
  for (size_t i = 0; i < list->deviceCount(); i++) {
    try {
      auto device = list->getDevice(i);
      auto device_info = device->getDeviceInfo();
      std::string serial = device_info->serialNumber();
      std::string uid = device_info->uid();
      auto port_id = parseUsbPort(uid);
      ROS_INFO_STREAM("serial: " << serial);
      ROS_INFO_STREAM("port id : " << port_id);
    } catch (ob::Error &e) {
      ROS_ERROR_STREAM("list_device_node: " << e.getMessage());
    } catch (const std::exception &e) {
      ROS_ERROR_STREAM("list_device_node: " << e.what());
    } catch (...) {
      ROS_ERROR_STREAM("list_device_node: " << "unknown error");
    }
  }
  return 0;
}
