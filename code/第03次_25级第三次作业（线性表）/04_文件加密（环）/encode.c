#include <stdio.h>
#include <stdlib.h>
FILE *fin, *fout;
char key[40];
int usedkey[100]={0};
char newkey[40];
char trans[100];

int del_repeat_letter(char* words){
    int cnt = 0;
    while(*words != '\0'){
        if(*words >= 32 && *words <= 126){
            if(usedkey[*words - 32] == 0){
                newkey[cnt++] = *words;
                usedkey[*words - 32] = 1;
            }
        }
        words++;
    }
    newkey[cnt] = '\0';
    return cnt;
}

char process(char c){
    if(c<32 || c>126) return (char)c;
    else return trans[c-32];
}

typedef struct LNode{
    char data;
    struct LNode* link;
}LNode,*LinkList;


int main(){

    fgets(key, 40, stdin);
    int cnt = del_repeat_letter(key);

    if((fin=fopen("in.txt","r"))==NULL)
        return 0;
    if((fout=fopen("in_crpyt.txt","w"))==NULL){
        fclose(fin);
        return 0;
    }
    LinkList head = NULL, tail = NULL;
    for(int i = 0; i < cnt; i++){
        LinkList newNode = (LinkList)malloc(sizeof(LNode));
        newNode->data = newkey[i];
        if(head == NULL) {
            head = newNode;
            tail = newNode;
        }
        else {
            tail->link = newNode;
            tail = newNode;
        }
    }

    for(int i=0;i<95;i++){
        if(usedkey[i]==0){
            LinkList newNode = (LinkList)malloc(sizeof(LNode));
            newNode->data = (char)(i+32);
            if(head == NULL) {
                head = newNode;
                tail = newNode;
            }
            else {
                tail->link = newNode;
                tail = newNode;
            }
        }
    }
    tail->link = head; //make it a ring
    //now we've created the key ring
    //list is the beginning

    LinkList curr = head, prev=tail;

    for(int i = 0; i < 127-32-1; i++){
        char a = curr->data;
        int step = a-1; //record the step

        //to delete curr node:
        LinkList del=curr;
        prev->link = curr->link;
        curr = curr->link;
        free(del);

        while(step--){
            prev = curr;
            curr = curr->link;
        }
        trans[a-32] = curr->data;
    }

    trans[curr->data - 32] = newkey[0]; 
    free(curr);
    
    int ch;
    while((ch=fgetc(fin))!=EOF){
        fputc(process(ch), fout);
    }

    fclose(fin);
    fclose(fout);
    
    return 0;
}
