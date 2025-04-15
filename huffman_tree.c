#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

void inOrder(HuffmanNode *root){
    if(root == NULL)    return;
    inOrder(root->left);
    printf("%c ", root->c);
    inOrder(root->right);
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
    
    fclose(fi);
}

int isLeaf(HuffmanNode *node){
    return node && (node->left == NULL) && (node->right == NULL);
}


void codeTable(char *st[], HuffmanNode *node, const char*s, char *codelength){
    
    if(node->left != NULL || node->right != NULL){
        char leftStr[R];
        char rightStr[R];
        snprintf(leftStr, sizeof(leftStr), "%s0", s);
        snprintf(rightStr, sizeof(rightStr), "%s1", s);
        codeTable(st, node->left, leftStr, codelength);
        codeTable(st, node->right, rightStr, codelength);
    }
    else{
        codelength[(unsigned char)node->c] = strlen(s);
        st[(unsigned char)node->c] = strdup(s);
    } 
}

void encode(int *freq, char* filepathin, char *filepathout){
    HuffmanNode *root = buildTree(freq);
    char code_length[R] = {0};
    char *st[R] = {0};
    int size = 0;
    int num_strings = 0;
    
    codeTable(st, root, "", code_length);
    for(int i = 0; i < R; i++){
        size += (code_length[i] * freq[i]);
        num_strings += freq[i];
    }

    FILE *fi = fopen(filepathin, "r");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "wb");
    if(!fo){
        perror("Cannot open the output file\n");
        exit(1);
    }

    for (int i = 0; i < R; i++) {
        fwrite(&freq[i], sizeof(int), 1, fo);
    }

    fwrite(&size, sizeof(uint32_t), 1, fo);

    uint8_t buffer;
    int bit_count = 0;
    int c;

    while((c = fgetc(fi)) != EOF){
        char *bit_str = st[c];
        for (int j = 0; j < code_length[c]; j++) {
            buffer <<= 1;

            if (bit_str[j] == '1') {
                buffer |= 1;
            }
            bit_count++;

            if (bit_count == 8) {
                fwrite(&buffer, 1, 1, fo);
                bit_count = 0;
                buffer = 0;
            }
        }
    }

    if (bit_count > 0) {
        buffer <<= (8 - bit_count);
        fwrite(&buffer, 1, 1, fo);
    }
    
    for (int i = 0; i < R; i++) {
        if (st[i]) free(st[i]);
    }

    fclose(fi);
    fclose(fo);
}

void decode(char *filepathin, char *filepathout){
    FILE *fi = fopen(filepathin, "rb");
    if(!fi){
        perror("Cannot open the input file\n");
        exit(1);
    }

    int freq[R] = {0};

    for (int i = 0; i < R; i++) {
        fread(&freq[i], sizeof(int), 1, fi);
    }

    HuffmanNode *root = buildTree(freq);
    HuffmanNode *node = root;

    FILE *fo = fopen(filepathout, "w");
    if(!fo){
        perror("Cannot open the output file\n");
        exit(1);
    }

    uint32_t read_size;
    fread(&read_size, sizeof(read_size), 1, fi);
    uint8_t buffer = 0;

    while(fread(&buffer, sizeof(buffer), 1, fi) > 0 && (read_size - 8) > 0){
        for(int i = 7; i >= 0 && read_size > 0; i--){
            int bit = (buffer >> i) & 1;
            read_size--;
        
            node = (bit == 0) ? node->left : node->right;
        
            if (isLeaf(node)) {
                //printf("%c", node->c);
                fputc(node->c, fo);
                node = root;
            }
            bit = 0;
        }
    }

    fclose(fi);
    fclose(fo);
}


int main(int argc, char **argv){
    if(argc != 4){
        perror("Wrong number of arguments. Usage: <file> <mode> <output_file>\n");
        exit(1);
    }

    if(!strcmp(argv[2], "-c")){
        int s[R] = {0};
        read_freq(argv[1], s);
        printf("Compressing...\n");
        encode(s, argv[1], argv[3]);
    }
    else if(!strcmp(argv[2], "-d")){
        printf("Decompressing...\n");
        decode(argv[1], argv[3]);
    }
    else{
        perror("Unknown command\n");
        exit(1);
    }

    return 0;
}