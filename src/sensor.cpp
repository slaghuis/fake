// Copyright (c) 2022 Eric Slaghuis
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/* **********************************************************************
 * Published fake range sensor data by reading the odometry of the drone
 * and publishing the Z measurement as a range reading.
 * Faking a VL53L1X sensor
 * ***********************************************************************/
#include <memory>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/range.hpp"

using std::placeholders::_1;

class FakeSensor : public rclcpp::Node
{
  public:
    FakeSensor()
    : Node("fake_sensor")
    {
      publisher_ = this->create_publisher<sensor_msgs::msg::Range>("vl53l1x/range", 10);
      subscription_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "drone/odom", 10, std::bind(&FakeSensor::odom_callback, this, _1));
    }

  private:
    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
    {
      rclcpp::Time now = this->get_clock()->now();
      auto message = sensor_msgs::msg::Range();
      message.header.frame_id = "base_down";
      message.header.stamp = now;
      message.radiation_type = sensor_msgs::msg::Range::INFRARED;
      message.field_of_view = 0.47;              // Typically 27 degrees or 0,471239 radians
      message.min_range = 0.14;                  // 140 mm.  (It is actully much less, but this makes sense in the context
      message.max_range = 3.00;                  // 3.6 m. in the dark, down to 73cm in bright light

      message.range =  0.05 - (float) msg->pose.pose.position.z; // range in meters.  Offset of sensor above ground plus altitude in NED frame     
      
      publisher_->publish(message);
      
    }
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;
    rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FakeSensor>());
  rclcpp::shutdown();
  return 0;
}