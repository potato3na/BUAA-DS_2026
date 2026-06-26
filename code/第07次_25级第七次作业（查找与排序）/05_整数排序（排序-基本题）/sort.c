#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== 宏与常量 ==================== */

#define WORD_MAX 21   /* 单词最大长度（含 '\0'） */
#define DICT_MAX 3500 /* 字典最大单词数 */
#define NHASH 3001    /* 散列表长度（与作业一致） */
#define HASH_MULT 37  /* 散列乘数（与作业一致） */
/* ==================== Part B：排序 ==================== */
/*
 * 功能：交换两个整数
 * 返回：无
 */
void sortSwap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
/*
 * 功能：简单选择排序（升序）
 *       比较次数 = 选择最小元素时的两两比较次数
 * 返回：比较次数
 *
 * 使用示例：
 *   int cmp = sortSelect(a, n);
 *   printf("%d\n", cmp);
 */
int sortSelect(int k[], int n)
{
    int i, j, d, temp, t = 0;

    for (i = 0; i < n - 1; i++)
    {
        d = i;
        for (j = i + 1; j < n; j++)
        {
            if (k[j] < k[d])
                d = j;
            t++;
        }
        if (d != i)
        {
            temp = k[d];
            k[d] = k[i];
            k[i] = temp;
        }
    }
    return t;
}
/*
 * 功能：冒泡排序（升序，与 sort.c 一致）
 *       从后向前扫描；某趟无交换则提前结束
 *       比较次数 = 相邻元素比较次数
 * 返回：比较次数
 */
int sortBubble(int k[], int n)
{
    int i, j, flag = 1, temp, t = 0;

    for (i = n - 1; i > 0 && flag == 1; i--)
    {
        flag = 0;
        for (j = 0; j < i; j++)
        {
            if (k[j] > k[j + 1])
            {
                temp = k[j];
                k[j] = k[j + 1];
                k[j + 1] = temp;
                flag = 1;
            }
            t++;
        }
    }
    return t;
}

/*
 * 功能：堆调整（大顶堆下沉，与 sort.c 红字语句一致）
 *       比较次数 = 根与孩子的比较次数（while 内 t++）
 * 返回：本次调整中的比较次数
 */
int sortHeapAdjust(int k[], int i, int n)
{
    int j, temp, t = 0;

    temp = k[i];
    j = 2 * i + 1;
    while (j < n)
    {
        t++;
        if (j < n - 1 && k[j] < k[j + 1])
            j++;
        if (temp >= k[j])
            break;
        k[(j - 1) / 2] = k[j];
        j = 2 * j + 1;
    }
    k[(j - 1) / 2] = temp;
    return t;
}

/*
 * 功能：堆排序（升序，先建大顶堆再依次取出堆顶）
 * 返回：总比较次数
 */
int sortHeap(int k[], int n)
{
    int i, temp, t = 0;

    for (i = n / 2 - 1; i >= 0; i--)
        t += sortHeapAdjust(k, i, n);
    for (i = n - 2; i >= 0; i--)
    {
        temp = k[i + 1];
        k[i + 1] = k[0];
        k[0] = temp;
        t += sortHeapAdjust(k, 0, i + 1);
    }
    return t;
}

/*
 * 功能：归并两段有序区间 [left..leftend] 与 [leftend+1..rightend]
 *       比较次数 = merge 中红字 if 的执行次数
 * 返回：本次归并的比较次数
 */
int sortMerge(int x[], int tmp[], int left, int leftend, int rightend)
{
    int i = left, j = leftend + 1, q = left, t = 0;

    while (i <= leftend && j <= rightend)
    {
        t++;
        if (x[i] <= x[j])
            tmp[q++] = x[i++];
        else
            tmp[q++] = x[j++];
    }
    while (i <= leftend)
        tmp[q++] = x[i++];
    while (j <= rightend)
        tmp[q++] = x[j++];
    for (i = left; i <= rightend; i++)
        x[i] = tmp[i];
    return t;
}

/*
 * 功能：归并排序递归体
 * 返回：比较次数
 */
int sortMergeRec(int k[], int tmp[], int left, int right)
{
    int center, t = 0;

    if (left < right)
    {
        center = (left + right) / 2;
        t += sortMergeRec(k, tmp, left, center);
        t += sortMergeRec(k, tmp, center + 1, right);
        t += sortMerge(k, tmp, left, center, right);
    }
    return t;
}

/*
 * 功能：二路归并排序（升序，递归版）
 * 返回：总比较次数；tmp 需由调用方提供长度 n 的临时数组
 */
int sortMergeSort(int k[], int tmp[], int n)
{
    if (n <= 0)
        return 0;
    return sortMergeRec(k, tmp, 0, n - 1);
}

/*
 * 功能：快速排序（升序，与 sort.c 一致：选 left 为基准）
 *       比较次数 = 分界元素与其它元素的比较次数
 * 返回：总比较次数
 */
int sortQuick(int k[], int left, int right)
{
    int i, last, t = 0;

    if (left < right)
    {
        last = left;
        for (i = left + 1; i <= right; i++)
        {
            if (k[i] < k[left])
                sortSwap(&k[++last], &k[i]);
            t++;
        }
        sortSwap(&k[left], &k[last]);
        t += sortQuick(k, left, last - 1);
        t += sortQuick(k, last + 1, right);
    }
    return t;
}

int main()
{
    int num, sortType, a[100];
    scanf("%d %d", &num, &sortType);
    for (int i = 0; i < num; i++)
    {
        scanf("%d", &a[i]);
    }
    int cmptimes = 0;
    int tmp[100];

    switch (sortType)
    {
    case 1:
        cmptimes = sortSelect(a, num);
        for (int i = 0; i < num; i++)
        {
            printf("%d ", a[i]);
        }

        break;
    case 2:
        cmptimes = sortBubble(a, num);
        for (int i = 0; i < num; i++)
        {
            printf("%d ", a[i]);
        }
        break;
    case 3:
        cmptimes = sortHeap(a, num);
        for (int i = 0; i < num; i++)
        {
            printf("%d ", a[i]);
        }
        break;
    case 4:
        cmptimes = sortMergeSort(a, tmp, num);
        for (int i = 0; i < num; i++)
        {
            printf("%d ", a[i]);
        }
        break;
    case 5:
        cmptimes = sortQuick(a, 0, num - 1);
        for (int i = 0; i < num; i++)
        {
            printf("%d ", a[i]);
        }
        break;
    }

    printf("\n%d\n", cmptimes);
    return 0;
}
