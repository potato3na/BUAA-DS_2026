#include <stdio.h>
#include <stdlib.h>

typedef struct LNode {
    int data[2];
    struct LNode* link;
} LNode, *LinkList;


int main(){

    int base, exp;
    LinkList head, head2, head3, q, p;
    
    //create table 1
    head = (LinkList)malloc(sizeof(LNode));
    if (head == NULL) {
        return 1;
    }
    head->link = NULL;
    q = head;

    do{
        scanf("%d %d", &base, &exp);
        p = (LinkList)malloc(sizeof(LNode));
        if (p == NULL) {
            return 1;
        }
        p->data[0] = base;
        p->data[1] = exp;
        p->link = NULL;

        q->link = p;
        q = p;
    }while(getchar()!='\n');


    //create table 2
    head2 = (LinkList)malloc(sizeof(LNode));
    if (head2 == NULL) {
        return 1;
    }
    head2->link = NULL;
    q = head2;

    do {
        scanf("%d %d", &base, &exp);
        p = (LinkList)malloc(sizeof(LNode));
        if (p == NULL) {
            return 1;
        }
        p->data[0] = base;
        p->data[1] = exp;
        p->link = NULL;

        q->link = p;
        q = p;
    }while(getchar()!='\n');

    //create result table
    head3 = (LinkList)malloc(sizeof(LNode));
    if(head3 == NULL) {
        return 1;
    }
    head3->link = NULL;
    LinkList op1,op2;
    op1 = head->link;
    while(op1 != NULL) {
        op2 =  head2->link;
        while(op2 != NULL){
            p=(LinkList)malloc(sizeof(LNode));
            if (p == NULL) {
                return 1;
            }
            p->data[0] = op1->data[0] * op2->data[0];
            p->data[1] = op1->data[1] + op2->data[1];
            p->link = NULL;


            LinkList prev = head3;
            q = head3->link;
            
            while(q!=NULL && q->data[1] > p->data[1]){
                prev = q;
                q = q->link;
            }

            //prev is the position before the position to insert p

                if(q!=NULL && q->data[1] == p->data[1]){
                    q->data[0] += p->data[0];
                    free(p);
                }
                else{
                    prev->link = p;
                    p->link = q;
                }
            
            op2 = op2->link;
        }
        op1 = op1->link;
    }

    //print result
    q = head3->link;
    while(q!=NULL){
        if(q->data[0] != 0){
            printf("%d %d ", q->data[0], q->data[1]);
        }
        q = q->link;
    }
    printf("\n");
    return 0;

}

