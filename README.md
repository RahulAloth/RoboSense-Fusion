# RoboSense‑Fusion  
ROS2‑based Multi‑Sensor Perception Framework in Modern C++

RoboSense‑Fusion is a modular ROS2 perception engine written in Modern C++ (C++20). It targets indoor mobile robots operating in factory or warehouse environments and is designed to run both on a development PC and later on embedded SoCs such as NVIDIA Jetson, TI Jacinto, or NXP platforms.

The system provides a complete perception pipeline: radar, camera, vehicle dynamics, detection, tracking, and multi‑sensor fusion.

---

## 🚀 Core Goals

- Modern C++ implementation (C++17/20)
- ROS2‑native architecture (rclcpp, TF2, lifecycle nodes)
- Real‑time‑friendly design for embedded SoCs
- Modular sensor interfaces (radar, camera, dynamics)
- Extensible detection and fusion pipeline
- Clean separation of perception → detection → fusion

---

## 🧱 System Architecture

### Sensor Layer
- `radar_sim_node` — synthetic FMCW radar detections  
- `radar_tracker_node` — Modern C++ radar tracking algorithm  
- `camera_node` — USB/video camera publisher  
- `camera_preprocess_node` — image preprocessing  
- `vehicle_dynamics_node` — ego‑motion publisher

### Processing Layer
- Radar preprocessing and tracking  
- Camera preprocessing and detection  
- Ego‑motion integration

### Fusion Layer
- `fusion_node` — combines radar tracks, camera detections, and dynamics  
- Outputs fused objects for navigation and visualization

---

## 📡 ROS2 Topics

| Topic | Description |
|-------|-------------|
| `/radar/detections` | Raw radar detections |
| `/radar/tracks` | Tracked radar objects |
| `/camera/image_raw` | Raw camera frames |
| `/camera/preprocessed` | Preprocessed image data |
| `/camera/detections` | Camera‑based detections |
| `/vehicle/dynamics` | Ego‑motion data |
| `/fusion/objects` | Fused object list |

---

## 🧩 Message Definitions (Conceptual)

### `RadarDetection.msg`
- float32 range  
- float32 azimuth  
- float32 radial_velocity  
- float32 snr  

### `RadarTrack.msg`
- int32 id  
- float32 x  
- float32 y  
- float32 vx  
- float32 vy  
- float32 confidence  

### `CameraDetection.msg`
- int32 id  
- float32 x  
- float32 y  
- float32 width  
- float32 height  
- float32 confidence  

### `VehicleDynamics.msg`
- float32 speed  
- float32 yaw_rate  
- float32 steering_angle  

### `FusedObject.msg`
- int32 id  
- float32 x  
- float32 y  
- float32 vx  
- float32 vy  
- uint8 source_flags  

---

## 📁 Project Structure (C++‑oriented)

