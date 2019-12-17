#include "rosbag.h"
#include <thread>
#include <fstream>
using namespace std;

void record_auto_local_path()
{
    //首先需要检查这个文件是否存在，如果存在需要检查文件大小
    char *pwd = getenv("HOME");
    string PATH_DIR;
    PATH_DIR += (std::string)pwd;
    PATH_DIR += "/rosbag/auto_local_path.txt";
    // delete pwd;

    fstream _file;
    _file.open(PATH_DIR, ios::in);
    if (_file) //如果文件被创建
    {
        // std::cout << PATH_DIR << " 已经被创建" << std::endl;
        //检查文件大小

        struct stat statbuf;
        if (stat(PATH_DIR.c_str(), &statbuf) == 0) //0表示获取成功
        {
            double files_size_mb = ((double)statbuf.st_size / 1024 / 1024); //MB
            // cout << files_size_mb << endl;
            if (files_size_mb > 500) //设置上限500M
                system(("rm " + PATH_DIR).c_str());
        }
    }

    //首先记录路径的数据
    system("rostopic echo /auto_local_path >> ~/rosbag/auto_local_path.txt");
}

void record_pre_auto_local_path()
{
    //首先需要检查这个文件是否存在，如果存在需要检查文件大小
    char *pwd = getenv("HOME");
    string PATH_DIR;
    PATH_DIR += (std::string)pwd;
    PATH_DIR += "/rosbag/pre_auto_local_path.txt";
    // delete pwd;

    fstream _file;
    _file.open(PATH_DIR, ios::in);
    if (_file) //如果文件被创建
    {
        // std::cout << PATH_DIR << " 已经被创建" << std::endl;
        //检查文件大小

        struct stat statbuf;
        if (stat(PATH_DIR.c_str(), &statbuf) == 0) //0表示获取成功
        {
            double files_size_mb = ((double)statbuf.st_size / 1024 / 1024); //MB
            // cout << files_size_mb << endl;
            if (files_size_mb > 500) //设置上限500M
                system(("rm " + PATH_DIR).c_str());
        }
    }

    //首先记录路径的数据
    system("rostopic echo /planning/pre_auto_path >> ~/rosbag/pre_auto_local_path.txt"); //如果没有，会自动创建文件
}

void record_base_pose()
{
    //首先需要检查这个文件是否存在，如果存在需要检查文件大小
    char *pwd = getenv("HOME");
    string PATH_DIR;
    PATH_DIR += (std::string)pwd;
    PATH_DIR += "/rosbag/base_pose.txt";
    // delete pwd;

    fstream _file;
    _file.open(PATH_DIR, ios::in);
    if (_file) //如果文件被创建
    {
        // std::cout << PATH_DIR << " 已经被创建" << std::endl;
        //检查文件大小

        struct stat statbuf;
        if (stat(PATH_DIR.c_str(), &statbuf) == 0) //0表示获取成功
        {
            double files_size_mb = ((double)statbuf.st_size / 1024 / 1024); //MB
            // cout << files_size_mb << endl;
            if (files_size_mb > 100) //设置上限100M
                system(("rm " + PATH_DIR).c_str());
        }
    }

    system("rostopic echo /cti/robot_config/base_link_pose >> ~/rosbag/base_pose.txt");
}




int main(int argc, char **argv)
{
    ros::init(argc, argv, "rosbag_pkg_node");
    ros::NodeHandle node;
    Rosbag my_rosbag; //创建一个类对象

    ros::Timer timer_1 = node.createTimer(ros::Duration(60), &Rosbag::keep_bagsdir_security, &my_rosbag); //安全检查1分钟一次
    ros::Timer timer_2 = node.createTimer(ros::Duration(0.5), &Rosbag::button_record, &my_rosbag);        //button record
    ros::Timer timer_3 = node.createTimer(ros::Duration(0.5), &Rosbag::lost_record, &my_rosbag);          //button record

    // thread th_1(record_auto_local_path);
    // th_1.detach();
    // thread th_2(record_base_pose);
    // th_2.detach();
    // thread th_3(record_pre_auto_local_path);
    // th_3.detach();

    ros::AsyncSpinner spinner(10);
    spinner.start();
    ros::waitForShutdown();
    return 0;
}
