# RoboSense‑Fusion setup

This note captures everything completed so far, from having ROS 2 Jazzy installed to preparing the first module of the RoboSense‑Fusion project.

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
