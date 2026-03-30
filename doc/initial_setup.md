# RoboSense‑Fusion Workspace Setup History  
*A clean chronological log of all steps performed during environment setup.*

---
## Workstation set up
    - Ubuntu 24.04 LTS
    - ROS 2 Jazzy installed under /opt/ros/jazzy/
    - Basic development tools:
        - gcc, g++
        - colcon
        - cmake
    - Git + GitHub SSH access configured
	- VS Code installed (optional but recommended)
	
## ROS 2 Environment Setup
- Source ROS 2 Jazzy

```bash
source /opt/ros/jazzy/setup.bash
```
- Verify ROS installation

```bash
ros2 --version
echo $ROS_DISTRO
```
# Create Project Workspace

- Create workspace directory

```bash
cd ~/AiStätt/
git clone git@github.com:RahulAloth/RoboSense-Fusion.git
cd RoboSense-Fusion
```	

##  Repository Cloning

```bash
git clone git@github.com:RahulAloth/RoboSense-Fusion.git
cd RoboSense-Fusion/
ls
```

## Initialize ROS 2 Workspace Structure
- Created the standard ROS 2 workspace directories:
```bash
mkdir src
mkdir build
mkdir install
mkdir log
```
## Create ROS 2 Packages
 - Navigated into the source folder:
 ```bash
cd ~/AiStätt/RoboSense-Fusion/src
```
- Created three modular packages:
 ```bash
ros2 pkg create --build-type ament_cmake camera_node
ros2 pkg create --build-type ament_cmake camera_preprocess
ros2 pkg create --build-type ament_cmake camera_detection

```
 
## Build the Workspace
- Returned to workspace root:

 ```bash
cd ~/AiStätt/RoboSense-Fusion
colcon build --symlink-install
```

## Source the Workspace
- Initial sourcing:
 
 ```bash
source ~/AiStätt/RoboSense-Fusion/install/setup.bash
```
- Added to .bashrc:
 
 ```bash
echo "source ~/AiStätt/RoboSense-Fusion/install/setup.bash" >> ~/.bashrc
source ~/.bashrc

```
## Verify ROS 2 Environment
 ```bash
echo $ROS_DISTRO
```

## Install ROS Developer Tools

- These tools help generate VS Code configuration files.
 ```bash
sudo apt update
sudo apt install ros-dev-tools
 ```
 
 - Generate VS Code config
 ```bash
ros2 run ros_dev_tools generate_vscode_config
```

## Camera Tools & Video Device Setup
- To work with /dev/video0 cameras:
##### Install V4L2 utilities

 ```bash
sudo apt install v4l-utils
v4l2-ctl --list-devices
v4l2-ctl --list-formats-ext -d /dev/video0
 ```
 
 ##### Install FFmpeg for quick camera testing
 
  ```bash
sudo apt install ffmpeg
ffplay /dev/video0

 ```
##### Cross‑Compilation Tools (Optional)
- For ARM / Jetson / embedded builds:
```bash

sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
sudo apt install qemu-system-arm qemu-system-misc qemu-system-aarch64

```