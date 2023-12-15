#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/wait.h>
#include <stdlib.h>

#include <sys/epoll.h>

#include <iostream>
#include <algorithm>

#include <string>
using std::string;

#include <unordered_set>
using std::unordered_set;

#include <mutex>
using std::mutex;

#include <queue>
using std::queue;

#include <vector>
using std::vector;
using std::iterator;

#include <unordered_map>
using std::unordered_map;

#include "Thread_Controler"
using namespace susu_tools;

#include <string.h>

#include "./SUSU_LogPrinter/susu_log_printer"
#include "./SUSU_EpollObject/susu_epoll_object"
#include "./SUSU_InitParam/susu_init_param"

int startup();  //开始监听
int epoll_task(epoll_object* epoll_object);

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        return -1;
    }

    //1.读取参数
    susu_init_param* init_param = susu_init_param::get_Init_Param();
    init_param->load_init_param(argv[1]);  //因为内部有异常处理，所以无需在此处检查返回值

    Log_Printer* log = Log_Printer::get_Log_Printer();//在服务端启动时即初始log对象
    log->set_path(init_param->get_string_value("log_path"));
    log->set_print_level(init_param->get_int_value("log_level"));
    //log->set_immediately_print_level(init_param->get_int_value("immediately_print_level"));

    //2.建立并监听socket
    int server_sock = -1;    //待监听的socket
    int client_sock = -1;   //客户端
    
    //sockaddr_in 是 IPV4的套接字地址结构。定义在<netinet/in.h>,参读《TLPI》P1202
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    server_sock = startup(); //因为内部有异常处理，所以无需在此处检查返回值
    
    string temp_log_str;
    
    temp_log_str = "httpd running on port "+std::__cxx11::to_string(init_param->get_int_value("port"))+",socket "+std::__cxx11::to_string(server_sock);
    log->print_a_line(3,temp_log_str);      

    //根据配置文件，初始化若干个线程和epoll对象
    Thread_Controler::set_init_count(init_param->get_int_value("thread_init_count"));//设置初始线程数量
    Thread_Controler* thread_controler = Thread_Controler::get_Thread_Controler();//在服务端启动时即初始化线程池

    thread_controler->set_dynamic_param(init_param->get_int_value("thread_expansion_param"));//设置线程扩缩容参数
    thread_controler->set_thread_low_limit(init_param->get_int_value("thread_down_limit"));//设置最小线程数量
    thread_controler->set_thread_up_limit(init_param->get_int_value("thread_up_limit"));//设置最大线程数量

    vector<epoll_object*> epoll_object_vector;
    for(int i=0;i<init_param->get_int_value("thread_init_count");i++)
    {
        //新建一个epoll对象,每个对象最多管理 Init_Param.epoll_count 个socket
        epoll_object* p = new epoll_object(init_param->get_int_value("epoll_count"));
        temp_log_str = "set a epoll object,epoll count is "+ std::__cxx11::to_string(init_param->get_int_value("epoll_count"));
        log->print_a_line(3,temp_log_str);
        //把对象的loop函数作为可执行的task添加到线程池
        thread_controler->add_task(epoll_task,p);
        //由于load_balace的设计，每个task都独占1个线程

        //把epoll对象添加到vector内，方便管理
        epoll_object_vector.push_back(p);
    }
    //printf("the epoll object count is %d\n",epoll_object_vector.size());
    //for(auto it = epoll_object_vector.begin();it != epoll_object_vector.end();it++)
    //{
    //    printf("the object pointer is %p\n",*it);
    //}

    //int c = 0;    
    while (1)
    {
        //阻塞等待客户端的连接，参读《TLPI》P1157
        client_sock = accept(server_sock,
                       (struct sockaddr *)&client_name,
                       &client_name_len);
        if (client_sock != -1)
        {
            //printf("before add the count is :%d\n",c);
            //把client_socket添加给数量最低的epoll对象。
            //也就是说，在此处做了负载均衡
            temp_log_str = "accept a server_sock,the ip is" + client_name.sin_addr.s_addr;
            log->print_a_line(3,temp_log_str);

            int index = 0;
            int min = 1024*1024*1024;
            int target = 0;
            
            for(index=0;index<epoll_object_vector.size();index++)
            {
                if(epoll_object_vector[index]->get_current_event_count() < min)
                {
                    min = epoll_object_vector[index]->get_current_event_count();
                    target = index;
                }
            }
            //printf("get a fd %d\n",client_sock);
            //printf("add this event to:%p\n",(epoll_object_vector[target]));

            if( -1 == epoll_object_vector[target]->add_a_event(client_sock))
            {
                //若监听该socket失败。说明socket太多服务线程无法再管理新socket，此时由主线程把该socket关闭
                temp_log_str = "Failed to allocate this socket,try to close it." + client_name.sin_addr.s_addr;
                log->print_a_line(30,temp_log_str);
                close(client_sock);
            }
            //c++;
            //printf("now the count is%d\n",c);
            //int ret = epoll_object_vector[target]->add_a_event(client_sock);
            //printf("insert to %d %d\n",target,ret);
        }
        else
        {
            fprintf(stderr, "Failed in accept client_socket\n");
            
            temp_log_str = "Failed in accept client_socket";
            log->print_a_line(30,temp_log_str);
        }
    }
    close(server_sock);
    //epoll_close();
    return 0;
}

