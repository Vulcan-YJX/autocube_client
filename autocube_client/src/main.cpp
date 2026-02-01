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

#include <cstdlib>  // for setenv / _putenv
#include <iostream>

#include "autocube_client/autocube_client_node.hpp"

int main(int argc, char * argv[])
{
  #ifdef _WIN32
    _putenv("GRPC_DNS_RESOLVER=native");  // Windows
  #else
      setenv("GRPC_DNS_RESOLVER", "native", 1); // Linux/macOS
  #endif
  rclcpp::init(argc, argv);

  auto options = rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<AutocubeClientNode>(options);
  auto executor = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
  executor->add_node(node);
  executor->spin();
  rclcpp::shutdown();
  return 0;
}
