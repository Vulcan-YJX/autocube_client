import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('autocube_client'),
        'config',
        'param.yaml'
    )

    ranger_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('ranger_base'),
                'launch',
                'ranger_mini_v3.launch.py'
            )
        )
    )

    return LaunchDescription([
        ranger_launch,
        Node(
            package='autocube_client',
            executable='autocube_client_node',
            name='autocube_client_node',
            output='screen',
            parameters=[
                config,
                # {
                #     "battery1_topic": "/" + d1_namespace + "/system_status_broadcaster/battery1",
                #     "battery2_topic": "/" + d1_namespace + "/system_status_broadcaster/battery2",
                #     "cmd_vel_topic": "/" + d1_namespace + "/user_command/cmd_twist"
                # }
            ]
        )
    ])
