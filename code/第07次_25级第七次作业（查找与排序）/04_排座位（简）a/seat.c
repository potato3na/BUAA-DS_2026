#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_MAX 21
#define ID_MAX 9
#define STU_MAX 100

typedef struct
{
    char name[NAME_MAX];
    char id[ID_MAX];
    int seat;
} Student;

int cmp(const void *a, const void *b)
{
    Student *s1 = (Student *)a;
    Student *s2 = (Student *)b;

    if (s1->seat != s2->seat)
        return s1->seat - s2->seat;
    else
        return atoi(s1->id) - atoi(s2->id);
}

int cmp_id(const void *a, const void *b)
{
    Student *s1 = (Student *)a;
    Student *s2 = (Student *)b;

    return atoi(s1->id) - atoi(s2->id);
}

int get_min(int a, int b)
{
    return a < b ? a : b;
}

int getMaxSeat(Student a[], int n)
{
    int maxSeat = a[0].seat;

    for (int i = 1; i < n; i++)
    {
        if (a[i].seat > maxSeat)
            maxSeat = a[i].seat;
    }

    return maxSeat;
}

int main()
{
    FILE *fp;
    fp = fopen("in.txt", "r");
    if (fp == NULL)
    {
        return 0;
    }

    Student seat_table[STU_MAX];
    int student_num = 0;

    scanf("%d", &student_num);

    for (int i = 0; i < student_num; i++)
    {
        fscanf(fp, "%s %s %d",
               seat_table[i].id,
               seat_table[i].name,
               &seat_table[i].seat);
    }

    fclose(fp);
    qsort(seat_table, student_num, sizeof(Student), cmp);
    int current_max_seat = seat_table[student_num - 1].seat;
    int Q = get_min(student_num, current_max_seat);
    int exchange = student_num - 1;
    for (int seat = 1; seat < seat_table[0].seat && seat <= Q; seat++)
    {
        seat_table[exchange--].seat = seat;
    }
    for (int i = 1; i < student_num; i++)
    {
        // 如果扫到了已经被改过的尾部，说明补漏阶段应该停止
        if (seat_table[i].seat < seat_table[i - 1].seat)
        {
            break;
        }

        // 如果两个相邻座位号之间有空缺
        if (seat_table[i].seat - seat_table[i - 1].seat > 1)
        {
            for (int seat = seat_table[i - 1].seat + 1;
                 seat < seat_table[i].seat && seat <= Q;
                 seat++)
            {
                seat_table[exchange--].seat = seat;
            }
        }
    }

    current_max_seat = getMaxSeat(seat_table, student_num);

    int last = seat_table[0].seat;

    for (int i = 1; i < student_num; i++)
    {
        if (seat_table[i].seat == last)
        {
            seat_table[i].seat = ++current_max_seat;
        }
        else
        {
            last = seat_table[i].seat;
        }
    }

    qsort(seat_table, student_num, sizeof(Student), cmp_id);

    fp = fopen("out.txt", "w");
    if (fp == NULL)
    {
        return 0;
    }

    for (int i = 0; i < student_num; i++)
    {
        fprintf(fp, "%s %s %d\n",
                seat_table[i].id,
                seat_table[i].name,
                seat_table[i].seat);
    }

    fclose(fp);

    return 0;
}
