#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXVER 512
#define MAXEDGE 4096
#define INFINITY 32767

int Weight[MAXVER][MAXVER];
int Edgeid[MAXVER][MAXVER];
int edges[MAXVER];

void Prim(int weights[][MAXVER], int n, int src, int edges[]);
void printPrimMST(int edges[], int weights[][MAXVER], int Edgeid[][MAXVER], int n, int src);
void sortArray(int a[], int n);
int main()
{
    int vi, vj, weight, id;
    int vernum, edgenum;

    scanf("%d %d", &vernum, &edgenum);

    // init the matrix
    for (int i = 0; i < vernum; i++)
    {
        for (int j = 0; j < vernum; j++)
        {
            if (i == j)
                Weight[i][j] = 0;
            else
                Weight[i][j] = INFINITY;

            Edgeid[i][j] = -1;
        }
    }

    // fill the matrix
    for (int i = 0; i < edgenum; i++)
    {
        scanf("%d %d %d %d", &id, &vi, &vj, &weight);
        Weight[vi][vj] = weight;
        Weight[vj][vi] = weight;
        Edgeid[vi][vj] = id;
        Edgeid[vj][vi] = id;
    }

    Prim(Weight, vernum, 0, edges);
    printPrimMST(edges, Weight, Edgeid, vernum, 0);

    return 0;
}

void Prim(int weights[][MAXVER], int n, int src, int edges[])
{
    int minweight[MAXVER], min;
    int i, j, k;

    for (i = 0; i < n; i++)
    {                                   /* 初始化相关数组 */
        minweight[i] = weights[src][i]; /* 将 src 顶点与之有边的权值存入数组 */
        edges[i] = src;                 /* 初始时所有顶点的前序顶点设为 src */
    }
    minweight[src] = 0; /* 将第一个顶点 src 加入生成树 */

    for (i = 1; i < n; i++)
    {
        min = INFINITY;
        k = -1;
        for (j = 0; j < n; j++)
        {
            if (minweight[j] != 0 && minweight[j] < min)
            {
                min = minweight[j];
                k = j;
            }
        }

        if (k == -1)
        { /* 图不连通时无法继续 */
            return;
        }

        minweight[k] = 0; /* 找到最小树的一个顶点 */
        for (j = 0; j < n; j++)
        {
            if (minweight[j] != 0 && weights[k][j] < minweight[j])
            {
                minweight[j] = weights[k][j]; /* 更新更小边权 */
                edges[j] = k;                 /* 将边 (k,j) 信息存入边数组 */
            }
        }
    }
}

void printPrimMST(int edges[], int weights[][MAXVER], int Edgeid[][MAXVER], int n, int src)
{
    int i, sum = 0;
    int orderEdge[MAXEDGE];
    int flag = 0;
    for (i = 0; i < n; i++)
    {
        if (i != src && weights[edges[i]][i] < INFINITY)
        {
            // printf("(%d,%d) weight=%d\n", edges[i], i, weights[edges[i]][i]);
            sum += weights[edges[i]][i];
            orderEdge[flag++] = Edgeid[edges[i]][i];
        }
    }
    printf("%d\n", sum);
    sortArray(orderEdge, flag);
    for (int i = 0; i < flag; i++)
    {
        printf("%d", orderEdge[i]);
        if (i != flag - 1)
            printf(" ");
        else
            printf("\n");
    }
}

void sortArray(int a[], int n)
{
    int i, j, temp;

    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }

}
