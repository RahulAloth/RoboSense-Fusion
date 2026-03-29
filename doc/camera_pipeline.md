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

