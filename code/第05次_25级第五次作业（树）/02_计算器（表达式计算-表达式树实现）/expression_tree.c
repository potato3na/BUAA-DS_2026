#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#define DataType int
#define MAXSIZE 1000

int top=-1;
struct node *stack[MAXSIZE];
enum symtype {DATA, OP, EQ,LP,RP,OTHER};//符号类型
enum oper {PLUS = '+', MINUS = '-', MULT = '*', DIV = '/'};//操作符类型

struct tt {
      enum symtype ttype; 
      union sym {
           DataType data;
           enum oper op;
      }value;
};

//digit: t.value.data
//operater: t.value.op

struct node {
    struct tt token; 
    struct node *left, *right;
};

struct node *talloc(struct tt t);
struct node *insertET(struct tt t);
void push(struct node *p);
struct node *pop();
DataType computing(struct node *p);
DataType valuing(DataType d1, DataType d2, enum oper op);
int gettoken(struct tt *t);
void infixToPostfix(struct tt postfix[], int *postLen);
int priority(enum oper op);

int main()
{
    // struct tt t;
    // struct node *root;
    // DataType result;
    // while(gettoken(&t) != -1)//从后缀表达式中取一项
    //      insertET(t);
    // root = pop();  //最后栈中元素为树根
    // result = computing(root);
    // printf("%d\n", result);
    // return 0;
    struct tt postfix[MAXSIZE];
    int postLen=0;
    struct node *root;
    DataType result;
    infixToPostfix(postfix, &postLen);
    for(int i=0; i<postLen; i++) {
        insertET(postfix[i]);
    }  
    root = pop();  //最后栈中元素为树根
    result = computing(root);
    if(root->token.ttype == OP) {
        printf("%c", root->token.value.op);
    }
    else if(root->token.ttype == DATA) {
        printf("%d", root->token.value.data);
    }
    if(root->left!=NULL){
        if(root->left->token.ttype == OP) {
        printf(" %c", root->left->token.value.op);
        }
        else if(root->left->token.ttype == DATA) {
            printf(" %d", root->left->token.value.data);
        }
    }
    
    if(root->right!=NULL){
        if(root->right->token.ttype == OP) {
        printf(" %c", root->right->token.value.op);
        }
        else if(root->right->token.ttype == DATA) {
            printf(" %d", root->right->token.value.data);
        }
    }
    printf("\n");
    printf("%d\n", result);
    return 0;
}


int gettoken(struct tt *t){
    char ch;
    int num;
    do {
        ch = getchar();
        if (ch == EOF)
            return -1;
    } while (isspace(ch));
    
    if(isdigit(ch)){
        num=ch-'0';

        while((ch=getchar())!=EOF && isdigit(ch)){
            num=num*10+ch-'0';
        }
        if(ch!=EOF){
            ungetc(ch,stdin);   //如果不是数字，放回输入流
        }

        t->ttype=DATA;
        t->value.data=num;
        return 0;
    }
    else if(ch=='+'||ch=='-'||ch=='*'||ch=='/'){
        t->ttype=OP;
        t->value.op=ch;
        return 0;
    }
    else if(ch=='='){
        t->ttype=EQ;
        return -1;
    }
    else if(ch=='('){
        t->ttype=LP;
        return 0;
    }
    else if(ch==')'){
        t->ttype=RP;
        return 0;
    }
    else{
        t->ttype=OTHER;
        return 0;
    }
}
struct node *talloc(struct tt t)
{
    struct node *p;
    p= (struct node *)malloc(sizeof(struct node));
    p->token = t;
    p->left = p->right = NULL;
    return p;
};  //创建一个元素指针

struct node *insertET(struct tt t)
{
    struct node *p=NULL;
    if(t.ttype == DATA) {
        p = talloc(t);
        push(p);
     } 
    else if(t.ttype == OP) {
        p = talloc(t);
        p->right = pop();
        p->left = pop();
        push(p);
    }
    return p;
}


void push(struct node *p)
{
    if(top==MAXSIZE-1) {
        printf("Stack overflow\n");
        exit(1);
    }
    stack[++top] = p;
}

struct node *pop()
{
    if(top==-1) {
        printf("Stack underflow\n");
        exit(1);
    }
    return stack[top--];
}

DataType computing(struct node *p)
{
    DataType d1, d2;
    if( p->token.ttype == OP){
         d1 = computing(p->left);
         d2 = computing(p->right);
         return valuing(d1,d2, p->token.value.op);
    }
    else 
         return p->token.value.data;
}
  
DataType valuing(DataType d1, DataType d2, enum oper op)
{
    DataType tmp;
     switch(op) {
          case PLUS: tmp = d1 + d2; break;
          case MINUS: tmp = d1 - d2; break;
          case MULT: tmp = d1 * d2; break;
          case DIV: tmp = d1 / d2; break;
     }
     return tmp;
} 

void infixToPostfix(struct tt postfix[], int *postLen)
{
    struct tt t;
    struct tt opStack[MAXSIZE];
    int opTop = -1;
    int ret;

    *postLen = 0;

    while ((ret = gettoken(&t)) != -1) {

        if (t.ttype == DATA) {
            postfix[(*postLen)++] = t;
        }

        else if (t.ttype == LP) {
            opStack[++opTop] = t;
        }

        else if (t.ttype == RP) {
            while (opTop != -1 && opStack[opTop].ttype != LP) {
                postfix[(*postLen)++] = opStack[opTop--];
            }

            if (opTop != -1 && opStack[opTop].ttype == LP) {
                opTop--;
            }
        }

        else if (t.ttype == OP) {
            while (opTop != -1 &&
                   opStack[opTop].ttype == OP &&
                   priority(opStack[opTop].value.op) >= priority(t.value.op)) {
                postfix[(*postLen)++] = opStack[opTop--];
            }

            opStack[++opTop] = t;
        }
    }

    while (opTop != -1) {
        if (opStack[opTop].ttype == OP) {
            postfix[(*postLen)++] = opStack[opTop];
        }

        opTop--;
    }
}

int priority(enum oper op)
{
    if (op == MULT || op == DIV)
        return 2;
    else if (op == PLUS || op == MINUS)
        return 1;
    else
        return 0;
}
