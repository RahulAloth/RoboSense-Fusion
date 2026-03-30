# RoboSense‑Fusion Workspace Setup History  
*A clean chronological log of all steps performed during environment setup.*

---

## 1. Repository Cloning

```bash
git clone git@github.com:RahulAloth/RoboSense-Fusion.git
cd RoboSense-Fusion/
ls
```

## 2. Initialize ROS 2 Workspace Structure
- Created the standard ROS 2 workspace directories:
```bash
mkdir src
mkdir build
mkdir install
mkdir log
```
## 3. Create ROS 2 Packages
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
 
## 4. Build the Workspace
- Returned to workspace root:

 ```bash
cd ~/AiStätt/RoboSense-Fusion
colcon build --symlink-install
```

## 5. Source the Workspace
- Initial sourcing:
 
 ```bash
source ~/AiStätt/RoboSense-Fusion/install/setup.bash
```
- Added to .bashrc:
 
 ```bash
echo "source ~/AiStätt/RoboSense-Fusion/install/setup.bash" >> ~/.bashrc
source ~/.bashrc

```
## 7. Verify ROS 2 Environment
 ```bash
echo $ROS_DISTRO


```


