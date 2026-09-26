import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import TimerAction

def generate_launch_description():

    map_file = os.path.join(get_package_share_directory('project_mapping'), 'maps', 'citylab_map.yaml')
    nav2_yaml = os.path.join(get_package_share_directory('project_localization'), 'config', 'amcl_config.yaml')
    rviz_file = os.path.join(get_package_share_directory('project_localization'), 'config', 'localization.rviz')


    map_node = Node(package='nav2_map_server',
            executable='map_server',
            name='map_server',
            output='screen',
            parameters=[{'use_sim_time': True}, 
                        {'yaml_filename':map_file} 
                       ])

    amcl_node = Node(package='nav2_amcl',
            executable='amcl',
            name='amcl',
            output='screen',
            parameters=[nav2_yaml]
)

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_file],
        output='screen',
        parameters=[{'use_sim_time': True}]
    )

    lifecycle_manager_node = Node(
            package='nav2_lifecycle_manager',
            executable='lifecycle_manager',
            name='lifecycle_manager_localization',
            output='screen',
            parameters=[{'use_sim_time': True},
                        {'autostart': True},
                        {'node_names': ['map_server', 'amcl']}])



    # create and return launch description object
    return LaunchDescription(
        [
            map_node,
            amcl_node,
            rviz_node,
            lifecycle_manager_node,

        ]
    )