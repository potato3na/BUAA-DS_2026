#include<stdio.h>
#include<string.h>
#include<stdlib.h>
typedef struct{
    char name[50];
    char tel[50];
}contact;

int compare_names(const void *a, const void *b) {
    contact *p1 = (contact *)a;
    contact *p2 = (contact *)b;
    return strcmp(p1->name, p2->name);
}


int main(){
    int telenum=0;
    scanf("%d", &telenum);
    contact table[100];
    for(int i=0;i<telenum;i++){
            scanf("%s %s", table[i].name, table[i].tel);
    }

    for(int i=0;i<telenum;i++){
        for(int j=i+1;j<telenum;j++){
            if((strcmp(table[i].name,table[j].name)==0) && (strcmp(table[i].tel,table[j].tel)==0)){
                for(int k=j+1;k<telenum;k++){
                    table[k-1]=table[k];
                }
                telenum--;
                j--;
            }
        }
    }

    char rawname[100][50];

    for(int i=0;i<telenum;i++){
        int repeated=0;
        strcpy(rawname[i], table[i].name);
        for(int j=0;j<i;j++){
            if(strcmp(rawname[i],rawname[j])==0){
                if (strcmp(table[i].tel,table[j].tel)!=0)   repeated++;
            }
        }
        if(repeated>0){
            strcat(table[i].name, "_");
            int len= strlen(table[i].name);
            table[i].name[len] = repeated + '0';
            table[i].name[len + 1] = '\0';
        }
    
    }

    qsort(table,telenum,sizeof(contact),compare_names);

    for(int i=0;i<telenum;i++){
        printf("%s %s\n", table[i].name, table[i].tel);
    }
    
    return 0;
}
