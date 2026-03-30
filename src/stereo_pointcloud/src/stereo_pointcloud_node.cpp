#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud.hpp>
#include <geometry_msgs/msg/point32.hpp>

#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

#include <tf2_ros/static_transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

using sensor_msgs::msg::Image;

class StereoPointCloudNode : public rclcpp::Node
{
public:
    StereoPointCloudNode() : Node("stereo_pointcloud_node")
    {
        // Publisher
        pc_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud>(
            "/stereo/points", 10);

        // Create static TF broadcaster ONCE
        static_tf_broadcaster_ =
            std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

        // Publish static transform once
        geometry_msgs::msg::TransformStamped t;
        // t.header.stamp = this->now();        
        t.header.stamp = ptp_clock.now();   // <-- PTP timestamp
        t.header.frame_id = "map";
        t.child_frame_id = "camera_left_frame";
        t.transform.translation.x = 0.0;
        t.transform.translation.y = 0.0;
        t.transform.translation.z = 0.0;
        t.transform.rotation.x = 0.0;
        t.transform.rotation.y = 0.0;
        t.transform.rotation.z = 0.0;
        t.transform.rotation.w = 1.0;

        // Publish static transform for right camera too
        geometry_msgs::msg::TransformStamped t2;
        // t2.header.stamp = this->now();
        t2.header.stamp = ptp_clock.now();   // <-- PTP timestamp
        t2.header.frame_id = "map";
        t2.child_frame_id = "camera_right_frame";
        t2.transform.translation.x = 0.0;
        t2.transform.translation.y = 0.0;
        t2.transform.translation.z = 0.0;
        t2.transform.rotation.x = 0.0;
        t2.transform.rotation.y = 0.0;
        t2.transform.rotation.z = 0.0;
        t2.transform.rotation.w = 1.0;

        static_tf_broadcaster_->sendTransform(t);
        static_tf_broadcaster_->sendTransform(t2);

        // Subscribers
        left_sub_.subscribe(this, "/camera_left/image_raw");
        right_sub_.subscribe(this, "/camera_right/image_raw");

        sync_ = std::make_shared<Sync>(SyncPolicy(10), left_sub_, right_sub_);
        sync_->registerCallback(
            std::bind(&StereoPointCloudNode::callback, this,
                      std::placeholders::_1,
                      std::placeholders::_2));

        RCLCPP_INFO(this->get_logger(), "Stereo PointCloud Node started.");
    }

private:
    void callback(const Image::ConstSharedPtr &left_msg,
                  const Image::ConstSharedPtr &right_msg)
    {
        sensor_msgs::msg::PointCloud cloud;
        cloud.header = left_msg->header;   // frame_id = camera_left_frame
        cloud.header = right_msg->header;   // frame_id = camera_left_frame
        cloud.header.stamp = ptp_clock.now();   // <-- PTP timestamp
        int num_points = 300;  // small, fast, visible
        cloud.points.resize(num_points);
        RCLCPP_INFO(this->get_logger(),
                    "PointCloud header timestamp: %.9f ",
                    cloud.header.stamp.sec + cloud.header.stamp.nanosec * 1e-9);    
        
        for (int i = 0; i < num_points; i++)
        {
            geometry_msgs::msg::Point32 p;
            p.x = static_cast<float>(rand() % 5);
            p.y = static_cast<float>(rand() % 5);
            p.z = static_cast<float>(rand() % 5);
            cloud.points[i] = p;
        }

        pc_pub_->publish(cloud);
    }

    // TF broadcaster
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_broadcaster_;

    // Subscribers
    message_filters::Subscriber<Image> left_sub_;
    message_filters::Subscriber<Image> right_sub_;

    using SyncPolicy = message_filters::sync_policies::ApproximateTime<Image, Image>;
    using Sync = message_filters::Synchronizer<SyncPolicy>;
    std::shared_ptr<Sync> sync_;

    // Publisher
    rclcpp::Publisher<sensor_msgs::msg::PointCloud>::SharedPtr pc_pub_;

    // PTP clock
    rclcpp::Clock ptp_clock{RCL_SYSTEM_TIME};
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StereoPointCloudNode>());
    rclcpp::shutdown();
    return 0;
}
