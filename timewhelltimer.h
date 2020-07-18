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

class time_wheel
{
public:
    time_wheel():cur_slot(0)
    {
        for(int i =0;i<N;++i)
        {
            slots[i] = NULL;        //初始化每个槽的头结点
        }
    }
    ~time_wheel()
    {
        for(int i =0;i<N;++i)
        {
            tw_timer*tmp = slots[i];
            while(tmp){
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }

    //根据定时值timeout创建一个定时器，并把它插入合适的槽中
    tw_timer*add_timer(int timeout)
    {
        if(timeout<0)
        {
            return nullptr;
        }
        int ticks = 0;
        //根据带插入定时器的超时值计算他在时间轮转动多少个滴答后被触发，并将该滴答数
        //存储在变量ticks中，如果待插入定时器的超时值小于时间轮的槽间隔si，则将ticks想桑折合为1.否则向下折合为timeout/SI
        if(timeout<SI)
        {
            ticks = 1;
        }else{
            ticks = timeout/SI;
        }
        //计算待插入的定时器在时间轮转动多少圈才被触发
        int rotation = ticks/N;
        //计算待插入的定时器应该被插入那个槽中
        int ts = (cur_slot + (ticks%N))%N;
        //创建新的定时器，在时间轮转动rotation圈后被触发，且位于第ts个槽上
        tw_timer*timer = new tw_timer(rotation,ts);
        //如果第ts个槽中没有任何定时器，则将新建的定时器插入其中，并将该定时器设置为该槽的头结点
        if(!slots[ts]){
            printf("add timer,rotation is %d, ts is %d ,cur_slot is %d\n",rotation,ts,cur_slot);
            slots[ts] = timer;
        }else{
            timer->next = slots[ts];    
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    //删除目标定时器
    void del_timer(tw_timer*timer)
    {
        if(!timer){
            return;
        }

        int ts = timer->time_slot;
        //solts[ts]是目标定时器所在槽的头结点，如果目标定时器就是该头结点，则需要重置该第ts个槽的头结点
        if(timer == slots[ts])
        {
            slots[ts] == slots[ts]->next;
            if(slots[ts]){
                slots[ts]->prev = nullptr;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if(timer->next){
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }

    //SI时间到达后，调用该函数，时间轮向前滚动一个槽的间隔
    void tick()
    {
        tw_timer *tmp = slots[cur_slot];
        printf("current slot ois %d\n",cur_slot);
        while(tmp)
        {
            printf("tick the timer once \n");
            //如果定时器的riotation值大于0，则它在本轮不起作用
            if(tmp->rotation>0)
            {
                tmp->rotation--;
                tmp=tmp->next;
            }
            else        //否则，说明定时器已经到期，于是执行定时任务，然后删除该定时器
            {
                tmp->cb_func(tmp->user_data);
                if(tmp==slots[cur_slot])
                {
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if(slots[cur_slot])
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp=slots[cur_slot];
                }else{
                    tmp->prev->next = tmp->next;
                    if(tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer*tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }

        }
    cur_slot = ++cur_slot%N;        //更新时间轮的当前槽，以反映时间轮的转动        
    }

private:
    //时间轮上槽的个数
    static const int N = 60;
    //每1s时间轮转动一次，槽间隔为一秒
    static const int SI =1;
    //时间轮的槽，其中每个元素指向一个定时器链表，链表无序
    tw_timer*slots[N];
    //时间轮的当前槽
    int cur_slot;
};






#endif