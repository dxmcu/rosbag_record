#include "rosbag.h"

Rosbag::Rosbag()
{
    this->dir_check(PATH_DIR);
    this->sub_auto_local_path = this->n.subscribe("/auto_local_path", 1, &Rosbag::auto_local_path_callback, this);
    this->sub_target_pose = this->n.subscribe("/cti/move_controller/target_pose", 1, &Rosbag::target_pose_callback, this);
}

//收到astar路径的回调函数
void Rosbag::auto_local_path_callback(const cti_msgs::AutoPath &msg)
{
    if (msg.source == 2)
        system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "astar").c_str());
}

//收到target pose的回调函数
void Rosbag::target_pose_callback(const cti_msgs::TargetPose &msg)
{
    system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "target").c_str());
}

//检测计算机指定的目录是否存在，如果不存在，那么创建一个目录
void Rosbag::dir_check(const char *dir_str)
{
    if (opendir(dir_str) == NULL)
    {
        std::cout << "dir " << dir_str << " is not exist, and will be created." << std::endl;
        //创建文件夹
        if (0 != mkdir(dir_str, S_IRWXU))
        {
            std::cout << "create dir failed." << std::endl;
            return;
        }
        else
        {
            std::cout << "create dir succ." << std::endl;
            return;
        }
    }
    else
    {
        std::cout << "dir " << dir_str << " is already exist." << std::endl;
    }
}

//循环查看文件夹内文件数量，与大小，如果数量大小不满足要求，那么需要循环删除时间最早的数据，直到满足要求
//需要单独开一条线程
void Rosbag::keep_bagsdir_security(const ros::TimerEvent &event)
{
    const char *dir_str = PATH_DIR;
    //运行之前需要先查看这个目录是否存在，如果不存在，就直接退出
    if (opendir(dir_str) == NULL)
    {
        std::cout << "dir " << dir_str << " is not exist." << std::endl;
        return;
    }

    int files_num = 0;
    double files_size_gb = 0; //GB
    this->cal_num_size(dir_str, files_num, files_size_gb);

    // cout << "first num = " << files_num << std::endl;
    // cout << "first size = " << files_size_gb << " gb" << endl;

    while (files_num > BAGS_NUM_UP || files_size_gb > BAGS_SIZE_UP)
    {
        //删除时间最早的那一个文件，然后重新计算files_num与files_size_gb
        std::vector<string> files = this->get_files_name(dir_str);

        int min_time_index = 0; //初始化index
        long min_time = 0;
        struct stat buf;
        if (stat(files[min_time_index].c_str(), &buf) == 0)
        {
            min_time = buf.st_mtime;
        };

        for (int i = 0; i < (int)files.size(); i++)
        {
            //找到时间最早（小）的那个文件
            struct stat statbuf;
            if (stat(files[i].c_str(), &statbuf) == 0) //0表示获取成功
            {
                long time = statbuf.st_mtime;
                if (min_time > time)
                {
                    min_time = time;
                    min_time_index = i;
                }
            }
        }
        //执行删除命令
        system(("rm -rf " + files[min_time_index]).c_str());
        this->cal_num_size(dir_str, files_num, files_size_gb);

        // cout << "delete one" << endl;
        // cout << "num = " << files_num << std::endl;
        // cout << "size = " << files_size_gb << " gb" << endl;
    }
}

//计算给定文件目录下所有文件的数量与总的大小
void Rosbag::cal_num_size(const char *dir_str, int &files_num, double &files_size_gb)
{
    std::vector<string> files = this->get_files_name(dir_str);
    files_num = files.size();
    files_size_gb = 0.0;
    for (int i = 0; i < files_num; i++)
    {
        struct stat statbuf;
        //    const char *s = files[i].c_str();
        if (stat(files[i].c_str(), &statbuf) == 0) //0表示获取成功
        {
            files_size_gb += ((double)statbuf.st_size / 1024 / 1024 / 1024); //gb
            // long time = statbuf.st_mtime;
            // cout << files[i] << time << endl;
        }
    }
}

std::vector<string> Rosbag::get_files_name(const char *dir_str)
{
    struct dirent *ptr;
    DIR *dir = opendir(dir_str);
    std::vector<string> files;
    while ((ptr = readdir(dir)) != NULL)
    {
        //跳过'.'和'..'两个目录
        if (ptr->d_name[0] == '.')
            continue;
        //检查一下是目录还是文件
        string str = dir_str + (string)ptr->d_name;
        if (opendir(str.c_str()) != NULL) //不为空说明这是一个文件夹
        {
            // cout<<(dir_str + (string)ptr->d_name)<<" is wenjianjia"<<endl;
        }
        else if (str[str.length() - 1] == 'g') //只获取.bag文件
        {
            files.push_back(str);
            // cout<<dir_str << (string)ptr->d_name<<"  is wenjian"<<endl;
        }
    }

    // for (int i = 0; i < (int)files.size(); i++)
    //     cout << i << " = " <<files[i] << std::endl;

    return files;
}
