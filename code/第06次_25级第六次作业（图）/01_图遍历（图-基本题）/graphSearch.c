
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXVER 512
#define MAXEDGE 4096
#define INFINITY 32767

typedef int vertype;

typedef struct edge
{
    int adjvex; /* 邻接顶点编号 */
    int weight; /* 权值；无权图中可统一设为 1 */
    struct edge *next;
} ELink;

typedef struct ver
{
    vertype vertex; /* 顶点信息；若顶点信息就是编号，可令 vertex=i */
    ELink *link;    /* 指向该顶点邻接表的头指针 */
} VLink;

void initListGraph(VLink graph[], int n);
ELink *insertEdge(ELink *head, int avex);
ELink *deleteEdge(ELink *head, int avex);
void destoryGraph(VLink graph[], int n);
void createGraph(VLink graph[], int *n);
void VISIT(VLink G[], int v);
void DFS(VLink G[], int v);
void travelDFS(VLink G[], int n);
void deleteVertex(VLink graph[], int *n, int v);

typedef struct
{
    int data[MAXVER];
    int front;
    int rear;
    int size;
} Queue;

void initQueue(Queue *Q);
int emptyQ(Queue *Q);
int enQueue(Queue *Q, int x);
int deQueue(Queue *Q);
void BFS(VLink G[], int v);
void travelBFS(VLink G[], int n);

int delete_num;
VLink graph[MAXVER];
int n;
int main()
{
    createGraph(graph, &n);
    travelDFS(graph, n);
    travelBFS(graph, n);
    deleteVertex(graph, &n, delete_num);
    travelDFS(graph, n);
    travelBFS(graph, n);
}
/* 初始化邻接表图 */
void initListGraph(VLink graph[], int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        graph[i].vertex = i;
        graph[i].link = NULL;
    }
}

ELink *insertEdge(ELink *head, int avex)
{
    ELink *e, *p, *pre;
    e = (ELink *)malloc(sizeof(ELink));
    if (e == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    e->adjvex = avex;
    e->weight = 1;
    e->next = NULL;

    if (head == NULL||avex < head->adjvex)
    {
        e->next = head;
        return e;
    }
    pre = head;
    p = head->next;
    while(p!=NULL&&avex>p->adjvex)
    {
        pre = p;
        p = p->next;
    }
    e->next = p;
    pre->next = e;
    return head;
}

/* 删除链表中的一个邻接点 avex */
ELink *deleteEdge(ELink *head, int avex)
{
    ELink *p = head, *pre = NULL;
    while (p != NULL)
    {
        if (p->adjvex == avex)
        {
            if (pre == NULL)
                head = p->next;
            else
                pre->next = p->next;
            free(p);
            return head;
        }
        pre = p;
        p = p->next;
    }
    return head;
}

/* 释放邻接表图 */
void destoryGraph(VLink graph[], int n)
{
    int i;
    ELink *p, *q;
    for (i = 0; i < n; i++)
    {
        p = graph[i].link;
        while (p != NULL)
        {
            q = p->next;
            free(p);
            p = q;
        }
        graph[i].link = NULL;
    }
}

/*
 * 创建无向邻接表图。
 * 输入格式与 PPT 保持一致：
 * 第一行：顶点个数 n
 * 后续 n 行：每行第一个数为顶点 v1，之后为它的邻接顶点，以 -1 结束。
 * 例如：
 * 4
 * 0 1 2 -1
 * 1 0 3 -1
 * 2 0 -1
 * 3 1 -1
 */
void createGraph(VLink graph[], int *n)
{
    int i, v1, v2, edgeNum;
    scanf("%d %d", n, &edgeNum);
    initListGraph(graph, *n);

    for (i = 0; i < edgeNum; i++)
    {
        scanf("%d %d", &v1, &v2);
        graph[v1].link = insertEdge(graph[v1].link, v2);
        graph[v2].link = insertEdge(graph[v2].link, v1);
    }
    scanf("%d", &delete_num);
    return;
}

void deleteVertex(VLink graph[], int *n, int v)
{
    int i;
    ELink *p, *q;

    if (v < 0 || v >= *n)
        return;

    // 1. 删除其他顶点邻接表中指向 v 的边
    for (i = 0; i < *n; i++)
    {
        if (i == v)
            continue;
        graph[i].link = deleteEdge(graph[i].link, v);
    }

    // 2. 释放顶点 v 自己的邻接表
    p = graph[v].link;
    while (p != NULL)
    {
        q = p->next;
        free(p);
        p = q;
    }
    graph[v].link = NULL;

    // 3. 顶点数组前移
    for (i = v + 1; i < *n; i++)
    {
        graph[i - 1] = graph[i];
    }

    // 4. 修改邻接表中的顶点编号
    for (i = 0; i < *n - 1; i++)
    {
        for (p = graph[i].link; p != NULL; p = p->next)
        {
            if (p->adjvex > v)
                p->adjvex--;
        }
    }

    graph[*n - 1].link = NULL;
    graph[*n - 1].vertex = *n - 1;

    (*n)--;
}
/* ============================================================
 * 三、DFS 深度优先遍历
 * PPT 对应函数：travelDFS、DFS
 * ============================================================ */

int Visited[MAXVER] = {0};

void VISIT(VLink G[], int v)
{
    printf("%d ", G[v].vertex);
}

void DFS(VLink G[], int v)
{
    ELink *p;
    Visited[v] = 1; /* 标识某顶点被访问过 */
    VISIT(G, v);    /* 访问某顶点 */
    for (p = G[v].link; p != NULL; p = p->next)
        if (!Visited[p->adjvex])
            DFS(G, p->adjvex);
}

void travelDFS(VLink G[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        Visited[i] = 0;
    for (i = 0; i < n; i++)
        if (!Visited[i])
            DFS(G, i);
    printf("\n");
}

/* ============================================================
 * 四、BFS 广度优先遍历
 * PPT 中使用 enQueue、deQueue、emptyQ，这里补全队列。
 * ============================================================ */

void initQueue(Queue *Q)
{
    Q->front = 0;
    Q->rear = 0;
    Q->size = 0;
}

int emptyQ(Queue *Q)
{
    return Q->size == 0;
}

int enQueue(Queue *Q, int x)
{
    if (Q->size >= MAXVER)
        return 0;
    Q->data[Q->rear++] = x;
    Q->size++;
    return 1;
}

int deQueue(Queue *Q)
{
    int x;
    if (emptyQ(Q))
        return -1;
    x = Q->data[Q->front++];
    Q->size--;
    return x;
}

void BFS(VLink G[], int v)
{
    ELink *p;
    Queue Q;
    initQueue(&Q);

    Visited[v] = 1; /* 标识某顶点已入队 */
    enQueue(&Q, v);
    while (!emptyQ(&Q))
    {
        v = deQueue(&Q); /* 取出队头元素 */
        VISIT(G, v);     /* 访问当前顶点 */
        for (p = G[v].link; p != NULL; p = p->next)
        {
            if (!Visited[p->adjvex])
            {
                Visited[p->adjvex] = 1; /* 标识某顶点入队 */
                enQueue(&Q, p->adjvex);
            }
        }
    }
}

void travelBFS(VLink G[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        Visited[i] = 0;
    for (i = 0; i < n; i++)
        if (!Visited[i])
            BFS(G, i);
    printf("\n");
}

