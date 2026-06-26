#include <stdio.h>
#include <stdlib.h>
typedef struct LNode {
    int data;
    int times;
    struct LNode* link;
} LNode, *LinkList;


int find_min(LinkList curr);
void delete_min(LinkList curr);

LinkList head=NULL;
LinkList curr=NULL;
int main(){
    int all_num=0;

    scanf("%d", &all_num);
    /*creat a LinkList (len=all_num)*/
    for(int i=0;i<all_num;i++){
        int num;
        scanf("%d", &num);
        if(i==0){
            head=(LinkList)malloc(sizeof(LNode));
            head->data=num;
            head->link=head;
            curr=head;
            continue;
        }
        LinkList p=(LinkList)malloc(sizeof(LNode));
        p->data=num;
        p->link=head;
        curr->link=p;
        curr=p;
    }
    while(head!=NULL)   delete_min(head);
    return 0;
}


int find_min(LinkList curr){
    LinkList q=curr;
    int min=curr->data; 
    do{
        q=q->link;
        if(q->data<min) min=q->data;
    }while(q!=curr);
    return min;
}

void delete_min(LinkList curr){
    if(curr->link==curr){
        printf("%d \n", curr->data);
        head=NULL;
        free(curr);
        return;
    }

    int min=find_min(curr);
    LinkList q=curr,prev=curr;

    while(q->data!=min){
        printf("%d ", q->data);
        prev=q;
        q=q->link;
    }
    printf("%d ", q->data);
    printf("\n");

    /*delete the min item*/
    LinkList temp=q;
    if(prev!=q){
        prev->link=q->link;
        free(temp);
    }
    else{
        prev=q;
        /*find tail*/
        while(prev->link!=q) prev=prev->link;
        if(prev!=NULL)  prev->link=q->link;
        free(temp);
    }
    head=prev->link;   //new head
}
