cmake_minimum_required(VERSION 3.8)
project(multi_camera_pkg)

# Dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

##############################
# Component Library
##############################
add_library(dual_camera_component SHARED
  src/dual_camera_node.cpp
)

target_include_directories(dual_camera_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(dual_camera_component
  rclcpp
  rclcpp_components
  sensor_msgs
  cv_bridge
  OpenCV
)

# Register component
rclcpp_components_register_nodes(dual_camera_component "DualCameraNode")

##############################
# Executable (loads component)
##############################
add_executable(dual_camera_node
  src/main.cpp
)

target_link_libraries(dual_camera_node
  dual_camera_component
)

ament_target_dependencies(dual_camera_node
  rclcpp
  rclcpp_components
)

##############################
# Install Targets
##############################
install(TARGETS
  dual_camera_component
  dual_camera_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

##############################
# Package
##############################
ament_package()
