#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
typedef struct{
    char word[50];
    int freq;
}WList;

WList dictionary[2000];
int word_cnt=0;
int qst_cmp(const void *a, const void *b){
    WList *w1=(WList *)a;
    WList *w2=(WList *)b;
    return strcmp(w1->word, w2->word);
}


void add_word(char *temp){
    for(int i=0; i<word_cnt; i++){
        if(strcmp(dictionary[i].word,temp)==0){
            dictionary[i].freq++;
            return;
        }
    }
    strcpy(dictionary[word_cnt].word,temp);
    dictionary[word_cnt++].freq=1;
}

FILE *fin, *fout;
int main(){
    if((fin=fopen("article.txt","r"))==NULL) return 0;
    int ch;
    int index=0;
    char temp_word[50];
    while((ch=fgetc(fin))!=EOF){
        if((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')){
            temp_word[index++]=tolower(ch);
        }
        else{
            if(index>0){
                temp_word[index]='\0';
                add_word(temp_word);
                index=0;
            }
        }
    }

    if(index>0){
        temp_word[index]='\0';
        add_word(temp_word);      
    }


    fclose(fin);
    qsort(dictionary, word_cnt, sizeof(WList), qst_cmp);
    for(int i=0;i<word_cnt;i++){
        printf("%s %d\n", dictionary[i].word, dictionary[i].freq);
    }
    return 0;
}


