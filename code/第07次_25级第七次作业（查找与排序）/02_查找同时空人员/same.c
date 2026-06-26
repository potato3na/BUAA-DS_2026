#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct mobile_log
{
    char telephone[12];
    char base_id;
    char login_time[7];
    char logout_time[7];
} log;

int cmp(const void *a, const void *b)
{
    log *la = (log *)a;
    log *lb = (log *)b;
    if (strcmp(la->telephone, lb->telephone) != 0)
    {
        return strcmp(la->telephone, lb->telephone);
    }
    else
    {
        return la->base_id-lb->base_id;
    }
}
int if_overlap(char *login_time1, char *logout_time1, char *login_time2, char *logout_time2)
{
    if (strcmp(logout_time1, login_time2) < 0 || strcmp(logout_time2, login_time1) < 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int main()
{
    int log_num;
    scanf("%d", &log_num);
    log logs[1000];
    for (int i = 0; i < log_num; i++)
    {
        scanf("%s %c %s %s", logs[i].telephone, &logs[i].base_id, logs[i].login_time, logs[i].logout_time);
    }
    char search_telephone[12];
    log overlap_telephones[1000];
    int overlap_count = 0;
    scanf("%s", search_telephone);
    for (int i = 0; i < log_num; i++)
    {
        if (strcmp(logs[i].telephone, search_telephone) != 0)
        {
            continue;
        }
        // 如果是查找的手机号则判断该时段内是否有重叠
        for (int j = 0; j < log_num; j++)
        {
            if (i == j)
            {
                continue;
            }
            if (strcmp(logs[j].telephone, search_telephone) == 0)
            {
                continue;
            }
            if ((logs[i].base_id==logs[j].base_id)&& if_overlap(logs[i].login_time, logs[i].logout_time, logs[j].login_time, logs[j].logout_time)==1)
            {
                overlap_telephones[overlap_count++] = logs[j];
            }
        }
    }
    qsort(overlap_telephones, overlap_count, sizeof(log), cmp);
    for(int i=0;i<overlap_count;i++)
    {
        if(i>0 && strcmp(overlap_telephones[i].telephone, overlap_telephones[i-1].telephone)==0&&(overlap_telephones[i].base_id==overlap_telephones[i-1].base_id))
        {
            continue;
        }
        printf("%s %c\n", overlap_telephones[i].telephone, overlap_telephones[i].base_id);
    }
    return 0;
}

