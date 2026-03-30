#include <rclcpp/rclcpp.hpp>
#include "dual_camera_node.cpp"

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    rclcpp::NodeOptions options;
    options.use_intra_process_comms(true);   // ZERO-COPY ENABLED

    auto node = std::make_shared<DualCameraNode>(options);

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
