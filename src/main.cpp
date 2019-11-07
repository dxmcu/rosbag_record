#include"rosbag.h"
// #include <thread>

int main(int argc, char **argv)
{
    ros::init(argc,argv,"rosbag_pkg_node");
    ros::NodeHandle node;
    Rosbag my_rosbag;//创建一个类对象

    ros::Timer timer_1 = node.createTimer(ros::Duration(10),&Rosbag::keep_bagsdir_security,&my_rosbag);//安全检查
    // ros::Timer timer_2 = node.createTimer(ros::Duration(5),&Rosbag::a,&my_rosbag);//安全检查

    ros::AsyncSpinner spinner(10);
    spinner.start();
    ros::waitForShutdown();
    return 0;
}