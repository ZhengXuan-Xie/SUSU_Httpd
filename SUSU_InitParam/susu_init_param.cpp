#include "susu_init_param"

namespace susu_tools{

susu_init_param* susu_init_param_handle = NULL;
susu_init_param* susu_init_param::get_Init_Param()
{
    if(susu_init_param_handle == NULL)
    {
        init_mutex.lock();
        if(susu_init_param_handle == NULL)
        {
            susu_init_param_handle = new susu_init_param();
        }
        init_mutex.unlock();
    }
    return susu_init_param_handle;
}

::std::mutex susu_init_param::init_mutex;

susu_init_param::susu_init_param()
{
    string_param_kv.insert(pair<string,string>("log_path","./"));
    string_param_kv.insert(pair<string,string>("root_path","./"));

    int_param_kv.insert(pair<string,int>("port",12345));
    int_param_kv.insert(pair<string,int>("thread_up_limit",2));
    int_param_kv.insert(pair<string,int>("thread_down_limit",1));
    int_param_kv.insert(pair<string,int>("thread_expansion_param",4));
    int_param_kv.insert(pair<string,int>("epoll_count",1024));
    int_param_kv.insert(pair<string,int>("stdout_flag",1));  // 1=true 0=false in c++
    int_param_kv.insert(pair<string,int>("stderr_flag",1));  // 1=true 0=false in c++
    int_param_kv.insert(pair<string,int>("log_level",1));
    int_param_kv.insert(pair<string,int>("immediately_print_level",1));
    int_param_kv.insert(pair<string,int>("thread_init_count",1));
}
susu_init_param::~susu_init_param()
{

}
int susu_init_param::load_init_param(const char* str)  //加载参数
{
    try{
        //打开文件
        FILE* init_file = fopen(str,"r");
        char key[128] = {0};
        char value[1024] = {0};
        char line[1280] = {0};

        if(init_file != NULL)
        {
            memset(key,128,0);
            memset(value,1024,0);
            memset(line,1280,0);
            while(fscanf(init_file,"%s\n",line) != EOF)
            {
                int i;
                int check = 1;
                for(i=0;i<strlen(line);i++)
                {
                    if(line[i] == '=' && i <= 128)
                    {
                        strncpy(key,line,i);
                        strncpy(value,line+i+1,strlen(line)-i-1);
                        check = 0;
                        break;
                    }
                    else
                    {
                        check = 1;
                    }
                }
                if(check)
                {continue;}

                if(int_param_kv.find(key) != int_param_kv.end())
                {
                    int_param_kv[key] = atoi(value);
                }
                else if(string_param_kv.find(key) != string_param_kv.end())
                {
                    string_param_kv[key] = value;
                }

                memset(key,0,128);
                memset(value,0,1024);
                memset(line,0,1280);
            }
            init_param_print();
        }
        else
        {
            throw "init config is not exist";//throw关键字抛出错误类型 const char*
        }
               
    }
    catch(const char* error_string)
    {
        print_a_log_line(error_string); //打印到log 和 标准错误流上
        exit(-1);
    }
    return 0;
}

void susu_init_param::init_param_print()
{
    for(auto i = int_param_kv.begin();i != int_param_kv.end();i++)
    {
        std::cout<<i->first<<"="<<i->second<<std::endl;
    }
    for(auto i = string_param_kv.begin();i != string_param_kv.end();i++)
    {
        std::cout<<i->first<<"="<<i->second<<std::endl;
    }
}

};