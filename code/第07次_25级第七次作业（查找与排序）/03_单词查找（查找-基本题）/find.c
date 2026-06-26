#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define NHASH 3001
#define WORD_MAX 21
#define WORD_NUM 3500
#define HASH_MULT 37

unsigned int hash(char *str);
void binaryInsertionSort(int a[], int n);
int if_enter(char *s);
char words[WORD_NUM][WORD_MAX];

typedef struct
{
    int start;
    int count;
} Index;
Index indexTable[26];

int searchSeqSorted(const char words[][WORD_MAX], int n,
                    const char *key, int *cmpCount);
int searchBin(const char words[][WORD_MAX], int n,
              const char *key, int *cmpCount);
void searchBuildLetterIndex(const char words[][WORD_MAX], int n,
                            int index[26][2]);
int searchBinRange(const char words[][WORD_MAX], int low, int high,
                   const char *key, int *cmpCount);
int searchByIndex(const char words[][WORD_MAX], int index[26][2],
                  const char *key, int *cmpCount);
unsigned int hashStr(const char *str);
typedef struct HashNode
{
    char word[WORD_MAX];
    struct HashNode *link;
} HashNode;
void hashInsertTail(HashNode *table[], const char *word);
int searchHashChain(HashNode *table[], const char *key, int *cmpCount);
void hashFreeAll(HashNode *table[]);
int cmp_times = 0;
int main()
{
    FILE *fp;
    fp = fopen("dictionary3000.txt", "r");
    if (fp == NULL)
    {
        return 0;
    }
    /*------读入文件-----*/
    int i = 0;
    while (i < WORD_NUM && fscanf(fp, "%s", words[i]) == 1)
    {
        i++;
    } // 存入数组
    int words_num = i;
    /*------构造索引表-----*/
    int index[26][2];
    searchBuildLetterIndex(words, words_num, index);
    /*------构造哈希表-----*/
    HashNode *hash_table[NHASH];
    for (int i = 0; i < NHASH; i++)
    {
        hash_table[i] = NULL;
    }
    for (int i = 0; i < words_num; i++)
    {
        hashInsertTail(hash_table, words[i]);
    }
    /*------获取输入-----*/
    char key[WORD_MAX];
    int opnum;
    scanf("%s %d", key, &opnum);
    switch (opnum)
    {
        /*------type 1 顺序查找-----*/
    case 1:
        if (searchSeqSorted(words, words_num, key, &cmp_times))
            printf("1 %d\n", cmp_times);
        else
            printf("0 %d\n", cmp_times);
        break;

        /*------type 2 折半查找-----*/
    case 2:
        if (searchBin(words, words_num, key, &cmp_times))
            printf("1 %d\n", cmp_times);
        else
            printf("0 %d\n", cmp_times);
        break;

        /*------type 3 索引折半查找-----*/
    case 3:
        if (searchByIndex(words, index, key, &cmp_times))
            printf("1 %d\n", cmp_times);
        else
            printf("0 %d\n", cmp_times);
        break;

    case 4:
        /*------type 4 查找hash-----*/
        if (searchHashChain(hash_table, key, &cmp_times))
            printf("1 %d\n", cmp_times);
        else
            printf("0 %d\n", cmp_times);
        break;

    }

    hashFreeAll(hash_table); // 释放哈希表全部结点
    fclose(fp);
}

int orderFind(int a[], int n, int key)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] >= key)
            return i;
    }
    return -1;
}
int binarySearch(const int a[], int n, int key)
{
    int low, high, mid;

    low = 0;
    high = n - 1;

    while (low <= high)
    {
        mid = low + (high - low) / 2;

        if (a[mid] == key)
            return mid;
        else if (key < a[mid])
            high = mid - 1;
        else
            low = mid + 1;
    }
    return -1;
}
/*
 * 功能：在已按字典序排好序的单词表中做顺序查找
 *       遇到相同或第一个比 key 大的单词即停止
 * 返回：1 找到，0 未找到；*cmpCount 为 strcmp 比较次数
 *
 * 使用示例：
 *   int cnt;
 *   if (searchSeqSorted(words, n, "yes", &cnt))
 *       printf("1 %d\n", cnt);
 */
int searchSeqSorted(const char words[][WORD_MAX], int n,
                    const char *key, int *cmpCount)
{
    int i, cond;

    *cmpCount = 0;
    for (i = 0; i < n; i++)
    {
        cond = strcmp(words[i], key);
        (*cmpCount)++;
        if (cond == 0)
            return 1;
        if (cond > 0)
            return 0;
    }
    return 0;
}

