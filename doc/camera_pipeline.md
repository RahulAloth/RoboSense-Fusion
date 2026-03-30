# Camera Pipeline Setup (ROS2 Jazzy + OpenCV + Ubuntu 24.04)
## Aim
- Establish a stable, real‑time camera feed inside ROS2 Jazzy using OpenCV, ensuring compatibility with Ubuntu 24.04’s updated video stack (V4L2, GStreamer, MJPEG).

### Pre requistics
##### Initial Setup Completed
- Installed ROS2 Jazzy
- Created a custom ROS2 package: camera_node
- Implemented a multi‑camera C++ node using:
    - rclcpp
    - sensor_msgs/msg/Image
    - cv_bridge
    - opencv2

- Built the workspace successfully using:
```
	colcon build --symlink-install
```
##### Verify Camera Hardware

- Confirm the camera works perfectly at the OS level:
````
ffplay /dev/video0
````
- Live video feed visible
- Confirms hardware + drivers + permissions are correct
````
v4l2-ctl --list-formats-ext
````
- Camera supports MJPEG at all resolutions
- Camera supports YUYV at lower FPS
- Ideal for OpenCV

#### Test Open CV in Python:

````
cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M','J','P','G'))

````
- Result
````
Opened: True
Empty: False
````
#### Notes:
- Ubuntu 24.04 ships OpenCV compiled with GStreamer as the default backend.
- Inside ROS2, OpenCV may  silently fall back to GStreamer, and it may cause:
	- empty frames
	- pipeline errors
	- backend mismatch
	- MJPEG decoding failures
- So we may have to disable openCV globally. 

````
setenv("OPENCV_VIDEOIO_PRIORITY_GSTREAMER", "0", 1);
setenv("OPENCV_VIDEOIO_PRIORITY_V4L2", "1000", 1);
````
- Force V4L2 backend
 
 ````
 cv::VideoCapture cap(id, cv::CAP_V4L2);
 ````
- Forced MJPEG (stable for your camera):

`````
cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
`````
-  Set supported resolution + FPS:

````
cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
cap.set(cv::CAP_PROP_FPS, fps_);
````

- Add debug print to confirm frame size

````
if (frame.empty()) {
    RCLCPP_WARN(this->get_logger(), "Empty frame from camera %d", id);
    continue;
}

RCLCPP_DEBUG(this->get_logger(), "Camera %d frame: %d x %d", id, frame.cols, frame.rows);

`````
#### Build and test

`````
colcon build --symlink-install
source install/setup.bash
ros2 run camera_node camera_node --ros-args -p camera_ids:="[0]" -p fps:=30

`````
#### Verify

`````
rqt
Plugins → Visualization → Image View
Select: /camera0/image_raw
`````

##### Why is OpenCV using GStreamer?

- Ubuntu 24.04 ships OpenCV compiled with GStreamer as the default backend .
- Even if you request V4L2, OpenCV sometimes falls back to GStreamer unless you explicitly disable it .
- So as a fix, to force OpenCV to use V4L2 and disable GStreamer .

- We need to do two things:
	- ✅ Fix 1 — Force V4L2 backend with API preference
Replace your capture line with:

````
cv::VideoCapture cap(id, cv::CAP_V4L2);
cap.set(cv::CAP_PROP_BACKEND, cv::CAP_V4L2);
````

# 2. Dual‑Camera ROS2 System
- A complete record of how the two‑camera system was built, configured, and prepared for stereo synchronization.
- Check the branch : 
`````
Repo : git@github.com:RahulAloth/RoboSense-Fusion.git
Branch name : dual_camera_mode
`````
## Architect

````
+---------------------------+
|  /camera_left/image_raw   |
+---------------------------+
             \
              \ (PTP timestamp)
               \
+---------------------------+
|  TimeSynchronizer         |
+---------------------------+
               /
              / (PTP timestamp)
             /
+---------------------------+
|  /camera_right/image_raw  |
+---------------------------+

````

## Project Overview

- This project implements a dual‑camera stereo system for a kiosusing ROS 2 Jazzy.
- The system includes:
    - Multiple ROS2 packages
    - A camera driver
    - Multi‑camera support
    - Stable device naming via udev
    - Workspace automation
    - VSCode integration
    - Verified camera streaming
    - A dual‑camera ROS2 node
    - Preparation for PTP‑synchronized stereo processing
- This document consolidates the entire development history into a clean, structured reference.

### Development Tools & VSCode Integration
- Install ROS dev tools

````
sudo apt install ros-dev-tools

````
- Generate VSCode config
````
ros2 run ros_dev_tools generate_vscode_config

````
- Create VSCode folder
````
mkdir -p .vscode
touch .vscode/c_cpp_properties.json


````
### Camera Driver Setup
- Install dependencies
````
sudo apt install ros-jazzy-cv-bridge ros-jazzy-vision-opencv
sudo apt install v4l-utils ffmpeg
````
- Check connected cameras
````
ls /dev/video*
v4l2-ctl --list-devices
````
- Check supported formats

````
v4l2-ctl --device=/dev/video0 --list-formats-ext
v4l2-ctl --device=/dev/video1 --list-formats-ext
````

- Test camera feed
````
ffplay /dev/video0
ffplay /dev/video1
````

### Creating Stable Camera Names (udev Rules)

- USB camera device numbers change on reboot.
- I fixed this by creating persistent names.
- Get serial numbers

````
udevadm info --query=all --name=/dev/video1 | grep SERIAL
udevadm info --query=all --name=/dev/video3 | grep SERIAL

````
- Create rule

````
sudo nano /etc/udev/rules.d/99-usb-cameras.rules
````
Example:

````
SUBSYSTEM=="video4linux", ATTRS{serial}=="123456", SYMLINK+="camera_left"
SUBSYSTEM=="video4linux", ATTRS{serial}=="789012", SYMLINK+="camera_right"
````

- Reload rules
````
sudo udevadm control --reload-rules
sudo udevadm trigger
````

- Verify
````
ls -l /dev/camera*
````

Now we have:
````
/dev/camera_left
/dev/camera_right

````
### Building & Testing the Camera Node

Built the workspace while developing using following command:
````
rm -rf build install log
colcon build --symlink-install
source install/setup.bash

````
- Run single‑camera node
````
ros2 run camera_node camera_node --ros-args -p camera_ids:="[0]" -p fps:=30
````

## Multi‑Camera Node Development

- Create a dedicated package:
````
ros2 pkg create --build-type ament_cmake multi_camera_pkg

````
- Use code change from the new branch for dual camera. 
````
colcon build --symlink-install
source install/setup.bash
ros2 run multi_camera_pkg dual_camera_node
````

- Confirm the following 
	- Both cameras open correctly
    - Both publish /camera_left/image_raw and /camera_right/image_raw
    - RViz2 can visualize both streams


## PTP‑Synchronized Stereo Subscriber
- Defer

## Camera Setup & Debugging
- List available video devices
`````
v4l2-ctl --list-devices
ls /dev/video*
`````
- Inspect camera formats
`````
v4l2-ctl --device=/dev/video0 --list-formats-ext
v4l2-ctl --device=/dev/video1 --list-formats-ext
`````
- Test camera streams
`````
ffplay /dev/video0
ffplay /dev/video1
`````

- Install required packages
`````
sudo apt install v4l-utils ffmpeg ros-jazzy-cv-bridge ros-jazzy-vision-opencv

