#include <stdio.h>
#include <stdlib.h>

struct line{
    int x1;
    int y1;
    int x2;
    int y2;
    int next;
    int in_degree;
};

int main()
{
    int n;
    int max_len=0;
    int max_index=0;
    scanf("%d", &n); //record the maximum nums of line
    struct line map[200];
    for(int i=0; i<n; i++){
        scanf("%d %d %d %d", &map[i].x1, &map[i].y1, &map[i].x2, &map[i].y2);
        map[i].next = -1;
        map[i].in_degree = 0;

    }

    for(int i=0; i<n; i++){
            for(int j=0; j<n; j++){
            if(map[i].x2==map[j].x1 && map[i].y2==map[j].y1){
                map[i].next = j;
                map[j].in_degree++;
            }
        }
    }
    for(int i=0; i<n; i++){
        if(map[i].in_degree==0){
            int now_len=1;
            int now=i;
            while(map[now].next != -1){
                now_len++;
                now = map[now].next;    //move forward
            }
            if(now_len > max_len){
                max_len = now_len;
                max_index = i;
            }
        }
    }

    printf("%d %d %d\n", max_len, map[max_index].x1, map[max_index].y1);
    return 0;
} 

