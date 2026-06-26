#include <stdio.h>
#include <stdlib.h>
typedef struct LNode {
    int data;
    int times;
    struct LNode* link;
} LNode, *LinkList;

int op, insertflag=0, cmp_cnt=0;
LinkList list=NULL;

void search_node(){ //is the item repeated?
    LinkList s=NULL,prev=list;
    for(s=list;s!=NULL;prev=s,s=s->link){
        cmp_cnt++;
        if(s->data == op){
            if(s==list){
                s->times++;
                return;
            }
            prev->link=s->link;
            s->link=list;
            list=s;
            s->times++;
            insertflag=1;
            return;
        }
    }
    //if didnt find op:

//    if(!insertflag){
    LinkList p=(LinkList)malloc(sizeof(LNode));
    p->data = op;
    p->times = 1;
    prev->link = p;
    p->link=NULL;
    // }
};



int main(){

    LinkList curr=NULL;
    curr=list;
    int all_num=0;
    scanf("%d", &all_num);

    while(all_num--){

        scanf("%d", &op);

        if(list==NULL){
            LinkList p=(LinkList)malloc(sizeof(LNode));
            list=p;
            list->data=op;
            list->times=1;
            list->link=NULL;
            continue;
        }

        search_node();

        insertflag=0;   //reset

    }

    LinkList haha = NULL;
    int len=0;
    printf("%d\n", cmp_cnt);
    for(haha=list, len=0; (len<5)&&(haha!=NULL);haha=haha->link,len++){
        printf("%d %d\n", haha->data, haha->times);
    }
    return 0;
}
