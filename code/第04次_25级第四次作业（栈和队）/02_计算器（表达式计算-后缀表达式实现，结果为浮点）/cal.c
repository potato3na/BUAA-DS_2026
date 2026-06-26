#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXSIZE 100
typedef double DataType;

enum symbol{num, op, eq, other};
enum oper {ept, add, min, mul, divi, lpar, rpar};   //ept is null
int Priority[]= {-1,0,0,1,1,2,2};   //correspond with enum oper

union sym{
    DataType num;
    enum oper op;
};

enum symbol getSym(union sym *item);
void operate(enum oper op);     //process the operator
void compute(enum oper op);     //compute
void pushNum(DataType num);     //push the data
DataType popNum();              //pop the data
void pushOp(enum oper op);      //push the operator
enum oper popOp();              //pop the operator
enum oper topOp();              //get the top operator in the stack
void error(char s[]);           //process the error

DataType Num_Stack[MAXSIZE];    //the num stack
int Ntop = -1;
enum oper Op_Stack[MAXSIZE];    //the operator stack
int Otop = -1;

int main(){
    union sym item;
    enum symbol s;
    while((s=getSym(&item))!=eq){    //keep reading op until meet '='
        if(s==num){
            pushNum(item.num);
        }
        else if(s==op){
            operate(item.op);
        }
        else{
            printf("Error in the expression!\n");
        }
    }
    while(Otop>=0){
        compute(popOp());
    }

    if(Ntop==0){
        printf("%.2f\n", popNum());
    }
    else{
        printf("Error in the expression!\n");
    }
    return 0;
}

enum symbol getSym(union sym *item){
    int c,n;
    while((c=getchar())!='='){
        if (c>='0' && c<= '9'){
            for(n=0;c>='0' && c<='9'; c=getchar())
                n=n*10+c-'0';   //translate char to num
            ungetc(c, stdin);
            item->num=n;
            return num;
        }
        else{
            switch(c){
                case '+': item->op = add; return op;
                case '-': item->op = min; return op;
                case '*': item->op = mul; return op;
                case '/': item->op = divi; return op;
                case '(': item->op = lpar; return op;
                case ')': item->op = rpar; return op;
                case ' ': case '\t': case '\n': break;
                default: return other;
            }
        }
    }
    return eq;
}

void operate(enum oper op){
    enum oper t;
    if(op!=rpar){
        /*
        1.the stack is not empty
        2.the current op priority <= stack top op priority
        3.stack top is not lpar
        */
        while( Otop >=0 && Priority[op] <= Priority[topOp()] && topOp() != lpar){
            compute(popOp());
        }
        pushOp(op);
    }
    else{
        while((t=popOp())!=lpar)    compute(t);
    }
}

void compute(enum oper op){
    DataType tmp;
    switch(op){
        case ept:
            break;
        case add:
            pushNum(popNum()+popNum());
            break;
        case min:
            tmp = popNum();     //the one popped first is the right operand.
            pushNum(popNum()-tmp);
            break;
        case mul:
            pushNum(popNum()*popNum());
            break;
        case divi:
            tmp=popNum();
            pushNum((double)popNum()/(double)tmp);   //the one popped first is the right operand.
            break;
    } 
}

void pushNum(DataType num){
    if(Ntop == MAXSIZE -1)  error("Data stack is full >_<");
    Num_Stack[++Ntop] = num;
}

DataType popNum(){
    if(Ntop == -1)  error("Error in the expression!");
    return Num_Stack[Ntop--];
}

void pushOp(enum oper op){
    if(Otop == MAXSIZE -1)  error("Operator stack is full >_<");
    Op_Stack[++Otop] = op;
}
enum oper popOp(){
    if(Otop == -1)  error("Error in the operator!");
    return Op_Stack[Otop--];
}
enum oper topOp(){
    return Op_Stack[Otop];
}
void error(char s[]){
    fprintf(stderr, "%s\n", s);
    exit(1);
}
