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
            node->parent->right = parent;
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
    FILE *fi = fopen(filepathin, "rb");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "wb");
    if(!fo){
        perror("Cannot open the input file\n");
        exit(1);
    }

    HuffmanTree tree = build_tree();

    int code[R + 8];
    uint8_t buf = 0;;
    int code_index = 0;

    int c;
    uint32_t size = 0;
    fwrite(&size, sizeof(uint32_t), 1, fo);

    while((c = fgetc(fi)) != EOF){
        HuffmanNode *to_encode = tree.exist_char[(int)c];

        if(to_encode == NULL){
            add_node(&tree, c);
            HuffmanNode *node = tree.root;
            while(node && node->left && node->left != tree.isNYT){
                code[code_index++] = 0;
                node = node->left;
            }
            
            char ch = c;
            for (int j = sizeof (ch) * 8 - 1; j >= 0; j--){
                code[code_index++] = ((ch >> j) & 0x1) ? 1 : 0;
            }
        }
        else{
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
                buf <<= 1;
                buf |= code[i] & 0x1;
            }

            fwrite(&buf, sizeof(uint8_t), 1, fo);
            buf = 0;
            size++;

            code_index -= 8;
            for(int i = 0; i < code_index; i++){
                code[i] = code[i + 8];
            }
        }
        
    }
    buf = 0;
    for(int i = 0; i < code_index; i++) {
        buf <<= 1;
        buf |= code[i] & 0x1;
    }
    buf <<= (8 - code_index);
    fwrite(&buf, sizeof(uint8_t), 1, fo);
    size++;


    fseek(fo, 0, SEEK_SET);
    fwrite(&size, sizeof(uint32_t), 1, fo);

    fclose(fi);
    fclose(fo);
    free_tree(tree.root);
}

void decode(char *filepathin, char *filepathout){
    FILE *fi = fopen(filepathin, "rb");
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
    int c = 0;
    int isByte = 0;
    int ch = 0;
    uint8_t buf;
    HuffmanNode *node = tree.root;

    uint32_t size;
    fread(&size, sizeof(uint32_t), 1, fi);
    printf("%d\n", size);

    for(int i = 0; i < size; i++){
        fread(&buf, sizeof(uint8_t), 1, fi);
        for(int j = 0; j < 8; j++){
            c = (buf >> (7 - j)) & 0x1;
            if(node->isNYT == 0 && c == 0 && isByte == 0){
                node = node->left;
            }
            else if(node->isNYT != 0){
                isByte++;
                ch += pow(2, 8 - isByte) * c;
                if(isByte - 8 == 0){
                    isByte -= 8;
                    fputc(ch, fo);
                    add_node(&tree,(char) ch);
                    ch = 0;
                    node = tree.root;
                }
                //update(c);
            }
            else{
                fputc(node->right->c, fo);
                node = tree.root;
            }
        }
    }
    printf("\n");
    fclose(fi);
    fclose(fo);
    free_tree(tree.root);
}

int is_identical(char *f1, char *f2){
    FILE *fi1 = fopen(f1, "r");
    if(!fi1){
        perror("Cannot open input file 1\n");
        exit(1);
    }
    FILE *fi2 = fopen(f2, "r");
    if(!fi2){
        perror("Cannot open input file 2\n");
        exit(1);
    }
    int c1, c2;
    while((c1 = fgetc(fi1)) != EOF){
        c2 = fgetc(fi2);
        if(c1 != c2){
            printf("%c - %c\n", c1, c2);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv){
    if(argc != 4){
        perror("Usage: <input file> <output encoded file> <output decoded file>");
        exit(1);
    }
    char *s = "Cal de mare";
    encode(s, argv[1], argv[2]);
    decode(argv[2], argv[3]);
    is_identical(argv[1], argv[3])?printf("True\n"):printf("False\n");
    return 0;
}