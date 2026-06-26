#include <stdio.h>
#include <string.h>

int main() {
    int len=0;
    char words[100][100];
    while(scanf("%s",words[len])!=EOF){
        len++;
    }

    int wordslen[100]={0};  //record the length of every word
    int letternum[26]={0}; //record the frequency of every letter
    char letter;
    for(int i=0;i<len; i++){
        wordslen[i]=strlen(words[i]);
        for(int j=0;j<wordslen[i];j++){
            letter = words[i][j];
            if(letter>='a'&&letter<='z'){
                letternum[letter-'a']++;
            }
        }
    }
    int maxtimes=0;
    for(int i=0;i<26;i++)
    {
        if(letternum[i]>maxtimes){
            maxtimes=letternum[i];
        }
    }
    for(int y=maxtimes;y>=0;y--){
        for(int x=0;x<26;x++){
            if(y==0){
                printf("%c",'a'+x);                
            }
            else{
                if(letternum[x]>=y)
                    printf("*");
                else
                    printf(" ");
            }
        }
        printf("\n");
    }


    return 0;


}




