#include "rosbag.h"

Rosbag::Rosbag()
{
    char *bag_pwd = getenv("HOME");
    PATH_DIR += (std::string)bag_pwd;
    PATH_DIR += "/rosbag/";
    // cout << PATH_DIR << endl;
    this->last_error.data = 0;

    this->dir_check(PATH_DIR.c_str());
    this->sub_button = this->n.subscribe("/cti/rblite/record", 1, &Rosbag::button_callback, this);

    this->record = false;//一次只能允许触发一个录包程序

    this->button_dur = ros::Duration(15); //15秒
    this->button_record_time = ros::Time::now();
    this->is_button_record = false;//现在是否处于button录包阶段
}

//平台按钮触发录包
void Rosbag::button_callback(const cti_msgs::RobotCmd &msg)
{
    if (this->record)
        return;
    if (ros::Time::now() - this->button_record_time > this->button_dur) //如果时间超过15秒，才可以触发一次录包
    {
        this->record = true;//上锁
        this->is_button_record = true;//触发按钮录包功能
        this->button_record_time = ros::Time::now();//记录本次点击按钮时间（要保证每两次点击按钮时间间隔不能超过设定的时间）
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
        system(("rosbag record -a --duration=10 -o " + (string)PATH_DIR + "button").c_str());
        this->record = false;//解锁
    }
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
