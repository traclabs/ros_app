project(CFE_ROS_APP C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)

# Create the app module
add_cfe_app(ros_app fsw/src/ros_app.c)

# Add table
add_cfe_tables(rosAppTable fsw/tables/ros_app_tbl.c)

