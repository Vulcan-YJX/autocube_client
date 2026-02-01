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

#include "autocube_client/autocube_client_node.hpp"

AutocubeClientNode::AutocubeClientNode(const rclcpp::NodeOptions & options)
: Node("autocube_client_node", options), running_(true)
{
  this->get_parameter("address", address_);
  this->get_parameter("cmd_vel_topic", cmd_vel_topic_);

  twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(cmd_vel_topic_, 10);

  channel_ = grpc::CreateChannel(address_, grpc::InsecureChannelCredentials());
  stub_ = autocube::TwistService::NewStub(channel_);

  twist_stream_ = stub_->TwistStream(&twist_context_);

  if (!twist_stream_) {
    RCLCPP_ERROR(this->get_logger(), "Failed to create gRPC stream");
    return;
  }

  reader_thread_ = std::thread(&AutocubeClientNode::reader_twist_loop, this);

  RCLCPP_INFO(this->get_logger(), "gRPC Server listening on: '%s'", address_.c_str());
}

void AutocubeClientNode::reader_twist_loop()
{
  autocube::TwistMessage msg;

  while (running_ && twist_stream_->Read(&msg)) {
    geometry_msgs::msg::TwistStamped ros_msg;
    ros_msg.header.stamp = this->get_clock()->now();
    ros_msg.header.frame_id = "base_link";
    
    ros_msg.twist.linear.x = msg.linear_x();
    ros_msg.twist.linear.y = msg.linear_y();
    ros_msg.twist.linear.z = msg.linear_z();
    ros_msg.twist.angular.x = msg.angular_x();
    ros_msg.twist.angular.y = msg.angular_y();
    ros_msg.twist.angular.z = msg.angular_z();

    twist_pub_->publish(ros_msg);
  }
  RCLCPP_WARN(this->get_logger(), "Reader twist thread exited");
  std::exit(EXIT_FAILURE);
}

AutocubeClientNode::~AutocubeClientNode()
{
  running_ = false;

  if (twist_stream_) {
    twist_stream_->WritesDone();
  }

  if (reader_thread_.joinable()) {
    reader_thread_.join();
  }

  auto status = twist_stream_->Finish();
  if (!status.ok()) {
    RCLCPP_ERROR(this->get_logger(), "gRPC finish error: %s", status.error_message().c_str());
  }
}
