#include <stdio.h>
#include <string.h>
#define to_lower(c) (c>='A' && c<='Z' ? c+'a'-'A':c)
#define MAXLINE 1024

void replace(char line[],char raw[], char rep[],FILE *fout){
    int l=strlen(line);
    int rl=strlen(raw);
    for (int i = 0; i < l;)
    {
        int match=1;
        for (int j = 0; j < rl; j++)
        {
            if(i+rl>l){ //if the rest of line is shorter than raw, then it can't be matched
                match=0;
                break;
            }
            
            else if(to_lower(line[i+j])!=to_lower(raw[j])){ 
                match=0;
                break;
            }
        }
        if(match){
            fputs(rep, fout);
            i+=rl; //jump the replaced raw length
        }
        else{
            fputc(line[i],fout);
            i++; //jump the current character
        }
    }
}
    
int main(){
    char raw[MAXLINE], rep[MAXLINE], in[MAXLINE];
    FILE *fin,*fout;
    scanf("%s", raw);
    scanf("%s", rep);
    if((fin=fopen("filein.txt","r"))==NULL)
        return 0;
    if((fout=fopen("fileout.txt","w"))==NULL){
        fclose(fin);
        return 0;
    }

    while(fgets(in,MAXLINE,fin)!=NULL){
        replace(in,raw,rep,fout);
    }

    fclose(fin);
    fclose(fout);

    return 0;
}
