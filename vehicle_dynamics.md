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

## 6. Custom ROS2 Message Definitions

The `vehicle_msgs` package contains all custom ROS2 messages, services, and action definitions shared between simulation, vehicle dynamics, control, and monitoring components.

### Package Structure

```text
vehicle_msgs/
├── CMakeLists.txt
├── package.xml
├── msg/
│   ├── VehicleState.msg
│   ├── VehicleCommand.msg
│   └── VehicleStatus.msg
├── srv/
│   └── SetDriveMode.srv
└── action/
    └── ExecuteTrajectory.action
```

### VehicleState.msg

Represents the current vehicle motion state estimated by the dynamics model.

```text
std_msgs/Header header

float64 x
float64 y
float64 yaw

float64 velocity
float64 steering_angle

float64 acceleration
float64 yaw_rate
```

### VehicleCommand.msg

Represents driver or controller commands.

```text
std_msgs/Header header

float64 target_velocity
float64 target_acceleration
float64 steering_angle
```

### VehicleStatus.msg

Provides health and state information.

```text
std_msgs/Header header

bool autonomous_enabled
bool emergency_stop

string current_mode
float64 battery_level
```

### SetDriveMode.srv

```text
string requested_mode
---
bool success
string message
```

Supported modes:

- MANUAL
- AUTONOMOUS
- REMOTE
- EMERGENCY_STOP
## 7. Vehicle Dynamics C++ Simulation Node

### Overview

The `vehicle_dynamics` package implements a kinematic bicycle model that simulates vehicle motion based on steering and velocity commands.

The node subscribes to vehicle commands and publishes the resulting vehicle state at a fixed simulation rate.

### Package Structure

```text
vehicle_dynamics/
├── CMakeLists.txt
├── package.xml
├── include/
│   └── vehicle_dynamics/
│       └── vehicle_dynamics_node.hpp
├── src/
│   ├── vehicle_dynamics_node.cpp
│   └── main.cpp
├── launch/
│   └── vehicle_dynamics.launch.py
└── config/
    └── simulation_params.yaml
```

### Node Interfaces

#### Subscribed Topics

| Topic | Type |
|---------|---------|
| `/vehicle/command` | `vehicle_msgs/msg/VehicleCommand` |

#### Published Topics

| Topic | Type |
|---------|---------|
| `/vehicle/state` | `vehicle_msgs/msg/VehicleState` |

---

### Kinematic Bicycle Model

Vehicle state:

```text
x
y
yaw
velocity
```

Control inputs:

```text
acceleration
steering_angle
```

State Update Equations

```text
x_dot = v * cos(yaw)

y_dot = v * sin(yaw)

yaw_dot = (v / L) * tan(delta)

v_dot = acceleration
```

Where:

```text
L      = Wheelbase
v      = Vehicle Velocity
delta  = Steering Angle
yaw    = Vehicle Heading
```

---

### UML Class Diagram

```text
+--------------------------------------------------------+
| VehicleDynamicsNode : rclcpp::Node                     |
+--------------------------------------------------------+
| - state_pub_                                            |
| - command_sub_                                          |
| - timer_                                                |
| - wheelbase_                                            |
| - sim_step_                                             |
| - state_                                                |
+--------------------------------------------------------+
| + VehicleDynamicsNode()                                 |
| - commandCallback()                                     |
| - updateVehicleModel()                                  |
| - publishState()                                        |
+--------------------------------------------------------+
```

---

### Data Flow

```text
Vehicle Controller
        |
        |
        v
+----------------------+
| VehicleCommand.msg   |
+----------------------+
        |
        v

+----------------------+
| VehicleDynamicsNode  |
+----------------------+

        |
        v

+----------------------+
| VehicleState.msg     |
+----------------------+

        |
        v

Visualization / Logging
Control Algorithms
Remote Monitoring
```

---

### Core Simulation Loop

```cpp
void VehicleDynamicsNode::updateVehicleModel()
{
    state_.x += state_.velocity * cos(state_.yaw) * dt_;

    state_.y += state_.velocity * sin(state_.yaw) * dt_;

    state_.yaw +=
        (state_.velocity / wheelbase_) *
        tan(current_steering_angle_) *
        dt_;

    state_.velocity +=
        current_acceleration_ *
        dt_;
}
```

### Default Parameters

```yaml
vehicle_dynamics:
  ros__parameters:
    wheelbase: 2.8
    simulation_rate_hz: 100.0
    publish_rate_hz: 50.0
```
