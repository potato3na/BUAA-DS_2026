#include <stdio.h>
#include <stdlib.h>
#define MAXNUM 120
typedef struct LNode{
    int pos;
    int barlen;
    struct LNode* next;
}LNode, *LinkList;

int main(){
    int idlenum;
    scanf("%d", &idlenum);
    LinkList curr=NULL,tail=NULL;

    for(int i=0; i<idlenum; i++){
        int barlen, barpos;
        scanf("%d %d", &barpos, &barlen);   
        LinkList p = (LinkList)malloc(sizeof(LNode));
        if(!p) return 1;
        p->pos = barpos;
        p->barlen = barlen;
        if(curr == NULL){
            curr = p;
            p->next = p;
            tail = p;
        }
        else{
            tail->next = p;
            tail = p;
            p->next = curr;
        }
    }

    int apply[MAXNUM];
    int cnt = 0;
    while(scanf("%d", &apply[cnt]) && apply[cnt] != -1){
        cnt++;
    }

    LinkList prev = tail;

    for(int i=0; i<cnt; i++){
        if(curr == NULL) break;
        LinkList p=curr;
        LinkList p_prev=prev;
        LinkList best=NULL;
        LinkList best_prev=NULL;
        do{
            if(p->barlen>=apply[i]){
                if( best==NULL || p->barlen < best->barlen){
                    best = p;
                    best_prev = p_prev;
                }
            }
            p_prev = p;
            p=p->next;
        }while(p!=curr);

        if(best!=NULL){
            if(best->barlen==apply[i]){
                if(best->next==best){
                    free(best);
                    curr = NULL;
                }
                else{
                    best_prev->next = best->next;
                    curr = best->next;
                    free(best);
                    prev = best_prev;
                }
            }
            else{
                best->barlen -= apply[i];
                curr = best;
                prev = best_prev;
            }

        }
    }

    //print left idle bars
    if(curr != NULL){
        LinkList p = curr;
        do
        {
            printf("%d %d\n", p->pos, p->barlen);
            p = p->next;
        } while (p!=curr);
        
    }
    return 0;
}
