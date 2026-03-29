#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>
#include <cstdlib>

/**
 * @brief MultiCameraNode Single point of entry for creating publishers and subscribers for multiple cameras.
 * 
 * This node demonstrates how to handle multiple cameras using OpenCV's VideoCapture with specific backend 
 * and codec settings, and publish the frames as ROS2 messages.
 */
class MultiCameraNode : public rclcpp::Node
{
public:
    MultiCameraNode() : Node("multi_camera_node")
    {
        // Disable GStreamer globally (critical for Ubuntu 24.04)
        setenv("OPENCV_VIDEOIO_PRIORITY_GSTREAMER", "0", 1);
        setenv("OPENCV_VIDEOIO_PRIORITY_V4L2", "1000", 1);

        // Declare parameters
        this->declare_parameter<std::vector<int>>("camera_ids", {0});
        this->declare_parameter<int>("fps", 30);

        camera_ids_ = this->get_parameter("camera_ids").as_integer_array();
        fps_ = this->get_parameter("fps").as_int();

        RCLCPP_INFO(this->get_logger(), "Starting MultiCameraNode with %zu cameras", camera_ids_.size());

        // Initialize cameras
        for (int id : camera_ids_) {
            // Force V4L2 backend
            cv::VideoCapture cap(id, cv::CAP_V4L2);

            if (!cap.isOpened()) {
                RCLCPP_ERROR(this->get_logger(), "Failed to open camera %d", id);
                continue;
            }

            // Force MJPEG
            cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));

            // Resolution + FPS
            cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
            cap.set(cv::CAP_PROP_FPS, fps_);

            // Debug info
            RCLCPP_INFO(this->get_logger(), "Camera %d backend: %s", id, cap.getBackendName().c_str());
            RCLCPP_INFO(this->get_logger(), "Camera %d FOURCC: %d", id, (int)cap.get(cv::CAP_PROP_FOURCC));
            RCLCPP_INFO(this->get_logger(), "Camera %d resolution: %.0fx%.0f",
                        cap.get(cv::CAP_PROP_FRAME_WIDTH),
                        cap.get(cv::CAP_PROP_FRAME_HEIGHT));

            cameras_[id] = std::move(cap);

            // Create publisher
            std::string topic = "/camera" + std::to_string(id) + "/image_raw";
            publishers_[id] = this->create_publisher<sensor_msgs::msg::Image>(topic, 10);

            RCLCPP_INFO(this->get_logger(), "Publishing camera %d on %s", id, topic.c_str());
        }

        // Timer for publishing frames
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000 / fps_),
            std::bind(&MultiCameraNode::capture_and_publish, this));
    }

private:
    void capture_and_publish()
    {
        for (auto &pair : cameras_) {
            int id = pair.first;
            cv::VideoCapture &cap = pair.second;

            cv::Mat frame;
            cap >> frame;

            if (frame.empty()) {
                RCLCPP_WARN(this->get_logger(), "Empty frame from camera %d", id);
                continue;
            }

            // Debug frame size
            RCLCPP_DEBUG(this->get_logger(), "Camera %d frame: %d x %d", id, frame.cols, frame.rows);

            auto msg = cv_bridge::CvImage(
                std_msgs::msg::Header(),
                "bgr8",
                frame
            ).toImageMsg();

            msg->header.stamp = this->now();
            msg->header.frame_id = "camera" + std::to_string(id);

            publishers_[id]->publish(*msg);
        }
    }

    std::vector<int64_t> camera_ids_;
    int fps_;

    std::map<int, cv::VideoCapture> cameras_;
    std::map<int, rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr> publishers_;

    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MultiCameraNode>());
    rclcpp::shutdown();
    return 0;
}
