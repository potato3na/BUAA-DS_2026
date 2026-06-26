#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef char Datatype;

typedef struct BTNode
{
    Datatype data[100];
    struct BTNode *left;
    struct BTNode *right;
    int times;
} BTNode;

typedef BTNode *BTNodeptr;
BTNodeptr newBTNode(char word[]);
BTNodeptr insertBST(BTNodeptr root, char word[]);
void inorder(BTNodeptr root);
int main()
{
    FILE *fp;
    fp = fopen("article.txt", "r");
    if (fp == NULL)
    {
        printf("can't open file\n");
        return 0;
    }
    char word[20];
    BTNodeptr root = NULL;
    int len = 0;
    char ch;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (isalpha(ch))
        {
            word[len++] = tolower(ch);
        }
        else
        {
            if (len > 0)
            {
                word[len] = '\0';
                root = insertBST(root, word);
                len = 0;
            }
        }
    }

    if (len > 0)
    { // address ending alpha
        word[len] = '\0';
        root = insertBST(root, word);
    }

    fclose(fp);

    int printed = 0;
    if (root != NULL)
    {
        printf("%s", root->data);
        printed = 1;
    }
    if (root != NULL && root->right != NULL)
    {
        if (printed)
            printf(" ");
        printf("%s", root->right->data);
        printed = 1;
    }
    if (root != NULL && root->right != NULL && root->right->right != NULL)
    {
        if (printed)
            printf(" ");
        printf("%s", root->right->right->data);
    }
    printf("\n");
    inorder(root);
    return 0;
}
void inorder(BTNodeptr root)
{
    if (root == NULL)
        return;
    inorder(root->left);
    printf("%s %d\n", root->data, root->times);
    inorder(root->right);
}

BTNodeptr newBTNode(char word[])
{
    BTNodeptr p = (BTNodeptr)malloc(sizeof(BTNode));
    if (p == NULL)
    {
        printf("malloc failed in newBTNode");
        return NULL;
    }
    strcpy(p->data, word);
    p->left = NULL;
    p->right = NULL;
    p->times = 1;
    return p;
}

BTNodeptr insertBST(BTNodeptr root, char word[])
{
    if (root == NULL)
    {
        return newBTNode(word);
    }
    if (strcmp(word, root->data) == 0)
    {
        root->times++;
    }
    else if (strcmp(word, root->data) < 0)
    {
        root->left = insertBST(root->left, word);
    }
    else
    {
        root->right = insertBST(root->right, word);
    }
    return root;
}

