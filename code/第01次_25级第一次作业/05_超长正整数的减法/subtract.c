#include <stdio.h>
#include <math.h>
#include <string.h>
void long_minus(char numlong[], char numshort[], int len_l, int len_s);
int main(){
    char raw_num[100],raw_num2[100];
    int len = 0, flag = 0, len2 = 0, flag2 = 0;
    if(fgets(raw_num,sizeof(raw_num),stdin)!=NULL){
        len = strlen(raw_num)-1;  //record the first num, and delete the '\n'
    }
    if(fgets(raw_num2,sizeof(raw_num2),stdin)!=NULL){
        len2 = strlen(raw_num2)-1;  //record the second num, and delete the '\n'
    }  
        while(raw_num[flag] == '0'){
            flag++;
        }
        while(raw_num2[flag2] == '0'){
            flag2++;
        }

        
        if(flag!=0){
            for(int i=0;i<len-flag;i++){
                raw_num[i]=raw_num[i+flag];
                // printf("%c",raw_num[i]);
            }

            len-=flag;
            raw_num[len] = '\0';
            // printf("\n");
        }

        if(flag2!= 0){
            for(int i=0;i<len2-flag2;i++){
                raw_num2[i]=raw_num2[i+flag2];
                // printf("%c",raw_num2[i]);
            }
            // printf("\n");
            len2-=flag2;
            raw_num2[len2] = '\0';
        }

        if(len> len2){
            long_minus(raw_num, raw_num2, len, len2);
        }

        else if (len< len2){
            printf("-");    
            long_minus(raw_num2, raw_num, len2, len);
        }

        else{   
            if(!strcmp(raw_num, raw_num2))  printf("0\n");
            else if (strcmp(raw_num, raw_num2)>0){
                long_minus(raw_num, raw_num2, len, len2);
            }
            else if (strcmp(raw_num, raw_num2)<0){
                printf("-");
                long_minus(raw_num2, raw_num, len2, len);
            }
        }




}

void long_minus(char numlong[], char numshort[], int len_l, int len_s){
    int diff = len_l - len_s, borrow=0;
    int result[100]={0};
    int i=len_l - 1;
    int j=len_s - 1;
    int k=0;

    while (i >= 0) {
        int dl = numlong[i] - '0';
        int ds = (j >= 0) ? (numshort[j] - '0') : 0; 
        diff = dl - ds - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result[k] = diff;
        k++;
        i--;
        j--;
    }

    while (k>1 && result[k - 1] == 0) {
        k--;
    }
    while(k--){
        printf("%d", result[k]);
    }
    printf("\n");
}



