#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cmd{
    int op;
    int pos;
    char str[512];
}cmd;

/*在pos位置处插入字符串*/
void insertString(char *old, const char *new, int pos){
    int len_old =strlen(old);
    int len_new = strlen(new);
    if(pos<0||pos>len_old) return;
    if(pos==len_old){
        strcat(old,new);
        return;
    }
    for(int i=len_old;i>=pos;i--){
        old[i+len_new]=old[i];  //expand new space >>
    }
    for(int i=0; i<len_new; i++){
        old[pos+i]=new[i];  //add new string
    }

}

/*在pos位置开始删除n个字符*/
void deleteString(char *old, int pos, int n) {
    int len_old = strlen(old);
    if (pos < 0||pos >= len_old||n <= 0) return;
    if (pos+n >= len_old) {
        old[pos]='\0';
        return;
    }
    int i;
    for (i=pos; old[i+n]!='\0'; i++) {
        old[i] = old[i+n];
    }
    old[i]='\0';
}

int top=-1;
int main(){
    cmd stack[512];
    char cur_str[512];
    fgets(cur_str,512,stdin);
    cur_str[strlen(cur_str)-1]='\0';  //eat \n
    int n;  //previous steps
    scanf("%d", &n);
    for(int i=0;i<n;i++){
        top++;
        scanf("%d %d %s", &stack[top].op, &stack[top].pos, stack[top].str);
        //op: 1-insert 2-delete 3-undo -1-end
        //here only 1&2
    }

    // char save_str[512];
    // strcpy(save_str, cur_str);
    // //restore the raw string
    // for(int i=top;i>=0;i--){
    //     if(stack[i].op==1){
    //         deleteString(save_str, stack[i].pos, strlen(stack[i].str));
    //     }
    //     else if(stack[i].op==2){
    //         insertString(save_str, stack[i].str, stack[i].pos);
    //     }
    // }

    /*before*/
    getchar();  //eat \n
    /*after*/
    int op;
    while(scanf("%d", &op) == 1){
        if(op==-1) break;
        else if(op==3){
            if(top>=0){
                if(stack[top].op == 1){
                deleteString(cur_str, stack[top].pos, strlen(stack[top].str));
                }
            else if(stack[top].op == 2){
                insertString(cur_str, stack[top].str, stack[top].pos);
                }
                top--;
            }
        }
        else{
            stack[++top].op=op;
            if(op==1){
                scanf("%d %s", &stack[top].pos, stack[top].str);
                insertString(cur_str, stack[top].str, stack[top].pos);
            }
            else if(op==2){
                int deletelen;
                scanf("%d %d", &stack[top].pos, &deletelen);

                for(int i=0;i<deletelen;i++){
                    stack[top].str[i]=cur_str[stack[top].pos+i];
                }
                stack[top].str[deletelen]='\0';

                deleteString(cur_str, stack[top].pos, deletelen);
            }
            }
    }

    // //execute all the commands
    // for(int i=0;i<=top;i++){
    //     switch(stack[i].op){
    //         case 1:
    //             insertString(save_str, stack[i].str, stack[i].pos);
    //             break;
    //         case 2:
    //             deleteString(save_str, stack[i].pos, strlen(stack[i].str));
    //             break;
    //     }
    // }

    // printf("%s", save_str);


    printf("%s", cur_str);
    printf("\n");

    return 0;
}
