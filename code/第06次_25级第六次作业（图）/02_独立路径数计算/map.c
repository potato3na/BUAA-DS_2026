#include <stdio.h>
#include <stdlib.h>
#define MAXSIZE 512
struct edge
{
    int eno;    // 边序号
    int adjvex; // 邻接顶点
    int weight; // 权值
    struct edge *next;
};

struct ver
{
    struct edge *link; // 指向该顶点邻接表的头指针
};
int target;
struct ver Graph[MAXSIZE];
char Visited[MAXSIZE] = {0};
int paths[MAXSIZE];
struct edge *insertEdge(struct edge *head, int eno, int avex);
void eDFS(int v, int top);
void printPath(int top);
int main()
{
    int eno, i, v1, v2;
    int vernum, edgenum;
    scanf("%d %d", &vernum, &edgenum);
    for (i = 0; i < edgenum; i++)
    {
        scanf("%d %d %d", &eno, &v1, &v2);
        Graph[v1].link = insertEdge(Graph[v1].link, eno, v2);
        Graph[v2].link = insertEdge(Graph[v2].link, eno, v1);
    }

    target = vernum - 1;
    Visited[0] = 1;
    eDFS(0, 0);
    return 0;
}
void eDFS(int v, int top)
{
    struct edge *p;
    if (v == target)
    {
        printPath(top);
        return;
    }
    for (p = Graph[v].link; p != NULL; p = p->next)
    {
        if (!Visited[p->adjvex])
        {
            paths[top] = p->eno;
            Visited[p->adjvex] = 1;
            eDFS(p->adjvex, top + 1);
            Visited[p->adjvex] = 0;
        }
    }
}

struct edge *insertEdge(struct edge *head, int eno, int avex)
{
    struct edge *e, *p;
    e = (struct edge *)malloc(sizeof(struct edge));
    if (e == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    e->eno = eno;
    e->adjvex = avex;
    e->weight = 1;
    e->next = NULL;

    if (head == NULL)
    {
        head = e;
        return head;
    }
    for (p = head; p->next != NULL; p = p->next)
        ;
    p->next = e;
    return head;
}

void printPath(int top)
{
    int i;
    for (i = 0; i < top; i++)
    {
        printf("%d ", paths[i]);
    }
    printf("\n");
}
