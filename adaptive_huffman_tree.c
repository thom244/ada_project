#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define R 256

typedef struct HuffmanNode{
    char c;
    int freq;
    int isNYT; //Not Yet Transffered
    struct HuffmanNode *left;
    struct HuffmanNode *right;
    struct HuffmanNode *parent;
}HuffmanNode;

typedef struct{
    HuffmanNode *root;
    HuffmanNode *isNYT;
    HuffmanNode *exist_char[R];
}HuffmanTree;

HuffmanNode *createNode(char c, int freq, int isNYT){
    HuffmanNode *node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->c = c;
    node->freq = freq;
    node->isNYT = isNYT;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

HuffmanTree build_tree(){
    HuffmanTree tree;
    tree.root = createNode(0, 0, 1);
    tree.isNYT = tree.root;
    for(int i = 0; i < R; i++){
        tree.exist_char[i] = NULL;
    }
    return tree;
}

void free_tree(HuffmanNode* node) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void add_node(HuffmanTree *tree, char c){
    HuffmanNode *node = tree->isNYT;
    HuffmanNode *to_add = createNode(c, 1, 0);
    HuffmanNode *parent = createNode(0, 1, 0);

    parent->right = to_add;
    parent->left = node;
    if (node->parent) {
        if (node->parent->left == node)
            node->parent->left = parent;
        else
            node->parent->right = to_add;
    } else {
        tree->root = parent;
    }
    to_add->parent = parent;
    node->parent = parent;
    tree->exist_char[(int)c] = to_add;
}

void print_tree(HuffmanTree tree){
    HuffmanNode *node = tree.root;
    while(node && node->right){
        printf("%c ", node->right->c);
        node = node->left;
    }
    printf("\n");
}

void encode(char *s, char *filepathin, char *filepathout){
    FILE *fi = fopen(filepathin, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "w");
    if(!fo){
        perror("Cannot open the input file\n");
        exit(1);
    }

    HuffmanTree tree = build_tree();

    int code[R + 8];
    uint8_t buf[8];
    int code_index = 0;

    for(int i = 0; i < strlen(s); i++){
        HuffmanNode *to_encode = tree.exist_char[(int)s[i]];

        if(to_encode == NULL){
            add_node(&tree, s[i]);
            HuffmanNode *node = tree.root;
            while(node && node->left && node->left != tree.isNYT){
                code[code_index++] = 0;
                node = node->left;
            }
            
            char c = s[i];
            for (int j = sizeof (c) * 8 - 1; j >= 0; j--){
                code[code_index++] = ((c >> j) & 0x1) ? 1 : 0;
            }
        }
        else{
            //printf("in e\n");
            HuffmanNode *node = tree.root;
            while(node->right != to_encode){
                code[code_index++] = 0;
                node = node->left;
            }
            code[code_index++] = 1;
            //update(tree, to_encode);
        }

        while(code_index >= 8){
            for(int i = 0; i < 8; i++){
                buf[i] = code[i];
                fprintf(fo, "%d", buf[i]);
                //write(buf);
            }
            code_index -= 8;
            for(int i = 0; i < code_index; i++){
                code[i] = code[i + 8];
            }
        }
        
    }
    for(int i = 0; i < code_index - 1; i++){
        if(i < code_index){
            fprintf(fo, "%d", code[i]);
        }
        else{
            //fprintf(fo, "0");
        }
    }
    fprintf(fo, "\n");
    fclose(fi);
    fclose(fo);
    //free_tree(&tree);
}

void decode(char *filepathin, char *filepathout){
    FILE *fi = fopen(filepathin, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "w");
    if(!fo){
        perror("Cannot open the input file\n");
        exit(1);
    }

    HuffmanTree tree = build_tree();
    int c;
    int isByte = 0;
    int ch = 0;
    HuffmanNode *node = tree.root;
    while((c = fgetc(fi)) != EOF){
        if(node->isNYT == 0 && c == '0' && isByte == 0){
            node = node->left;
        }
        else if(node->isNYT != 0){
            isByte++;
            ch += pow(2, 8 - isByte) * (c - '0');
            if(isByte - 8 == 0){
                isByte -= 8;
                printf("%c", ch);
                add_node(&tree,(char) ch);
                ch = 0;
                node = tree.root;
            }
            //update(c);
        }
        else{
            printf("%c", node->right->c);
            node = tree.root;
        }
    }
    printf("\n");
    fclose(fi);
    fclose(fo);
    //free_tree(&tree);
}

int main(int argc, char **argv){
    if(argc != 4){
        perror("Usage: <input file> <output encoded file> <output decoded file>");
        exit(1);
    }
    char *s = "Cal de mare";
    encode(s, argv[1], argv[2]);
    decode(argv[2], argv[3]);
    return 0;
}