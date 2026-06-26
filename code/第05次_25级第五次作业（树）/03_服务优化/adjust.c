#include <stdio.h>
#include <stdlib.h>
#define MAXID 1000
#define MAXGATE 120
#define MAXDEG 3
typedef struct
{
    int gate;
    int flow;
} Gate;
int child[MAXID][MAXDEG];
int degree[MAXID];

Gate gates[MAXGATE];

int cmp(const void *a, const void *b)
{
    Gate *x = (Gate *)a;
    Gate *y = (Gate *)b;

    if (x->flow != y->flow)
        return y->flow - x->flow; // 流量大的在前

    return x->gate - y->gate; // 流量相同，编号小的在前
}
int main()
{
    int r, s, i;
    while (scanf("%d", &r) == 1)
    {
        if (r == -1)
            break;
        degree[r] = 0;
        while (scanf("%d", &s) == 1 && s != -1)
        {
            child[r][degree[r]++] = s;
        }
    }
    // BFS
    int queue[MAXID];
    int front = 0, rear = 0;
    queue[rear++] = 100; // 根节点编号为100
    int gateCount = 0;
    int gates_order[MAXGATE];
    while (front < rear)
    {
        int u = queue[front++]; // 出队
        for (int i = 0; i < degree[u]; i++)
        {
            int v = child[u][i];
            if (v < 100)
            {                                // boarding gate (No.<100)
                gates_order[gateCount++] = v; // record boarding gate num
            }
            else
            {
                queue[rear++] = v; // fork node
            }
        }
    }
    for (int i = 0; i < gateCount; i++)
    {
        scanf("%d %d", &gates[i].gate, &gates[i].flow);
    }
    qsort(gates, gateCount, sizeof(Gate), cmp);
    for (int i = 0; i < gateCount; i++)
    {
        printf("%d->%d\n", gates[i].gate, gates_order[i]);
    }
    return 0;
}

