#ifndef ROSBAG_H
#define ROSBAG_H

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include "cti_msgs/AutoPath.h"
#include "cti_msgs/TargetPose.h"
#include "cti_msgs/ErrorCode.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string>
//#include <sys/types.h>

#define BAGS_NUM_UP 90
#define BAGS_SIZE_UP 15 //gb

using namespace std;

class Rosbag
{
public:
	ros::NodeHandle n;

	//回调函数
	ros::Subscriber sub_auto_local_path;
	ros::Subscriber sub_target_pose;
	ros::Subscriber sub_astar_error;
	void auto_local_path_callback(const cti_msgs::AutoPath &msg);
	void target_pose_callback(const cti_msgs::TargetPose &msg);
	void astar_error_callback(const cti_msgs::ErrorCode &msg);
	void keep_bagsdir_security(const ros::TimerEvent &event);
	// void a(const ros::TimerEvent &event)
	// {
	// 	system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR).c_str());
	// }

	Rosbag();

private:
    string PATH_DIR;
	cti_msgs::ErrorCode last_error;
	void dir_check(const char *dir_str);
	std::vector<string> get_files_name(const char *dir_str);
	void cal_num_size(const char *dir_str, int &files_num, double &files_size_gb);
};

#endif