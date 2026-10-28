# System Architecture and Design Diagrams

## 1. Overall ROS2 Jazzy Package Architecture

The robotic system is designed using modular ROS2 Jazzy packages, separating responsibilities across perception, localization, mapping, planning, control, and system integration. This modular architecture improves maintainability, scalability, and testability while leveraging native ROS2 communication mechanisms such as Topics, Services, Actions, and Parameters.

### Core Packages

- `robot_bringup`
  - Contains launch files, configuration files, parameter maps, and startup scripts required to initialize the complete robotic system.

- `robot_interfaces`
  - Defines custom ROS2 messages (`.msg`), services (`.srv`), and actions (`.action`) shared across packages.

- `robot_perception`
  - Processes camera, LiDAR, and IMU data.
  - Performs sensor fusion and obstacle detection.

- `robot_localization`
  - Maintains the robot pose estimate using odometry, IMU, GNSS, and sensor fusion algorithms.

- `robot_mapping`
  - Creates and updates occupancy grid maps or 3D environment representations.

- `robot_planning`
  - Computes global and local paths.
  - Performs obstacle avoidance and trajectory generation.

- `robot_control`
  - Implements motion control algorithms.
  - Generates actuator commands for the robot base.

---

## 2. High-Level ROS2 Package Relationship

```text
+---------------------------------------------------------+
|                    robot_bringup                        |
|      Launch Files • Configuration • Parameters          |
+----------------------------+----------------------------+
                             |
                             v

+---------------------------------------------------------+
|               ROS2 Communication Framework              |
|          Topics • Services • Actions • Parameters       |
+---------------------------------------------------------+
      |             |             |             |
      v             v             v             v

+-------------+ +-------------+ +-------------+ +-------------+
| Perception  | |Localization | |  Mapping    | |  Planning   |
+-------------+ +-------------+ +-------------+ +-------------+
      |               |               |               |
      +---------------+---------------+---------------+
                              |
                              v

                    +-------------------+
                    |      Control      |
                    +-------------------+
                              |
                              v

                    +-------------------+
                    | Motors / Hardware |
                    +-------------------+

                              ^
                              |
                    +-------------------+
                    |    Interfaces     |
                    +-------------------+
```

---

## 3. ROS2 Data Flow

```text
Camera ---------+
                |
LiDAR ----------+----> robot_perception
                |
IMU ------------+

                          |
                          v

                robot_localization

                          |
                          v

                    robot_mapping

                          |
                          v

                   robot_planning

                          |
                          v

                    robot_control

                          |
                          v

                     Actuators
```

---

## 4. Design Principles

- **Modularity** – Each package owns a single system responsibility.
- **Loose Coupling** – Components communicate exclusively through ROS2 interfaces.
- **Scalability** – New sensors and algorithms can be integrated with minimal changes.
- **Reusability** – Common interfaces are centralized in `robot_interfaces`.
- **Maintainability** – Isolated packages simplify testing and debugging.
- **Hardware Independence** – Simulation and physical hardware share identical ROS2 interfaces.
- **ROS2 Native Design** – Utilizes DDS-based communication, lifecycle nodes, and parameterized deployment.

---

## 5. Communication Architecture

```text
+---------------------------+
|     robot_perception      |
+---------------------------+
          |
          | Publish
          v

   /perception/status
   RobotStatus.msg

          |
          v

+---------------------------+
|      robot_planning       |
+---------------------------+
          |
          | Publish
          v

        /cmd_vel
       Twist.msg

          |
          v

+---------------------------+
|      robot_control        |
+---------------------------+
          |
          | Service
          v

  /set_operation_mode
       SetMode.srv
```
### Topic & Service Specifications

#### `/perception/status`

- **Type:** `robot_interfaces/msg/RobotStatus`
- **Rate:** `10 Hz`
- **Publisher:** `/obstacle_detector_node`
- **Subscriber:** `/motor_controller_node`
- **Description:** Publishes processed obstacle information, obstacle locations, safety margins, and environmental awareness status generated from LiDAR perception data.

---

#### `/sensor/scan`

- **Type:** `sensor_msgs/msg/LaserScan`
- **Rate:** `20 Hz`
- **Publisher:** LiDAR Sensor Driver
- **Subscriber:** `/obstacle_detector_node`
- **Description:** Carries raw LiDAR scan measurements used for obstacle detection, environment perception, and sensor fusion processing.

---

#### `/cmd_vel`

