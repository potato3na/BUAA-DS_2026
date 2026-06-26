#include<stdio.h>
#include<string.h>
#include<stdlib.h>
FILE *fin,*fout;
int op=1;   //default
typedef struct{
    char name[55];
    char author[25];
    char publisher[35];
    char date[15];
}book;

book rlib[505],nlib[505];

int compare_books(const void *a, const void *b){
    book *p1=(book *)a;
    book *p2=(book *)b;
    return strcmp(p1->name,p2->name);
}

int cmp_book(const book *a, const book *b){
    if(strcmp(a->name,b->name)!=0)  return strcmp(a->name,b->name);
    else if(strcmp(a->author,b->author)!=0)  return strcmp(a->author,b->author);
    else if(strcmp(a->publisher,b->publisher)!=0)  return strcmp(a->publisher,b->publisher);
    else return strcmp(a->date,b->date);
}

int main(){

    if((fin=fopen("books.txt","r"))==NULL)  return 0;
    if((fout=fopen("ordered.txt","w"))==NULL) {
        fclose(fin);
        return 0;
    }

    int num=0;

    while(fscanf(fin,"%s %s %s %s", rlib[num].name, rlib[num].author, rlib[num].publisher, rlib[num].date)!=EOF){
        num++;
    }


    qsort(rlib,num,sizeof(book),compare_books);

    while(scanf("%d",&op)!=EOF){
        if(op==0) break;
        switch(op){
            case 1: //add
            {
                book newbook;
                scanf("%s %s %s %s", newbook.name, newbook.author, newbook.publisher, newbook.date);
                int l = 0, r = num;
                while (l < r) {
                    int m = (l + r) / 2;
                    if (cmp_book(&rlib[m], &newbook) < 0) l = m + 1;
                    else r = m;
                }   //use the binary search to find the position to insert the new book
                for (int j = num; j > l; --j) rlib[j] = rlib[j - 1];
                rlib[l] = newbook;
                num++;

                break;
            }
            case 2: //search
            {
                char searchname[55];
                scanf("%s", searchname);
                for(int i=0;i<num;i++){
                    if(strstr(rlib[i].name,searchname)!=NULL){
                        printf("%-50s%-20s%-30s%-10s\n", rlib[i].name, rlib[i].author, rlib[i].publisher, rlib[i].date);
                    }
                }
                break;
            }

            case 3: //delete
            {
                char delname[55];
                scanf("%s", delname);
                for(int i=0;i<num;i++){
                    if(strstr(rlib[i].name,delname)!=NULL){
                        for(int j=i+1;j<num;j++){
                            rlib[j-1]=rlib[j];
                        }
                        i--;
                        num--;
                    }
                }
                break;
            }
        }
    }

    for(int i=0;i<num;i++){
        fprintf(fout,"%-50s%-20s%-30s%-10s\n", rlib[i].name, rlib[i].author, rlib[i].publisher, rlib[i].date);
    }

    fclose(fin);
    fclose(fout);

    return 0;
}

