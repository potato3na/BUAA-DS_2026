#include<stdio.h>
#include<stdlib.h>

#define PRIVATE 1
#define PUBLIC 2
#define FOREIGN 3

typedef struct windows{
    int type;
    int wait_people;   // 表示窗口忙到哪个周期
} Window;

typedef struct customers{
    int id;
    int arrive_time;
    int sevice_time;
} Customer;

int origin_type(int i){
    if(i<3) return PRIVATE;
    else if(i==4) return PUBLIC;
    else return FOREIGN;
}

int get_private_window_num(Window windows[]){
    int cnt = 0;
    for(int i=0;i<5;i++){
        if(windows[i].type == PRIVATE){
            cnt++;
        }
    }
    return cnt;
}

void add_private_window(Window windows[]){
    for(int i=3;i<5;i++){
        if(windows[i].type!=PRIVATE){
            windows[i].type=PRIVATE;
            break;
        }
    }
}

void reduce_private_window(Window windows[], int time){
    // 优先减少空闲的临时对私窗口
    for(int i = 4; i >= 3; i--){
        if(windows[i].type == PRIVATE && windows[i].wait_people <= time){
            windows[i].type = origin_type(i);
            return;
        }
    }

    // 如果临时窗口正在服务，先改回原类型
    // 它会继续服务当前客户，但服务完后不会再接对私客户
    for(int i = 4; i >= 3; i--){
        if(windows[i].type == PRIVATE){
            windows[i].type = origin_type(i);
            return;
        }
    }
}

int main(){
    int loop_times;
    int loop_people[1000];
    Window windows[5];

    for(int i=0;i<5;i++){
        if(i<3) windows[i].type=PRIVATE;
        else if(i==4) windows[i].type=PUBLIC;
        else windows[i].type=FOREIGN;

        windows[i].wait_people=0;
    }

    scanf("%d", &loop_times);

    int total_people=0;

    Customer queue[10000];  // 排队队列
    int ans[10000];         // 每个客户的等待时间
    int front=0, rear=0;    // 队列头尾下标

    int id=1, served=0;     // 客户编号从1开始

    for(int i=0;i<loop_times;i++){
        scanf("%d", &loop_people[i]);
        total_people+=loop_people[i];
    }

    for(int time=0; served<total_people; time++){
        int new_customers=0;

        // 1. 新客户到达
        if(time<loop_times){
            new_customers=loop_people[time];

            for(int i=0;i<new_customers;i++){
                scanf("%d", &queue[rear].sevice_time);
                queue[rear].id = id++;
                queue[rear].arrive_time = time;
                rear++;
            }
        }

        // 2. 有新客户到达时，判断是否增加对私窗口
        int wait_people = rear - front;
        int private_win = get_private_window_num(windows);

        if(new_customers > 0){
            while(private_win < 5 && wait_people >= private_win * 7){
                add_private_window(windows);
                private_win = get_private_window_num(windows);
            }
        }

        // 3. 空闲对私窗口开始服务队首客户
        int served_this_time = 0;

        for(int i=0;i<5;i++){
            if(front >= rear){
                break;
            }

            if(windows[i].type == PRIVATE && windows[i].wait_people <= time){
                Customer c = queue[front++];

                ans[c.id] = time - c.arrive_time;

                windows[i].wait_people = time + c.sevice_time;

                served++;
                served_this_time++;
            }
        }

        // 4. 有客户开始服务后，等待人数减少，及时减少临时窗口
        if(served_this_time > 0){
            wait_people = rear - front;
            private_win = get_private_window_num(windows);

            while(private_win > 3 && wait_people < private_win * 7){
                reduce_private_window(windows, time);
                private_win = get_private_window_num(windows);
            }
        }
    }

    for(int i=1;i<=total_people;i++){
        printf("%d : %d\n", i, ans[i]);
    }

    return 0;
}
