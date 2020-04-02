#include "rosbag.h"
#include <thread>
#include <fstream>
using namespace std;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "rosbag_pkg_node");
    ros::NodeHandle node;
    Rosbag my_rosbag; //创建一个类对象

    ros::Timer timer_1 = node.createTimer(ros::Duration(60), &Rosbag::keep_bagsdir_security, &my_rosbag); //安全检查1分钟一次
    ros::Timer timer_2 = node.createTimer(ros::Duration(0.5), &Rosbag::button_record, &my_rosbag);        //button record

    ros::AsyncSpinner spinner(10);
    spinner.start();
    ros::waitForShutdown();
    return 0;
}
