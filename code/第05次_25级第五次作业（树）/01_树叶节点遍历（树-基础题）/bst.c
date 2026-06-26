//二叉查找树
#include <stdio.h>
#include <stdlib.h>


typedef struct bst
{
    int data;
    struct bst *left;
    struct bst *right;
}BTNode;

void print_leaf_elements(BTNode *t, int current_level);
BTNode *searchBST(BTNode *t, int key);
BTNode *insertBST(BTNode *p, int item);

int main(){
    BTNode *root=NULL;
    int level0=1;
    int ele_num;
    int ele;
    scanf("%d", &ele_num);
    for(int i=0;i<ele_num;i++){
        scanf("%d", &ele);
        if(i==0){
            root=insertBST(root,ele);
        }
        else{
            insertBST(root,ele);
        }
    }
    print_leaf_elements(root, level0);
    return 0;
}

void print_leaf_elements(BTNode *t, int current_level) {
    int level;
    if (t != NULL) {
        level=current_level;
        if (t->left == NULL && t->right == NULL) {
            printf("%d %d\n", t->data, level);
        }
        level++;
        print_leaf_elements(t->left, level);
        print_leaf_elements(t->right, level);
    }
}

BTNode *searchBST(BTNode *t, int key) {
    while (t != NULL) {
        if (key == t->data)
            return t;
        else if (key < t->data)
            t = t->left;
        else
            t = t->right;
    }
    return NULL;
}


BTNode *insertBST(BTNode *p, int item) {
    if (p == NULL) {
        p = malloc(sizeof(BTNode));
        p->data = item;
        p->left = p->right = NULL;
    } else if (item < p->data) {
        p->left = insertBST(p->left, item);
    } else if (item >= p->data) {
        p->right = insertBST(p->right, item);
    } 
    
    // else {
    //     // 相等时按题意处理：忽略、计数、或插入右子树
    // }
    return p;
}


