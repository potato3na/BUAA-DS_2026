#include<stdio.h>
int is_valid(int r, int c) {
    return r >= 0 && r < 19 && c >= 0 && c < 19;
}

int main()
{
    int board[25][25] = {0};
    int win=0,mark=0,k=0,l=0;
    int start1,start2;
    for(int i=0;i<19;i++){
        for(int j=0;j<19;j++)
            scanf("%d", &board[i][j]);
    }


    for(int i=0;i<19;i++){
        for(int j=0; j<19; j++){
            if(board[i][j]==0) continue;
            mark=board[i][j];

            if(j+3<19){
                win=1;
                for(k=j+1;k<j+4;k++)    //search rightside
                    if(board[i][k]==mark)   win++;
                    
                if (win==4){
                    if((j>0 && board[i][j-1]==0) || (j+4<19 && board[i][j+4]==0)){
                        printf("%d:%d,%d\n",mark,i+1,j+1);
                        return 0;
                    }
                }
                   
            }

            if(i+3<19){
                win=1;
                for(k=i+1;k<i+4;k++)    //search rightside
                    if(board[k][j]==mark)   win++;
                    
                if (win==4){
                    if((i>0 && board[i-1][j]==0) || ( i+4<19 && board[i+4][j]==0)){
                        printf("%d:%d,%d\n",mark,i+1,j+1);
                        return 0;
                    }
                }
                   
            }

            //search left cross
            if(i+3<19 && j-3>=0){
                win=1;
                for(int k=1;k<4;k++){
                    if(board[i+k][j-k]==mark) win++;
                }
                if(win == 4){
                    if(( i-1>=0 && j+1<=18 && board[i-1][j+1]==0)||(i+4<=18 && j-4>=0) && board[i+4][j-4]==0){
                        printf("%d:%d,%d\n", mark, i + 1, j + 1);
                        return 0;
                    }
                }
            }

            
            //search right cross
            if(i+3<19 && j+3<19){
                win=1;
                for(int k=1;k<4;k++){
                    if(board[i+k][j+k]==mark) win++;
                }
                if(win == 4){
                    if(( i+4<=18 && j+4<=18 && board[i+4][j+4]==0)||(i-1>=0 && j-1>=0) && board[i-1][j-1]==0){
                        printf("%d:%d,%d\n", mark, i + 1, j + 1);
                        return 0;
                    }
                }
            }

        
            
        }
    }
    printf("No\n");
    return 0;
}

