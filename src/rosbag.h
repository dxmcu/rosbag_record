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
	ros::Subscriber sub_button;
	void button_callback(const cti_msgs::RobotCmd &msg);

	void keep_bagsdir_security(const ros::TimerEvent &event);
	void button_record(const ros::TimerEvent &event);

	Rosbag();

private:
	string PATH_DIR;
	cti_msgs::ErrorCode last_error;
	void dir_check(const char *dir_str);
	std::vector<string> get_files_name(const char *dir_str);
	void cal_num_size(const char *dir_str, int &files_num, double &files_size_gb);

	bool record;//记录是否需要录包，一次性只允许触发一个录包

	//记录时间
	ros::Duration button_dur;
	ros::Time button_record_time;
	bool is_button_record;

};

#endif