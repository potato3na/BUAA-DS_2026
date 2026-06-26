#include<stdio.h>
#include<string.h>
int main()
{
    int len=0;
    int explen=0;   //the '-' expansion length
    char words[1000];
    if(fgets(words,sizeof(words),stdin)!=NULL){
        len=strlen(words);
        for(int i=0;i<len-2;i++){
            if(words[i+1]=='-'){
                if((words[i+2]<='z'&&words[i]>='a'&&words[i+2]>words[i])||
                (words[i+2]<='Z'&&words[i]>='A'&&words[i+2]>words[i])||
                (words[i+2]<='9'&&words[i]>='0'&&words[i+2]>words[i])){
                    explen=words[i+2]-words[i];
                    for(int j=len-1;j>=i+2;j--){
                        words[j+(explen-1)-1]=words[j];
                    }
                    for(int k=i+1;k<=i+explen;k++){
                        words[k]=words[i]+k-i;
                    }
                    len+=explen-1;
                    words[len-1] = '\0'; 
                }
            }
        }
            printf("%s",words);
        printf("\n");
    }
    return 0;

}
