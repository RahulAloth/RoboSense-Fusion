# RoboSense‑Fusion setup

- Below describes the major setup done for making the Project working 
---

## ✔ ROS 2 Environment

- Install ROS 2 **Jazzy** 
- Make System is ready for Modern C++ (C++17/20) development.
- No additional ROS installation required.

---

## ✔ Workspace Setup

A dedicated workspace for RoboSense‑Fusion:

```bash
mkdir -p ~/robosense_ws/src
cd ~/robosense_ws
colcon build
source /opt/ros/jazzy/setup.bash
```

## ✔ Project Creation

The core package for the perception engine:
```bash
cd ~/robosense_ws/src
ros2 pkg create robosense_fusion \
  --build-type ament_cmake \
  --dependencies rclcpp rclcpp_lifecycle tf2 tf2_ros std_msgs geometry_msgs
```

- This creates the base structure where all modules will be added.
## ✔ Project Direction Defined
- RoboSense‑Fusion is a Modern C++ ROS 2 perception engine for a factory‑floor robot using:
    - Radar
    - Camera
    - Vehicle dynamics
    - Detection
    - Fusion
- The architecture is modular and SoC‑ready.
## ✔ Module 1 Directory Layout
```bash
robosense_fusion/
  include/
    robosense_fusion/
      core/
        engine_node.hpp
  src/
    core/
      engine_node.cpp
  launch/
    engine.launch.py
  CMakeLists.txt
  package.xml
```
## ✔ Module 1 Node Responsibilities
    - Create a lifecycle node (engine_node)
    - Publish /system/heartbeat
    - Broadcast static TF2 transforms:
        - base_link → camera_link
        - base_link → radar_link
    - Load parameters:
        - use_sim_time
        - enable_radar
        - enable_camera
        - enable_fusion
