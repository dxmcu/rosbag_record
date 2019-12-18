#include "rosbag.h"

Rosbag::Rosbag()
{
    char *bag_pwd = getenv("HOME");
    PATH_DIR += (std::string)bag_pwd;
    PATH_DIR += "/rosbag/";
    // cout << PATH_DIR << endl;
    this->last_error.data = 0;

    this->dir_check(PATH_DIR.c_str());
    // this->sub_auto_local_path = this->n.subscribe("/auto_local_path", 1, &Rosbag::auto_local_path_callback, this);
    // this->sub_target_pose = this->n.subscribe("/cti/move_controller/target_pose", 1, &Rosbag::target_pose_callback, this);
    this->sub_astar_error = this->n.subscribe("/cti/node/errorCode", 1, &Rosbag::astar_error_callback, this);
    this->sub_state_error = this->n.subscribe("/cti/move_controller/carState", 1, &Rosbag::state_error_callback, this);
    this->sub_button = this->n.subscribe("/cti/rblite/record", 1, &Rosbag::button_callback, this);

    state_1.id = 0;
    state_2.id = 0;
    state_3.id = 0;

    this->record = false;

    this->button_dur = ros::Duration(15); //15秒
    this->button_record_time = ros::Time::now();
    this->is_button_record = false;

    this->lost_dur = ros::Duration(60);
    this->lost_record_time = ros::Time::now();
    this->is_lost_record = false;
}

//平台按钮触发录包
void Rosbag::button_callback(const cti_msgs::RobotCmd &msg)
{
    if (this->record)
        return;
    if (ros::Time::now() - this->button_record_time > this->button_dur) //如果时间超过15秒，才可以触发一次录包
    {
        this->record = true;//上锁
        this->is_button_record = true;
        this->button_record_time = ros::Time::now();
    }

    // if (msg.name.empty())
    // {
    //     system(("rosbag record -a --duration=5 -o " + (string)PATH_DIR + "button").c_str());
    // }
    // else
    // {
    //     string str = msg.name;
    //     //滤掉name中奇怪的字符
    //     for (auto it =str.begin(); it < str.end(); it++)
    //     {
    //         if ((*it <= '9' && *it >= '0') ||
    //             (*it <= 'Z' && *it >= 'A') ||
    //             (*it <= 'z' && *it >= 'a'))
    //             ;
    //         else
    //         {
    //             str.erase(it);
    //             it--;
    //         }
    //     }
    //     system(("rosbag record -a --duration=5 -o " + (string)PATH_DIR + str).c_str());
    // }
}

void Rosbag::button_record(const ros::TimerEvent &event)
{
    if (this->is_button_record)
    {
        this->is_button_record = false;
        system(("rosbag record -a --duration=5 -o " + (string)PATH_DIR + "button").c_str());
        this->record = false;//解锁
    }
}

//lost
void Rosbag::state_error_callback(const cti_msgs::State &msg)
{
    //定位丢失录数据包
    state_1 = state_2;
    state_2 = state_3;
    state_3 = msg;
    if (this->record)
        return;
    if (state_1.id != 5 && state_2.id == 5 && state_3.id == 5)
    {
        if (ros::Time::now() - lost_record_time > lost_dur)
        {
            this->record = true;//开始录数据，上锁
            this->is_lost_record = true;
            this->lost_record_time = ros::Time::now();
        }
    }
}

void Rosbag::lost_record(const ros::TimerEvent &event)
{
    if (this->is_lost_record)
    {
        this->is_lost_record = false;
        system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "lost").c_str());
        this->record = false;//录完数据打开锁
    }
}

void Rosbag::astar_error_callback(const cti_msgs::ErrorCode &msg)
{
    if (this->record)
        return;
    //本次数据必须在指定的条件下才录数据
    int front_three_bit = msg.data / 100;
    int last_bit = msg.data - (((int)(msg.data / 10)) * 10);
    if (front_three_bit != 202)
    {
        this->last_error = msg;
        return;
    }
    if (last_bit != 6 && last_bit != 7)
    {
        this->last_error = msg;
        return;
    }

    //对上次数据也有要求：本次数据不能和上次数据一样，否则会导致重复录入没用的数据
    if (this->last_error.data == msg.data)
    {
        this->last_error = msg;
        return;
    }
    this->record = true; //开始锁定，进行录数据
    //开始录入数据
    this->last_error = msg;
    if (last_bit == 6)
        system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "astar_error6").c_str());
    else if (last_bit == 7)
        system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "astar_error7").c_str());
    this->record = false; //录完数据，打开锁
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
    system(("rosbag record -a --duration=4 -o " + (string)PATH_DIR + "target" + std::to_string(msg.command)).c_str());
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
    const char *dir_str = PATH_DIR.c_str();
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
