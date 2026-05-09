import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, RegisterEventHandler, EmitEvent
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
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

    autocube_node = Node(
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

    shutdown_on_node_exit = RegisterEventHandler(
        OnProcessExit(
            target_action=autocube_node,
            on_exit=[EmitEvent(event=Shutdown(reason='autocube_client_node exited'))]
        )
    )

    return LaunchDescription([
        ranger_launch,
        autocube_node,
        shutdown_on_node_exit,
    ])
