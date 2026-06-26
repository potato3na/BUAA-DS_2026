#include <stdio.h>
#include <string.h>

#define MAXDEFINE 100
#define MAXCALLFUNC 10
#define MAXNAME 20
#define MAXSIZE 200
int Top=-1;

struct Record{
	char funcName[MAXNAME];		//record current function name
    int usedFunc[MAXCALLFUNC];		//record the index of all the function it called
	int usedlen;		//dynamic call count
}recordlist[MAXDEFINE];

int func_cnt=0;
int STACK[MAXSIZE];		//store the index of using function

void checkUsed();
int search_func(char s[]);
void initStack();
int isEmpty();
int isFull();
void push(int s[], int item);
int pop(int s[]);

int main(){
	char func_name[MAXNAME];
	int op;
   do{
	scanf("%1d", &op);
	if(op==8){
		scanf("%s", func_name);
		int cur_idx=search_func(func_name);
		if(cur_idx==-1){
			strcpy(recordlist[func_cnt].funcName,func_name);
			recordlist[func_cnt].usedlen=0;
			push(STACK,func_cnt);
			func_cnt++;
		}
		else	push(STACK, cur_idx);

		checkUsed();  
	}
	else if(op==0){
		pop(STACK);
	}

	else{
		printf("expressnion error\n");
	}
	
   }while(Top!=-1); 

   for(int i=0; i<func_cnt;i++){
	if(recordlist[i].usedlen==0)	continue;	//the function does not call any function
	printf("%s:", recordlist[i].funcName);
	for(int j=0; j<recordlist[i].usedlen;j++){
		int prt=recordlist[i].usedFunc[j];
		printf("%s",recordlist[prt].funcName);
        if(j < recordlist[i].usedlen - 1) {
            printf(",");
        } else {
            printf("\n");
        }
	}
   }



   return 0;
}

void checkUsed(){
	if(Top<1)	return;	//the function called is main function
	int index=STACK[Top-1];	//the index of the function which calls the current function
	int index2=STACK[Top];	//the index of the current function
	for(int i=0; i<recordlist[index].usedlen;i++){
		if(recordlist[index].usedFunc[i]==index2)
			return;			//the callee have been recorded
	}
	/*add the callee function*/
	recordlist[index].usedFunc[recordlist[index].usedlen]=index2;
	recordlist[index].usedlen++;
}

int search_func(char s[]){	//return the index of the function
	for(int i=0; i<func_cnt; i++){
		if(strcmp(recordlist[i].funcName, s)==0)
			return i;
	}
	return -1;	//not found
}


void initStack()
{
	Top=-1;
}

int isEmpty(){
	return Top==-1;
}

int isFull(){
	return Top==MAXSIZE-1;
}

void push(int s[], int item){
	if(isFull())
		printf("error ");
	else
		s[++Top]=item;
}

int pop(int s[]){
	if(isEmpty())
		printf("error ");
	else
		return s[Top--];
}
