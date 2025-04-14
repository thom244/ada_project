#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R 256

typedef struct HuffmanNode{
    char c;
    int freq;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
}HuffmanNode;

HuffmanNode *createNode(char c, int freq, HuffmanNode *left, HuffmanNode *right){
    HuffmanNode *node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->c = c;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

int cmp(const void* a, const void* b){
    HuffmanNode *A = *(HuffmanNode**)a;
    HuffmanNode *B = *(HuffmanNode**)b;
    return A->freq - B->freq;
}

HuffmanNode *buildTree(int *freq){
    HuffmanNode **nodes = (HuffmanNode**)malloc(R * sizeof(HuffmanNode));
    int count = 0;
    for(int i = 0; i < R; i++){
        if(freq[i] > 0){
            nodes[count++] = createNode((char)i, freq[i], NULL, NULL);
        }
    }

    qsort(nodes, count, sizeof(HuffmanNode*), cmp);
    
    while(count > 1){
        HuffmanNode *left = nodes[0];
        HuffmanNode *right = nodes[1];
        HuffmanNode *parent = createNode('\0', left->freq + right->freq, left, right);

        nodes[1] = parent;
        count--;
        for(int i = 0; i < count; i++){
            nodes[i] = nodes[i + 1];
        }
        qsort(nodes, count, sizeof(HuffmanNode*), cmp);
    }
    HuffmanNode *root = nodes[0];
    free(nodes);
    return root;
}

void read_freq(char *file_path, int *s){
    int count = 0;
    FILE *fi = fopen(file_path, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }
    int c;
    while((c = fgetc(fi)) != EOF){
        count++;
        s[c]++;
    }
}

int isLeaf(HuffmanNode *node){
    if(node == NULL)    return 1;
    return (node->left == NULL) && (node->right == NULL);
}


void codeTable(char *st[], HuffmanNode *node, const char*s){
    
    if(node->left != NULL || node->right != NULL){
        char leftStr[R];
        char rightStr[R];
        snprintf(leftStr, sizeof(leftStr), "%s0", s);
        snprintf(rightStr, sizeof(rightStr), "%s1", s);
        codeTable(st, node->left, leftStr);
        codeTable(st, node->right, rightStr);
    }
    else{
        st[(unsigned char)node->c] = strdup(s);
    } 
}

void encode(int *freq, char* filepathin, char *filepathout){
    HuffmanNode *root = buildTree(freq);
    char *st[R];
    codeTable(st, root, "");
    FILE *fi = fopen(filepathin, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }
    FILE *fo = fopen(filepathout, "w");
    if(!fo){
        perror("Cannot open the output file\n");
        exit(1);
    }
    int c;
    while((c = fgetc(fi)) > 0){
        fprintf(fo, "%s", st[c]);
    }
    
    fclose(fi);
    fclose(fo);
}

void decode(int *freq, char *filepathin, char *filepathout){
    HuffmanNode *root = buildTree(freq);
    HuffmanNode *node = root;

    FILE *fi = fopen(filepathin, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "w");
    if(!fo){
        perror("Cannot open the output file\n");
        exit(1);
    }
    char c;
    while((c = fgetc(fi)) != EOF){
        node = (c == '0') ? node->left : node->right;

        if(node->left == NULL && node->right == NULL){
            fputc(node->c, fo);
            node = root;
        }
    }
}


int main(int argc, char **argv){
    if(argc != 4){
        perror("Wrong number of arguments. Usage: <file_to_be_encoded> <encoded_file> <decoded_file>\n");
        exit(1);
    }
    int s[R] = {0};
    read_freq(argv[1], s);
    encode(s, argv[1], argv[2]);
    decode(s, argv[2], argv[3]);
    return 0;
}