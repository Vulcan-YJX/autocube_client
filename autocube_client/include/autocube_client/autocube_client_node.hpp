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

#include <grpcpp/grpcpp.h>
#include <twist.grpc.pb.h>

#include <chrono>
#include <memory>
#include <string>

#include "geometry_msgs/msg/twist_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

class AutocubeClientNode : public rclcpp::Node
{
public:
  explicit AutocubeClientNode(const rclcpp::NodeOptions & options);

  ~AutocubeClientNode();

private:
  void reader_twist_loop();

  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr twist_pub_;

  std::shared_ptr<grpc::Channel> channel_ = nullptr;
  std::unique_ptr<autocube::TwistService::Stub> stub_ = nullptr;
  grpc::ClientContext twist_context_;
  std::shared_ptr<grpc::ClientReaderWriter<autocube::TwistMessage, autocube::TwistMessage>>
    twist_stream_ = nullptr;

  std::string address_;
  std::string cmd_vel_topic_;

  // Thread
  std::thread reader_thread_;
  std::atomic<bool> running_;
};

#endif  // AUTOCUBE_CLIENT__AUTOCUBE_CLIENT_NODE_HPP_