/*
 * 功能：折半查找（上课 PPT 标准写法）
 * 返回：1 找到，0 未找到；*cmpCount 为比较次数
 *
 * 使用示例：
 *   int cnt;
 *   searchBin(words, n, "wins", &cnt);   // 未找到，cnt 约为 log2(n)
 */
int searchBin(const char words[][WORD_MAX], int n,
              const char *key, int *cmpCount)
{
    int low, high, mid, res;

    low = 0;
    high = n - 1;
    *cmpCount = 0;
    while (low <= high)
    {
        mid = (low + high) / 2;
        (*cmpCount)++;
        res = strcmp(words[mid], key);
        if (res == 0)
            return 1;
        if (res < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return 0;
}

/*
 * 功能：为已排序单词表构建 26 字母索引表
 *       index[c-'a'][0] = 以 c 为首字母的单词起始下标
 *       index[c-'a'][1] = 该字母对应单词个数
 * 返回：无
 *
 * 使用示例：
 *   int index[26][2];
 *   searchBuildLetterIndex(words, n, index);
 */
void searchBuildLetterIndex(const char words[][WORD_MAX], int n,
                            int index[26][2])
{
    int i, k;
    char first;

    for (k = 0; k < 26; k++)
        index[k][0] = index[k][1] = 0;

    if (n == 0)
        return;

    first = 'a';
    index[first - 'a'][0] = 0;
    for (i = 0; i < n; i++)
    {
        if (words[i][0] > first)
        {
            index[first - 'a'][1] = i - index[first - 'a'][0];
            index[words[i][0] - 'a'][0] = i;
            first = words[i][0];
        }
    }
    index[first - 'a'][1] = n - index[first - 'a'][0];
}

/*
 * 功能：在指定下标范围内对折半查找（供索引查找内部使用）
 * 返回：1 找到，0 未找到；*cmpCount 为比较次数
 */
int searchBinRange(const char words[][WORD_MAX], int low, int high,
                   const char *key, int *cmpCount)
{
    int mid, res;

    *cmpCount = 0;
    while (low <= high)
    {
        mid = (low + high) / 2;
        (*cmpCount)++;
        res = strcmp(words[mid], key);
        if (res == 0)
            return 1;
        if (res < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return 0;
}

/*
 * 功能：索引表 + 折半查找
 * 返回：1 找到，0 未找到；*cmpCount 为比较次数
 *
 * 使用示例：
 *   int cnt;
 *   searchByIndex(words, index, "yes", &cnt);
 */
int searchByIndex(const char words[][WORD_MAX], int index[26][2],
                  const char *key, int *cmpCount)
{
    int low, high;

    low = index[key[0] - 'a'][0];
    high = index[key[0] - 'a'][0] + index[key[0] - 'a'][1] - 1;
    return searchBinRange(words, low, high, key, cmpCount);
}

/*
 * 功能：计算字符串散列值（作业给定公式）
 * 返回：0 .. NHASH-1
 */
unsigned int hashStr(const char *str)
{
    unsigned int h = 0;
    const char *p;

    for (p = str; *p != '\0'; p++)
        h = HASH_MULT * h + (unsigned char)*p;
    return h % NHASH;
}

/*
 * 功能：向散列表指定桶的链表尾插入单词
 *       字典按序读入时，链自然保持字典序（与 find.c 参考实现一致）
 * 返回：无
 */
void hashInsertTail(HashNode *table[], const char *word)
{
    HashNode *p, *q;
    unsigned int k;

    k = hashStr(word);
    p = (HashNode *)malloc(sizeof(HashNode));
    strcpy(p->word, word);
    p->link = NULL;
    if (table[k] == NULL)
        table[k] = p;
    else
    {
        q = table[k];
        while (q->link != NULL)
            q = q->link;
        q->link = p;
    }
}

/*
 * 功能：链地址法散列查找
 *       在冲突链表中顺序比较，遇到更大单词或链表结束即停止
 * 返回：1 找到，0 未找到；*cmpCount 为比较次数
 */
int searchHashChain(HashNode *table[], const char *key, int *cmpCount)
{
    HashNode *p;
    int cond;

    *cmpCount = 0;
    p = table[hashStr(key)];
    while (p != NULL)
    {
        cond = strcmp(p->word, key);
        (*cmpCount)++;
        if (cond == 0)
            return 1;
        if (cond > 0)
            return 0;
        p = p->link;
    }
    return 0;
}

/*
 * 功能：释放散列表全部结点
 * 返回：无
 */
void hashFreeAll(HashNode *table[])
{
    int i;
    HashNode *p, *q;

    for (i = 0; i < NHASH; i++)
    {
        p = table[i];
        while (p != NULL)
        {
            q = p->link;
            free(p);
            p = q;
        }
        table[i] = NULL;
    }
}

