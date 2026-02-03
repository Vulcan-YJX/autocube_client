import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

d1_namespace = 'd1'

unique_id_file = '/proc/device-tree/serial-number'
try:
    with open(unique_id_file, 'r') as file:
        unique_id = file.read().strip()
        unique_id = unique_id.replace('\x00', '')[6:]
        d1_namespace = 'd1'+unique_id
except FileNotFoundError:
        d1_namespace='d1'
except Exception as e:
        d1_namespace='d1'


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('autocube_client'),
        'config',
        'param.yaml'
    )

    return LaunchDescription([
        Node(
            package='autocube_client',
            executable='autocube_client_node',
            name='autocube_client_node',
            output='screen',
            parameters=[
                config,
                {
                    "battery1_topic": "/" + d1_namespace + "/system_status_broadcaster/battery1",
                    "battery2_topic": "/" + d1_namespace + "/system_status_broadcaster/battery2"
                }
            ]
        )
    ])
