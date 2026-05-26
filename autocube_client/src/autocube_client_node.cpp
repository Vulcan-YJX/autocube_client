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
  this->get_parameter("user_cmd_topic", user_cmd_topic_);
  this->get_parameter("battery_type", battery_type_);
  this->get_parameter("battery1_topic", battery1_topic_);
  this->get_parameter("battery2_topic", battery2_topic_);
  this->get_parameter("twist_topic", twist_topic_);
  this->get_parameter("twist_type", twist_type_);
  this->get_parameter("json_topic", json_topic_);
  this->get_parameter("autocube_json_topic", autocube_json_topic_);

  battery1_sub_ = this->create_subscription<sensor_msgs::msg::BatteryState>(
    battery1_topic_, 10,
    std::bind(&AutocubeClientNode::battery1_callback, this, std::placeholders::_1));

  battery2_sub_ = this->create_subscription<sensor_msgs::msg::BatteryState>(
    battery2_topic_, 10,
    std::bind(&AutocubeClientNode::battery2_callback, this, std::placeholders::_1));

  if (twist_type_ == "TwistStamped") {
    twist_stamped_sub_ = this->create_subscription<geometry_msgs::msg::TwistStamped>(
      twist_topic_, 10,
      std::bind(&AutocubeClientNode::twist_stamped_callback, this, std::placeholders::_1));
  } else if (twist_type_ == "TwistWithCovariance") {
    twist_with_covariance_sub_ =
      this->create_subscription<geometry_msgs::msg::TwistWithCovariance>(
      twist_topic_, 10,
      std::bind(
        &AutocubeClientNode::twist_with_covariance_callback, this, std::placeholders::_1));
  } else if (twist_type_ == "TwistWithCovarianceStamped") {
    twist_with_covariance_stamped_sub_ =
      this->create_subscription<geometry_msgs::msg::TwistWithCovarianceStamped>(
      twist_topic_, 10,
      std::bind(
        &AutocubeClientNode::twist_with_covariance_stamped_callback, this,
        std::placeholders::_1));
  } else if (twist_type_ == "Odom") {
    odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
      twist_topic_, 10,
      std::bind(&AutocubeClientNode::odom_callback, this, std::placeholders::_1));
  } else {
    twist_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      twist_topic_, 10,
      std::bind(&AutocubeClientNode::twist_callback, this, std::placeholders::_1));
  }

  json_sub_ = this->create_subscription<std_msgs::msg::String>(
    json_topic_, 10,
    std::bind(&AutocubeClientNode::json_callback, this, std::placeholders::_1));

  user_cmd_pub_ = this->create_publisher<ddt_msgs::msg::UserCommand>(user_cmd_topic_, 10);
  json_cmd_pub_ = this->create_publisher<std_msgs::msg::String>(autocube_json_topic_, 10);

  set_controller_status_client_ =
    this->create_client<std_srvs::srv::SetBool>("command/set_controller_status");

  teleop_param_client_ =
    std::make_shared<rclcpp::AsyncParametersClient>(this, "teleop_command");

  channel_ = grpc::CreateChannel(address_, grpc::InsecureChannelCredentials());
  twist_stub_ = autocube::TwistService::NewStub(channel_);
  battery_stub_ = autocube::BatteryService::NewStub(channel_);
  heartbeat_stub_ = autocube::HeartbeatService::NewStub(channel_);
  json_stub_ = autocube::JsonService::NewStub(channel_);

  twist_stream_ = twist_stub_->TwistStream(&twist_context_);
  if (!twist_stream_) {
    RCLCPP_ERROR(this->get_logger(), "Failed to create twist stream");
    return;
  }

  battery_stream_ = battery_stub_->BatteryStream(&battery_context_);
  if (!battery_stream_) {
    RCLCPP_ERROR(this->get_logger(), "Failed to create battery stream");
    return;
  }

  json_stream_ = json_stub_->JsonStream(&json_context_);
  if (!json_stream_) {
    RCLCPP_ERROR(this->get_logger(), "Failed to create battery stream");
    return;
  }

  reader_thread_ = std::thread(&AutocubeClientNode::reader_twist_loop, this);
  heartbeat_thread_ = std::thread(&AutocubeClientNode::heartbeat_loop, this);
  json_thread_ = std::thread(&AutocubeClientNode::json_cmd_loop, this);

  RCLCPP_INFO(this->get_logger(), "gRPC Server listening on: '%s'", address_.c_str());

  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(100), std::bind(&AutocubeClientNode::timer_callback, this));
}

void AutocubeClientNode::battery1_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg)
{
  battery1_percent = msg->percentage;
}

void AutocubeClientNode::battery2_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg)
{
  battery2_percent = msg->percentage;
}

void AutocubeClientNode::json_callback(const std_msgs::msg::String::SharedPtr msg)
{
  autocube::JsonMessage json_msg;
  json_msg.set_json_data(msg->data);
  json_stream_->Write(json_msg);
}

void AutocubeClientNode::twist_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
  send_twist(*msg);
}

void AutocubeClientNode::twist_stamped_callback(
  const geometry_msgs::msg::TwistStamped::SharedPtr msg)
{
  send_twist(msg->twist);
}