- **Type:** `geometry_msgs/msg/Twist`
- **Rate:** `50 Hz`
- **Publisher:** `/motor_controller_node`
- **Subscriber:** Robot Base Controller / Actuator Interface
- **Description:** Contains linear and angular velocity commands generated by the control subsystem for robot movement execution.

---

#### `/set_operation_mode`

- **Type:** `robot_interfaces/srv/SetMode`
- **Server:** `/motor_controller_node`
- **Description:** Allows external clients or supervisory systems to switch the robot's operating mode.

**Supported Modes**

| Mode | Description |
|--------|-------------|
| `MANUAL` | Direct operator control of the robot. |
| `AUTONOMOUS` | Fully autonomous navigation and obstacle avoidance. |
| `EMERGENCY_STOP` | Immediately halts all robot motion. |
| `STANDBY` | Robot remains powered but inactive. |

---

### Communication Summary

| Interface | Type | Frequency | Purpose |
|------------|------|------------|----------|
| `/sensor/scan` | `sensor_msgs/msg/LaserScan` | 20 Hz | Raw LiDAR sensor data |
| `/perception/status` | `robot_interfaces/msg/RobotStatus` | 10 Hz | Obstacle and safety information |
| `/cmd_vel` | `geometry_msgs/msg/Twist` | 50 Hz | Motion control commands |
| `/set_operation_mode` | `robot_interfaces/srv/SetMode` | On Request | Robot mode switching |


## 5. UML-Style ASCII Class Diagrams

The system is implemented using ROS2 node-based components that inherit from `rclcpp::Node`. Each node encapsulates its own publishers, subscribers, services, and internal processing logic while communicating through ROS2 interfaces.

### ObstacleDetectorNode

```text
+-----------------------------------------------------------+
|         ObstacleDetectorNode : rclcpp::Node              |
+-----------------------------------------------------------+
| - lidar_sub_ : Subscription<LaserScan>::SharedPtr        |
| - status_pub_ : Publisher<RobotStatus>::SharedPtr        |
| - current_safety_margin_ : double                        |
+-----------------------------------------------------------+
| + ObstacleDetectorNode(options : NodeOptions)            |
| - scanCallback(msg : LaserScan::SharedPtr) : void        |
| - calculateSafetyMargin() : double                       |
| - detectObstacles() : void                               |
| - publishStatus() : void                                 |
+-----------------------------------------------------------+
```

**Responsibilities**

- Subscribe to LiDAR sensor data.
- Detect nearby obstacles.
- Calculate safe operating distance.
- Publish robot perception status.
- Provide processed environmental awareness information.

---

### MotorControllerNode

```text
+-----------------------------------------------------------+
|          MotorControllerNode : rclcpp::Node              |
+-----------------------------------------------------------+
| - status_sub_ : Subscription<RobotStatus>::SharedPtr     |
| - cmd_vel_pub_ : Publisher<Twist>::SharedPtr             |
| - mode_service_ : Service<SetMode>::SharedPtr            |
| - active_mode_ : std::string                             |
+-----------------------------------------------------------+
| + MotorControllerNode(options : NodeOptions)             |
| - statusCallback(msg : RobotStatus::SharedPtr) : void    |
| - handleModeChange(                                      |
|     req : SetMode::Request,                              |
|     res : SetMode::Response) : void                      |
| - generateVelocityCommand() : Twist                      |
| - executeControlLoop() : void                            |
+-----------------------------------------------------------+
```

**Responsibilities**

- Subscribe to perception status updates.
- Generate robot motion commands.
- Publish velocity commands to actuators.
- Manage robot operating modes.
- Support emergency stop and safety operations.

---

### Class Relationship Overview

```text
                    +------------------+
                    |   rclcpp::Node   |
                    +------------------+
                             ^
                             |
          +------------------+------------------+
          |                                     |
          |                                     |
          v                                     v

+--------------------------+      +--------------------------+
|   ObstacleDetectorNode   |      |    MotorControllerNode  |
+--------------------------+      +--------------------------+
| Sub: /sensor/scan        |      | Sub: /perception/status |
| Pub: /perception/status  |----->| Pub: /cmd_vel           |
| Safety calculations      |      | Service: SetMode        |
+--------------------------+      +--------------------------+
```

### ROS2 Component Relationships

```text
LaserScan
    |
    v
+--------------------------+
|   ObstacleDetectorNode   |
+--------------------------+
            |
            | RobotStatus
            v
+--------------------------+
|   MotorControllerNode    |
+--------------------------+
            |
            | Twist
            v
       Robot Base

External Client
        |
        | SetMode.srv
        v
+--------------------------+
|   MotorControllerNode    |
+--------------------------+
```
