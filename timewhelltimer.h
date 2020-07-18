#ifndef TIME_WHELL_TIMER
#define TIMER_WHELL_TIMER
#include<time.h>
#include<netinet/in.h>
#include<stdio.h>

#define BUFFER_SIZE 64

class tw_timer;
struct  client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
    /* data */
};

class tw_timer
{

public:
    tw_timer(int rot, int ts):next(NULL),prev(NULL),rotation(rot),time_slot(ts)
    {

    }

public:
    int rotation;   //记录定时器在时间轮转多少圈才生效
    int time_slot;  //记录定时器属于时间轮上哪个槽
    void (*cb_func)(client_data*);  //定时器回调
    client_data*user_data;  //客户数据
    tw_timer* next; //指向下一个定时器
    tw_timer*prev;  //指向前一个定时器
};

#endif