void AutocubeClientNode::twist_with_covariance_callback(
  const geometry_msgs::msg::TwistWithCovariance::SharedPtr msg)
{
  send_twist(msg->twist);
}

void AutocubeClientNode::twist_with_covariance_stamped_callback(
  const geometry_msgs::msg::TwistWithCovarianceStamped::SharedPtr msg)
{
  send_twist(msg->twist.twist);
}

void AutocubeClientNode::odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  send_twist(msg->twist.twist);
}

void AutocubeClientNode::send_twist(const geometry_msgs::msg::Twist & twist)
{
  autocube::TwistMessage twist_msg;
  twist_msg.set_linear_x(twist.linear.x);
  twist_msg.set_linear_y(twist.linear.y);
  twist_msg.set_linear_z(twist.linear.z);
  twist_msg.set_angular_x(twist.angular.x);
  twist_msg.set_angular_y(twist.angular.y);
  twist_msg.set_angular_z(twist.angular.z);
  twist_stream_->Write(twist_msg);
}

void AutocubeClientNode::reader_twist_loop()
{
  autocube::TwistMessage msg;

  while (running_) {
    if (twist_stream_->Read(&msg)) {
      ddt_msgs::msg::UserCommand ros_msg;
      ros_msg.header.stamp = this->get_clock()->now();
      ros_msg.header.frame_id = "base_link";

      ros_msg.twist.linear.x = msg.linear_x();
      ros_msg.twist.linear.y = msg.linear_y();
      ros_msg.twist.linear.z = msg.linear_z();
      ros_msg.twist.angular.x = msg.angular_x();
      ros_msg.twist.angular.y = msg.angular_y();
      ros_msg.twist.angular.z = msg.angular_z();

      user_cmd_pub_->publish(ros_msg);
    }
  }
  RCLCPP_WARN(this->get_logger(), "Reader twist thread exited");
}

void AutocubeClientNode::json_cmd_loop()
{
  autocube::JsonMessage msg;

  while (running_) {
    if (json_stream_->Read(&msg)) {
      try {
        std::string json_str = msg.json_data();
        nlohmann::json json_data = nlohmann::json::parse(json_str);
        if (json_data.contains("type")) {
          if(json_data["type"] == "robot"){
            if (json_data.contains("msg")) {
              nlohmann::json json_msg = json_data["msg"];
              ddt_msgs::msg::UserCommand cmd_msg;
              cmd_msg.header.stamp = this->get_clock()->now();
              cmd_msg.header.frame_id = "base_link";
              cmd_msg.fsm_mode = json_msg["cmd"].get<std::string>();
              user_cmd_pub_->publish(cmd_msg);
            }
          } else if (json_data["type"] == "robot_param") {
            if (json_data.contains("msg")) {
              if(json_data["param"] == "quattro"){
                auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
                request->data = json_data["value"].get<bool>();
                if (set_controller_status_client_->service_is_ready()) {
                  set_controller_status_client_->async_send_request(request);
                  RCLCPP_INFO(
                    this->get_logger(),
                    "Sent request to command/set_controller_status with value: %s",
                    request->data ? "true" : "false");
                } else {
                  RCLCPP_WARN(
                    this->get_logger(),
                    "Service command/set_controller_status not available");
                }
              } else if (json_data["param"] == "use_sdk") {
                bool value = json_data["value"].get<bool>();
                if (teleop_param_client_->service_is_ready()) {
                  teleop_param_client_->set_parameters(
                    {rclcpp::Parameter("use_sdk", value)});
                  RCLCPP_INFO(
                    this->get_logger(),
                    "Set teleop_command param use_sdk: %s",
                    value ? "true" : "false");
                } else {
                  RCLCPP_WARN(
                    this->get_logger(),
                    "Parameter service for teleop_command not available");
                }
              }

            }
          }
        }
        std_msgs::msg::String ros_msg;
        ros_msg.data = msg.json_data();
        json_cmd_pub_->publish(ros_msg);
      } catch (nlohmann::json::parse_error& e) {
          std::cout << "JSON解析失败: " << e.what() << std::endl;
      }
    }
  }
  RCLCPP_WARN(this->get_logger(), "Reader twist thread exited");
}

void AutocubeClientNode::heartbeat_loop()
{
  while (running_) {
    grpc::ClientContext heartbeat_context;
    heartbeat_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

    google::protobuf::Empty req;
    google::protobuf::Empty resp;
    grpc::Status status = heartbeat_stub_->Heartbeat(&heartbeat_context, req, &resp);

    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void AutocubeClientNode::timer_callback()
{
  auto battery_average = (battery1_percent + battery2_percent) / 2.0;
  autocube::BatteryMessage battery_msg;
  battery_msg.set_battery_one(battery1_percent);
  battery_msg.set_battery_two(battery2_percent);
  battery_msg.set_average(battery_average);
  battery_msg.set_type(battery_type_);
  battery_stream_->Write(battery_msg);
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
  if (heartbeat_thread_.joinable()) {
    heartbeat_thread_.join();
  }
  auto status = twist_stream_->Finish();
  if (!status.ok()) {
    RCLCPP_ERROR(this->get_logger(), "gRPC finish error: %s", status.error_message().c_str());
  }
}
