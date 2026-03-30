#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>

using sensor_msgs::msg::Image;

class StereoPTPSyncNode : public rclcpp::Node
{
public:
    StereoPTPSyncNode() : Node("stereo_ptp_sync_node")
    {
        // Subscribe to left and right camera topics
        left_sub_.subscribe(this, "/camera_left/image_raw");
        right_sub_.subscribe(this, "/camera_right/image_raw");

        // Synchronizer with queue size 10
        sync_ = std::make_shared<Sync>(SyncPolicy(10), left_sub_, right_sub_);
        // Register callback
        sync_->registerCallback(
            std::bind(&StereoPTPSyncNode::syncCallback, this,
                      std::placeholders::_1,
                      std::placeholders::_2));

        RCLCPP_INFO(this->get_logger(), "Stereo PTP Sync Node started.");
    }

private:
    void syncCallback(const Image::ConstSharedPtr &left,
                      const Image::ConstSharedPtr &right)
    {
        // 1. Camera tick time
        double tick_left = left->header.stamp.sec + left->header.stamp.nanosec * 1e-9;
        double tick_right = right->header.stamp.sec + right->header.stamp.nanosec * 1e-9;
        // 1. Camera tick time
        

        // 2. PTP time at this moment
        double ptp = ptp_clock.now().seconds(); // already includes nanoseconds

        // 3. Compute offset
        double tick_avg = 0.5 * (tick_left + tick_right);
        double offset = ptp - tick_avg;
        
        RCLCPP_INFO(this->get_logger(),
                    "ptp synch offset: %.9f ",
                    offset);
        // 4. For doing fusion,
        //    we can either use the original tick timestamps (tick_left, tick_right) or the PTP-synced timestamps (tick_left + offset, tick_right + offset).
        //    The PTP-synced timestamps will be more accurate for fusion, especially when there are variable delays in the system. The original tick timestamps may be more accurate for debugging and analysis of the camera timing itself. In practice, using the PTP-synced timestamps is recommended for
        //    better synchronization in fusion algorithms, while the original tick timestamps can be logged for reference and debugging purposes.
        RCLCPP_INFO(this->get_logger(),
                    "Camera timestamps: Left: %.9f   Right: %.9f",
                    tick_left, tick_right);
        RCLCPP_INFO(this->get_logger(),
                    "PTP-synced Camera timestamps: Left: %.9f   Right: %.9f",
                    tick_left + offset, tick_right + offset);
    }

    // Subscribers
    message_filters::Subscriber<Image> left_sub_;
    message_filters::Subscriber<Image> right_sub_;

    // Synchronizer
    using SyncPolicy = message_filters::sync_policies::ApproximateTime<Image, Image>;
    using Sync       = message_filters::Synchronizer<SyncPolicy>;
    std::shared_ptr<Sync> sync_;
    rclcpp::TimerBase::SharedPtr timer;
    rclcpp::Clock ptp_clock{RCL_SYSTEM_TIME};
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StereoPTPSyncNode>());
    rclcpp::shutdown();
    return 0;
}
