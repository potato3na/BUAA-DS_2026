#include <stdio.h>
#include <string.h>
#define MAXLINE 1024
FILE *fin,*fout;
int used[26]={0}; //create a hash table to record the used characters
char psw[MAXLINE],key[26];
int cnt=0;
int main(){

    while(scanf("%s",psw)!=EOF){
        break;
    }

    for(int i=0;psw[i]!='\0';i++){
        if(used[psw[i]-'a']==0){
            used[psw[i]-'a']=1;
            key[cnt++]=psw[i];
        }
    }

    for(int i=25;i>=0;i--){
        if(used[i]==0){
            key[cnt++]=i+'a';
        }
    }


    if((fin=fopen("encrypt.txt","r"))==NULL)
        return 0;
    if((fout=fopen("output.txt","w"))==NULL){
        fclose(fin);
        return 0;
    }
    
    int ch;
    while((ch=fgetc(fin))!=EOF){
        if(ch>='a' && ch<='z'){
            fputc(key[ch-'a'],fout);
        }
        else{
            fputc(ch,fout);
        }
    }

    fclose(fin);
    fclose(fout);
    return 0;
}
