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
#include "cti_msgs/State.h"
#include "cti_msgs/RobotCmd.h"
//#include <sys/types.h>

#define BAGS_NUM_UP 120
#define BAGS_SIZE_UP 22 //gb

using namespace std;

class Rosbag
{
public:
	ros::NodeHandle n;

	//回调函数
	ros::Subscriber sub_auto_local_path;
	ros::Subscriber sub_target_pose;
	ros::Subscriber sub_astar_error;
	ros::Subscriber sub_state_error; //lost
	ros::Subscriber sub_button;
	void auto_local_path_callback(const cti_msgs::AutoPath &msg);
	void target_pose_callback(const cti_msgs::TargetPose &msg);
	void astar_error_callback(const cti_msgs::ErrorCode &msg);
	void state_error_callback(const cti_msgs::State &msg); //定位丢失的时候录包
	void button_callback(const cti_msgs::RobotCmd &msg);

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

	cti_msgs::State state_1, state_2, state_3; //记录相邻三次的状态
};

#endif