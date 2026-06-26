#include<stdio.h>
#define MAXSIZE 100
#define ElemType int
ElemType STACK[MAXSIZE];
int op, Top, num;
void initStack();
int isEmpty();
int isFull();
void push();
ElemType pop();

int main(){
    initStack();
    while(scanf("%d", &op)!=EOF){

        if(op==-1) break;
        else if(op==1){
            scanf("%d", &num);
            push(STACK,num);
        }
        else if(op==0){
	        if(isEmpty())    printf("error ");
            else    printf("%d ", pop(STACK));
        }
    }

}

void initStack()
{
	Top=-1;
}

int isEmpty(){
	return Top==-1;
}

int isFull(){
	return Top==MAXSIZE-1;
}

void push(ElemType s[], ElemType item){
	if(isFull())
		printf("error ");
	else
		s[++Top]=item;
}

ElemType pop(ElemType s[]){
	if(isEmpty())
		printf("error ");
	else
		return s[Top--];
}
