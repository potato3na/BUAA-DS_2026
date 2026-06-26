#include<stdio.h>
#include<string.h>
#include<math.h>
int main(){
    char raw_num[100];
    int len=0,flag=0;
    float num=0;
    if(fgets(raw_num,sizeof(raw_num),stdin)!=NULL){
        len=strlen(raw_num)-1;
    }
    for(int i=0;i<len;i++){
        if(raw_num[i]=='.')
            flag = i;   //eg: rawnum=12.689,flag=2
    }

    int j=0;
    while(raw_num[j]=='0'||raw_num[j]=='.')
        j++;

    int quan=flag-j;
    if(quan>0)
        quan--;

    if (j==len-1)
        printf("%ce%d\n",raw_num[j],quan);
    else{
        printf("%c.",raw_num[j]);
        for(int i=j+1;i<len;i++){
            if(raw_num[i]!='.')
                printf("%c",raw_num[i]);
        }
        printf("e%d\n",quan);
    }

    return 0;
}
