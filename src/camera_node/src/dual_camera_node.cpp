#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class DualCameraNode : public rclcpp::Node
{
public:
    DualCameraNode(const rclcpp::NodeOptions & options)
    : Node("dual_camera_node", options)
    {
        // --- Force OpenCV to use V4L2 (Ubuntu 24.04 fix) ---
        setenv("OPENCV_VIDEOIO_PRIORITY_GSTREAMER", "0", 1);
        setenv("OPENCV_VIDEOIO_PRIORITY_V4L2", "1000", 1);

        // --- Publishers ---
        pub_left_  = create_publisher<sensor_msgs::msg::Image>("/camera_left/image_raw", 10);
        pub_right_ = create_publisher<sensor_msgs::msg::Image>("/camera_right/image_raw", 10);

        // --- Open cameras using numeric device IDs ---
        cap_left_.open(0, cv::CAP_V4L2);   // /dev/video2
        cap_right_.open(4, cv::CAP_V4L2);  // /dev/video3

        if (!cap_left_.isOpened()) {
            RCLCPP_ERROR(get_logger(), "Failed to open LEFT camera (/dev/video2)");
        }
        if (!cap_right_.isOpened()) {
            RCLCPP_ERROR(get_logger(), "Failed to open RIGHT camera (/dev/video3)");
        }
        if (!cap_left_.isOpened() || !cap_right_.isOpened()) {
            return;  // Stop initialization
        }

        // --- Force MJPEG for stable FPS ---
        cap_left_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
        cap_right_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));

        // --- Set resolution ---
        cap_left_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap_left_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        cap_right_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap_right_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        // --- Timer for ~30 FPS publishing ---
        timer_ = create_wall_timer(
            std::chrono::milliseconds(33),
            std::bind(&DualCameraNode::capture_and_publish, this)
        );

        RCLCPP_INFO(get_logger(), "DualCameraNode started successfully");
    }

private:
    void capture_and_publish()
    {
        cv::Mat frame_left, frame_right;

        // --- Capture frames ---
        if (!cap_left_.read(frame_left) || frame_left.empty()) {
            RCLCPP_WARN(get_logger(), "Left camera frame empty");
            return;
        }

        if (!cap_right_.read(frame_right) || frame_right.empty()) {
            RCLCPP_WARN(get_logger(), "Right camera frame empty");
            return;
        }

        // --- Convert to ROS2 messages ---
        auto msg_left  = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame_left).toImageMsg();
        auto msg_right = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame_right).toImageMsg();

        msg_left->header.stamp  = tick_count.now();
        msg_right->header.stamp = tick_count.now();

        msg_left->header.frame_id  = "camera_left_frame";
        msg_right->header.frame_id = "camera_right_frame";
        RCLCPP_INFO(this->get_logger(),
        "Camera frames captured. Left timestamp: %.9f   Right timestamp: %.9f",
                    msg_left->header.stamp.sec + msg_left->header.stamp.nanosec * 1e-9,
                    msg_right->header.stamp.sec + msg_right->header.stamp.nanosec * 1e-9);
        // --- Publish ---
        pub_left_->publish(*msg_left);
        pub_right_->publish(*msg_right);
    }

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_left_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr pub_right_;

    cv::VideoCapture cap_left_;
    cv::VideoCapture cap_right_;

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Clock tick_count{RCL_STEADY_TIME};
};
