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

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/battery_state.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "autocube_client/msg/user_command.hpp"

class AutocubeClientNode : public rclcpp::Node
{
public:
  explicit AutocubeClientNode(const rclcpp::NodeOptions & options);

  ~AutocubeClientNode();

private:
  void reader_twist_loop();

  void heartbeat_loop();

  void battery1_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);

  void battery2_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);

  void twist_callback(const geometry_msgs::msg::Twist::SharedPtr msg);

  void timer_callback();

  rclcpp::TimerBase::SharedPtr timer_;

  rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery1_sub_;
  rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery2_sub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_sub_;  

  rclcpp::Publisher<autocube_client::msg::UserCommand>::SharedPtr user_cmd_pub_;

  grpc::ClientContext twist_context_;
  grpc::ClientContext battery_context_;

  std::shared_ptr<grpc::Channel> channel_ = nullptr;
  std::unique_ptr<autocube::TwistService::Stub> twist_stub_ = nullptr;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::TwistMessage, autocube::TwistMessage>>
    twist_stream_ = nullptr;

  std::unique_ptr<autocube::BatteryService::Stub> battery_stub_ = nullptr;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::BatteryMessage, autocube::BatteryMessage>>
    battery_stream_ = nullptr;

  std::unique_ptr<autocube::HeartbeatService::Stub> heartbeat_stub_ = nullptr;

  double battery1_percent = 0;
  double battery2_percent = 0;

  int battery_type_ = 0;
  std::string address_;
  std::string twist_topic_;
  std::string battery1_topic_;
  std::string battery2_topic_;
  std::string user_cmd_topic_;

  // Thread
  std::thread heartbeat_thread_;
  std::thread reader_thread_;
  std::atomic<bool> running_;
};

#endif  // AUTOCUBE_CLIENT__AUTOCUBE_CLIENT_NODE_HPP_