int startup()   //开始监听
{
    susu_init_param* init_param = susu_init_param::get_Init_Param();
    int httpd = 0;
    try
    {
        if (init_param->get_int_value("port") == 0)
        {
            throw "port is 0";
        }
        
        //sockaddr_in 是 IPV4的套接字地址结构。定义在<netinet/in.h>,参读《TLPI》P1202
        struct sockaddr_in name;
    
        //socket()用于创建一个用于 socket 的描述符，函数包含于<sys/socket.h>。参读《TLPI》P1153
        //这里的PF_INET其实是与 AF_INET同义，具体可以参读《TLPI》P946
        httpd = socket(PF_INET, SOCK_STREAM, 0);
        if (httpd == -1)
        {
            throw "socket init failed";
        }

        memset(&name, 0, sizeof(name));
        name.sin_family = AF_INET;
        //htons()，ntohs() 和 htonl()包含于<arpa/inet.h>, 参读《TLPI》P1199
        //将Init_Param.port 转换成以网络字节序表示的16位整数
        name.sin_port = htons(init_param->get_int_value("port"));
        //INADDR_ANY是一个 IPV4通配地址的常量，包含于<netinet/in.h>
        //大多实现都将其定义成了0.0.0.0 参读《TLPI》P1187
        name.sin_addr.s_addr = htonl(INADDR_ANY);
        
        //设置端口复用，避免意外中断后无法快速启动
        int opt = 1;
        if (setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        
        //bind()用于绑定地址与 socket。参读《TLPI》P1153
        //如果传进去的sockaddr结构中的 sin_port 指定为0，这时系统会选择一个临时的端口号
        if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
        {
            throw "socket bind failed";
        }

        //最初的 BSD socket 实现中，backlog 的上限是5.参读《TLPI》P1156
        //baclkog实际上是tcp的 establish队列的长度，根据经验，若想和其它服务器组件配合得比较好，可以设置为512.这里的其它服务器组件主要是nignx等
        if (listen(httpd, 512) < 0)
        {
            throw "socket listen failed";
        }

    }
    catch(const char* error_string)
    {
        string temp = error_string;
        Log_Printer* log = Log_Printer::get_Log_Printer();//在服务端启动时即初始log对象
        log->print_a_line(3,temp);
        exit(-1);
    }
    return httpd;
}

int epoll_task(epoll_object* epoll_object)
{
    epoll_object->epoll_process();
    return 0;
}