#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNUM 512 // 地铁最大站数
#define MAXLEN 64  // 地铁站名的最大长度
#define INFINITY 32767

struct station
{
    char sname[MAXLEN]; // 车站名
    int ischange;       // 是否为换乘站，0-否，1-换乘
};

struct weight
{
    int wei; //  两个站间的权重，即相差站数，缺省为1
    int lno; // 两个顶点所在的线号
};

struct station BGvertex[MAXNUM];         // 地铁网络图顶点数组
struct weight BGweights[MAXNUM][MAXNUM]; // 网络图权重数组，邻接矩阵
int Vnum = 0;                            // 实际地铁总站数
int addVertex(struct station st)
{
    int i;
    for (i = 0; i < Vnum; i++)
    {
        if (strcmp(BGvertex[i].sname, st.sname) == 0)
            return i;
    }
    BGvertex[Vnum] = st;
    return Vnum++;
}
void initWeights()
{
    int i, j;
    for (i = 0; i < MAXNUM; i++)
    {
        for (j = 0; j < MAXNUM; j++)
        {
            if (i == j)
                BGweights[i][j].wei = 0;
            else
                BGweights[i][j].wei = INFINITY;
            BGweights[i][j].lno = -1;
        }
    }
}
void initMap()
{
    FILE *fp;
    int i, j, snum, lno, lnum, v1, v2;
    struct station st;
    fp = fopen("bgstations.txt", "r");
    if (fp == NULL)
    {
        printf("Cannot open file\n");
        exit(1);
    }
    fscanf(fp, "%d", &snum); // 线路总数
    for (i = 0; i < snum; i++)
    {
        fscanf(fp, "%d %d", &lno, &lnum);
        v1 = v2 = -1;
        for (j = 0; j < lnum; j++)
        {
            fscanf(fp, "%s %d", st.sname, &st.ischange);
            v2 = addVertex(st);
            if (v1 != -1)
            {
                BGweights[v1][v2].wei = BGweights[v2][v1].wei = 1;
                BGweights[v1][v2].lno = BGweights[v2][v1].lno = lno;
            }
            v1 = v2; // 当前站变为下一次循环的上一站
        }
    }
    fclose(fp);
}

/*
 * 功能：Dijkstra 单源最短路径
 * 参数：dist[i] 为 src 到 i 的最短距离，prev[i] 为路径前驱
 * 返回：成功 1，失败 0
 */
int mgDijkstra(int src, int dist[], int prev[])
{
    int done[MAXNUM] = {0};
    int i, j, v, min;

    if (src < 0 || src >= Vnum)
        return 0;

    for (i = 0; i < Vnum; i++)
    {
        dist[i] = BGweights[src][i].wei;
        if (BGweights[src][i].wei < INFINITY && i != src)
            prev[i] = src;
        else
            prev[i] = -1;
    }
    dist[src] = 0;
    done[src] = 1;
    prev[src] = -1;

    for (i = 1; i < Vnum; i++)
    {
        min = INFINITY;
        v = -1;

        for (j = 0; j < Vnum; j++)
        {
            if (!done[j] && dist[j] < min)
            {
                min = dist[j];
                v = j;
            }
        }

        if (v == -1)
            break;

        done[v] = 1;
        for (j = 0; j < Vnum; j++)
        {
            if (!done[j] &&
                BGweights[v][j].wei < INFINITY &&
                dist[v] + BGweights[v][j].wei < dist[j])
            {
                dist[j] = dist[v] + BGweights[v][j].wei;
                prev[j] = v;
            }
        }
    }
    return 1;
}



int findVertex(char name[])
{
    int i;
    for (i = 0; i < Vnum; i++)
    {
        if (strcmp(BGvertex[i].sname, name) == 0)
            return i;
    }
    return -1;
}

void printRoute(int start, int end, const int prev[])
{
    int path[MAXNUM];
    int route[MAXNUM];
    int cnt = 0;
    int i;

    int v = end;
    while(v!=-1)
    {
        path[cnt++] = v;
        v = prev[v];
    }//从end开始追踪，获得反向路径
    if(path[cnt-1]!=start)
    {
        printf("No path\n");
        return;
    }
    for (i = 0;i<cnt;i++)
    {
        route[i] = path[cnt-1-i];
    }//反转路径
    if(cnt==1)
    {
        printf("%s\n", BGvertex[start].sname);
        return;
    }
    
    printf("%s", BGvertex[route[0]].sname);

    int curLine = BGweights[route[0]][route[1]].lno;
    int rideCount = 1;
    for (i = 1; i < cnt - 1; i++)
    {
        int nextLine = BGweights[route[i]][route[i + 1]].lno;

        if (nextLine == curLine)
        {
            rideCount++;
        }
        else
        {
            // 到 route[i] 这个站换乘
            printf("-%d(%d)-%s", curLine, rideCount, BGvertex[route[i]].sname);
            curLine = nextLine;
            rideCount = 1;
        }
    }
    printf("-%d(%d)-%s\n", curLine, rideCount, BGvertex[end].sname);
}
int main()
{
    char startName[MAXLEN], endName[MAXLEN];
    int start, end;
    int dist[MAXNUM], prev[MAXNUM];

    initWeights();
    initMap();

    scanf("%s %s", startName, endName);
    start = findVertex(startName);
    end = findVertex(endName);

    if (start == -1 || end == -1)
    {
        printf("Invalid station name\n");
        return 0;
    }
    mgDijkstra(start, dist, prev);
    if (dist[end] == INFINITY)
    {
        printf("No path\n");
    }
    else
    {
        printRoute(start, end, prev);
    }

    return 0;
}
