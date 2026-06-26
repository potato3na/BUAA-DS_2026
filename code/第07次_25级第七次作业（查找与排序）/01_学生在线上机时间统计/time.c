#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct StudentInfo
{
    char id[9];
    char name[20];
    int usetime;
} stuInfo;
stuInfo student[105];

int cmp(const void *a, const void *b)
{
    stuInfo *sa = (stuInfo *)a;
    stuInfo *sb = (stuInfo *)b;
    if (sa->usetime != sb->usetime)
        return sb->usetime - sa->usetime;
    else
        return strcmp(sa->id, sb->id);
}
int main()
{
    int n;
    scanf("%d", &n);
    char id[9], name[20];
    int usetime = 0;
    int now_cnt = 0;
    int flag = 0;
    for (int i = 0; i < n; i++)
    {
        scanf("%8s %s %d", id, name, &usetime);
        flag = 0;
        for (int i = 0; i < now_cnt; i++)
        {
            if (strcmp(student[i].id, id) == 0)
            {
                student[i].usetime += usetime;
                flag = 1;
                break;
            }
        }
        if (flag == 0)
        {
            strcpy(student[now_cnt].id, id);
            strcpy(student[now_cnt].name, name);
            student[now_cnt].usetime = usetime;
            now_cnt++;
        }
    }
    qsort(student, now_cnt, sizeof(stuInfo), cmp);

    for (int i = 0; i < now_cnt; i++)
    {
        printf("%s %s %d\n", student[i].id, student[i].name, student[i].usetime);
    }
    return 0;
}
