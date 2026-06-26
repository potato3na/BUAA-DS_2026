#include<stdio.h>
#include<string.h>
int main(){
    char op='+';
    int multi=0, num=0, sum=0;

    //deal with the operator before
    while(scanf("%d",&num)==1){
        switch(op){
            case '+':
            multi+=num;
            break;
            
            case '-':
            multi-=num;
            break;
            
            case '*':
            multi*=num;
            break;
            
            case '/':
            multi/=num;
            break;
        }

        if(scanf(" %c",&op)!=1 || op == '=') break; //read the next operator
        else if(op=='+' || op=='-'){
            sum+=multi; //the part of * or / ends
            multi=0;
        }
    }

    sum+=multi; //the last calculation
    printf("%d\n",sum);

    return 0;   
}
