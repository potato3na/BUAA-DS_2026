#include <stdio.h>
#include <stdlib.h>
typedef struct LNode {
    int data;
    struct LNode* link;
} LNode, *LinkList;

int op;
LinkList list=NULL;

/**
 * 函数功能：在有序递增链表中插入数据项，保持有序
 * 调用方式：list = insertSorted(list, item); // 必须接收返回值，因为可能插在头部
* 时间复杂度：O(n)
 */
LinkList insertSorted(int item) {
    LinkList p = NULL, q = NULL, r = NULL;
    r = (LinkList)malloc(sizeof(LNode)); // 创建一个数据项为item的新结点
    r->data = item;  
    r->link = NULL;
    
    if(list == NULL)     /* 情况1：list是一个空表 */
        return r;
        
    // 寻找插入位置：p用于遍历，q始终指向p的前驱结点
    for(p = list; p != NULL && item > p->data; q = p, p = p->link) 
        ; 
        
    if( p == list ){     /* 情况2：在首结点前插入 (item比所有元素都小) */
        r->link = p;
        return r;        // 此时头指针发生了改变
    } else {             /* 情况3：在结点q后插入一个结点 (中间或尾部) */
        q->link = r;
        r->link = p;
    }
    return list;
}

int Count_Weight(){
    LinkList p=NULL, q=NULL, r=NULL;
    int weight_all=0;
    for(p=list; p!=NULL; p=p->link){
        weight_all+=p->data;
    }
    return weight_all;
}


int Below10_Weight(){
    int small_weight=0;
    LinkList p=list,q=NULL;

    while(p!=NULL){
        if(p->data<10){
            small_weight+=p->data;
            LinkList temp=p;
            if(q==NULL){
                list=list->link;
                p=list;
                free(temp);
            }   //if the deleted one is the head
            else{
                q->link=p->link;
                p=p->link;
                free(temp);
            }
        }
        else{
        q=p;
        p=p->link;
     }
    }
    return small_weight;
}

void printList(LinkList head){
    while(head){
        printf("%d ", head->data);
        head=head->link;
    }
    printf("\n");
}

int main() {

    LinkList p=NULL,q=NULL;
    // list=(LinkList)malloc(sizeof(LNode));
    while(scanf("%d", &op)!=EOF){
        if(op==999999){
            break;
        }
        else if(op>0){
            list=insertSorted(op);
        }
        else if(op<0 && op>=(-1)*Count_Weight()){
            int cnt=0;
            while(cnt<(-op) && list != NULL){
                cnt+=list->data;
                LinkList temp=list;
                list=list->link;
                free(temp); //read one delete one
                if(cnt>(-op)){
                    int left=cnt+op;    //op<0
                    list=insertSorted(left);
                    break;
                }

            }
        }
        else if(op==0){
            int a=Below10_Weight();
            if(a>0){
                list=insertSorted(a);
            }
        }

    }
    printList(list);
    return 0;
}
