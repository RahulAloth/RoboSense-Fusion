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