`````

- Persistent Camera Naming (udev rules)
- To avoid /dev/videoX changing on reboot, custom udev rules were created:
`````
sudo nano /etc/udev/rules.d/99-usb-cameras.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
ls -l /dev/camera*
`````
- Camera serial numbers were extracted using:
`````
udevadm info --query=all --name=/dev/video1 | grep SERIAL
`````
### ROS2 Camera Nodes
- Single camera
`````
ros2 run camera_node camera_node --ros-args -p camera_ids:="[0]" -p fps:=30
`````
- Dual camera
`````
ros2 run multi_camera_pkg dual_camera_node
`````
- Stereo pointcloud
`````
ros2 run stereo_pointcloud stereo_pointcloud_node
`````
- Preprocess + Detection
`````
ros2 run camera_preprocess camera_preprocess_node
ros2 run camera_detection camera_detection_node
`````
## PTP / gPTP Synchronization Setup
- Check NIC PTP capabilities
`````
ethtool -T eno1
`````

- Start PTP master
`````
sudo ptp4l -i eno1 -m -S
`````

- Synchronize system clock to PHC

`````
sudo phc2sys -s eno1 -w

`````
## ROS2 Nodes for Time Sync

- Stereo PTP Sync Node
`````
ros2 run stereo_ptp_sync_node stereo_ptp_sync_node
`````
- This node:
    - Reads camera tick timestamps
    - Reads PTP system time
    - Computes offset = PTP − tick
    - Converts camera timestamps into PTP domain
    - Logs synchronized timestamps

## Visualization Tools

- Image viewer
`````
rqt_image_view
`````
- 3D visualization

`````
rviz2
`````
- Used to verify:
    - Stereo images
    - Point clouds
    - TF frames
    - Fusion alignment
## System Setup Guide

- To set up the Stereo + LiDAR Fusion System, begin by preparing a clean ROS 2 workspace, installing required dependencies such as cv-bridge, vision-opencv, v4l-utils, and ffmpeg, and rebuilding the project with colcon. Configure and verify all connected cameras using v4l2-ctl, then create persistent device names via custom udev rules to ensure stable /dev/camera* mappings across reboots. Test camera streams with ffplay and validate image topics using rqt_image_view. For LiDAR and time synchronization, enable hardware timestamping on the Ethernet interface, start the PTP master using ptp4l, and synchronize the system clock to the PHC using phc2sys until stable lock is achieved. With the environment ready, launch the ROS 2 camera nodes, stereo processing nodes, and LiDAR drivers, then verify point clouds and image streams in RViz. This setup establishes a fully synchronized environment where camera tick timestamps and LiDAR PTP timestamps can be aligned into a unified time domain for reliable fusion.

- Note:
- I dont have any lidar at the moment. So I created a dummy point cloud using Camera sterio disparity calculation and added ptp time stamp there. It is enough to test the ptp synch and develop integration logic. 
