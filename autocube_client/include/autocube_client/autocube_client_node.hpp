// Copyright 2026 Vulcan
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

#ifndef AUTOCUBE_CLIENT__AUTOCUBE_CLIENT_NODE_HPP_
#define AUTOCUBE_CLIENT__AUTOCUBE_CLIENT_NODE_HPP_

#include <battery.grpc.pb.h>
#include <grpcpp/grpcpp.h>
#include <heartbeat.grpc.pb.h>
#include <twist.grpc.pb.h>
#include <json.grpc.pb.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/twist_with_covariance.hpp"
#include "geometry_msgs/msg/twist_with_covariance_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/battery_state.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/set_bool.hpp"
#include "ddt_msgs/msg/user_command.hpp"
#include "nlohmann/json.hpp"

class AutocubeClientNode : public rclcpp::Node
{
public:
  explicit AutocubeClientNode(const rclcpp::NodeOptions & options);

  ~AutocubeClientNode();

private:
  void reader_twist_loop();

  void heartbeat_loop();

  void json_cmd_loop();

  void battery1_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);

  void battery2_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);

  void twist_callback(const geometry_msgs::msg::Twist::SharedPtr msg);

  void twist_stamped_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg);

  void twist_with_covariance_callback(
    const geometry_msgs::msg::TwistWithCovariance::SharedPtr msg);

  void twist_with_covariance_stamped_callback(
    const geometry_msgs::msg::TwistWithCovarianceStamped::SharedPtr msg);

  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg);

  void send_twist(const geometry_msgs::msg::Twist & twist);

  void json_callback(const std_msgs::msg::String::SharedPtr msg);

  void timer_callback();

  rclcpp::TimerBase::SharedPtr timer_;

  rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery1_sub_;
  rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery2_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_sub_;
  rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr twist_stamped_sub_;
  rclcpp::Subscription<geometry_msgs::msg::TwistWithCovariance>::SharedPtr
    twist_with_covariance_sub_;
  rclcpp::Subscription<geometry_msgs::msg::TwistWithCovarianceStamped>::SharedPtr
    twist_with_covariance_stamped_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr json_sub_;

  rclcpp::Publisher<ddt_msgs::msg::UserCommand>::SharedPtr user_cmd_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr json_cmd_pub_;

  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr set_controller_status_client_;

  rclcpp::AsyncParametersClient::SharedPtr teleop_param_client_;

  grpc::ClientContext twist_context_;
  grpc::ClientContext battery_context_;
  grpc::ClientContext json_context_;

  std::shared_ptr<grpc::Channel> channel_ = nullptr;
  std::unique_ptr<autocube::TwistService::Stub> twist_stub_ = nullptr;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::TwistMessage, autocube::TwistMessage>>
    twist_stream_ = nullptr;

  std::unique_ptr<autocube::BatteryService::Stub> battery_stub_ = nullptr;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::BatteryMessage, autocube::BatteryMessage>>
    battery_stream_ = nullptr;
  
  std::unique_ptr<autocube::JsonService::Stub> json_stub_ = nullptr;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::JsonMessage, autocube::JsonMessage>>
    json_stream_ = nullptr;
  
  std::unique_ptr<autocube::HeartbeatService::Stub> heartbeat_stub_ = nullptr;

  double battery1_percent = 0;
  double battery2_percent = 0;

  int battery_type_ = 0;
  std::string address_;
  std::string twist_topic_;
  std::string twist_type_;
  std::string battery1_topic_;
  std::string battery2_topic_;
  std::string user_cmd_topic_;
  std::string json_topic_;
  std::string autocube_json_topic_;

  // Thread
  std::thread heartbeat_thread_;
  std::thread reader_thread_;
  std::thread json_thread_;
  std::atomic<bool> running_;
};

#endif  // AUTOCUBE_CLIENT__AUTOCUBE_CLIENT_NODE_HPP_
