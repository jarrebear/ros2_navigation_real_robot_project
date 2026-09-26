#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <rclcpp/rclcpp.hpp>

#include "custom_interfaces/srv/my_service_message.hpp"
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>

struct Spot {
  std::string label;

  double x;
  double y;
  double z;

  double qx;
  double qy;
  double qz;
  double qw;
};

class SpotRecorderService : public rclcpp::Node {
public:
  using Message = custom_interfaces::srv::MyServiceMessage;

  SpotRecorderService() : Node("spot_recorder") {

    spot_service_ = this->create_service<Message>(
        "/save_spot", std::bind(&SpotRecorderService::recorder_callback, this,
                                std::placeholders::_1, std::placeholders::_2));

    // rclcpp::QoS qos(rclcpp::KeepLast(10));

    // qos.reliable();
    // qos.transient_local();
    // qos.lifespan(RMW_DURATION_INFINITE);
    // qos.deadline(RMW_DURATION_INFINITE);
    // qos.liveliness(RMW_QOS_POLICY_LIVELINESS_AUTOMATIC);
    // qos.liveliness_lease_duration(RMW_DURATION_INFINITE);

    pose_subscriber_ = this->create_subscription<
        geometry_msgs::msg::PoseWithCovarianceStamped>(
        "/amcl_pose", 10,
        std::bind(&SpotRecorderService::pose_callback, this,
                  std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Spot Recording Service Ready...");
  }

private:
  void pose_callback(
      const geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr msg) {

    current_pose_ = msg;
  }

  void recorder_callback(const std::shared_ptr<Message::Request> request,
                         std::shared_ptr<Message::Response> response) {

    RCLCPP_INFO(this->get_logger(), "Spot Recording Service Requested!");

    if (!current_pose_) {
      RCLCPP_WARN(this->get_logger(), "No pose received yet.");
      response->write_successful = false;
      response->message = "No pose received yet.";
      return;
    }

    if (request->label == "end") {
      bool save_successful = saveSpotsToFile(spots_);
      response->write_successful = save_successful;
      if (save_successful) {
        response->message = "File saved successfully.";
      } else {
        response->message = "Failed to save file.";
      }

      return;
    }
    Spot spot;

    spot.label = request->label;

    spot.x = current_pose_->pose.pose.position.x;
    spot.y = current_pose_->pose.pose.position.y;
    spot.z = current_pose_->pose.pose.position.z;

    spot.qx = current_pose_->pose.pose.orientation.x;
    spot.qy = current_pose_->pose.pose.orientation.y;
    spot.qz = current_pose_->pose.pose.orientation.z;
    spot.qw = current_pose_->pose.pose.orientation.w;

    spots_.push_back(spot);

    response->write_successful = false;
    response->message = "Spot successfully recorded, not saved in file";
  }

  bool saveSpotsToFile(const std::vector<Spot> &spots) {
    std::ofstream file(
        "/home/user/ros2_ws/src/project_localization/spots/spots.txt");

    if (!file.is_open()) {
      return false;
    }

    for (const auto &spot : spots) {
      file << "label: " << spot.label << "\n";

      file << "position:\n";
      file << "  x: " << spot.x << "\n";
      file << "  y: " << spot.y << "\n";
      file << "  z: " << spot.z << "\n";

      file << "orientation:\n";
      file << "  x: " << spot.qx << "\n";
      file << "  y: " << spot.qy << "\n";
      file << "  z: " << spot.qz << "\n";
      file << "  w: " << spot.qw << "\n";

      file << "\n";
    }

    file.close();

    return true;
  }

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr
      pose_subscriber_;
  rclcpp::Service<Message>::SharedPtr spot_service_;

  std::vector<Spot> spots_;

  geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr current_pose_;
};

int main(int argc, char **argv) {

  rclcpp::init(argc, argv);
  auto node = std::make_shared<SpotRecorderService>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}