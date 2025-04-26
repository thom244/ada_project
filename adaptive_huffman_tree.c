#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define R 256

typedef struct HuffmanNode {
    unsigned char c;
    int freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

HuffmanNode* createNode(unsigned char c, int freq, HuffmanNode *left, HuffmanNode *right) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->c = c;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

void freeTree(HuffmanNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int isLeaf(HuffmanNode* node) {
    return node && (node->left == NULL && node->right == NULL);
}

int cmp(const void* a, const void* b){
    HuffmanNode *A = *(HuffmanNode**)a;
    HuffmanNode *B = *(HuffmanNode**)b;

    return A->freq - B->freq;
}

HuffmanNode *rebuildTree(int *freq){
    HuffmanNode **nodes = (HuffmanNode**)malloc(R * sizeof(HuffmanNode*));
    int count = 0;
    HuffmanNode *isNYT = createNode('\0', 0, NULL, NULL);
    nodes[count++] = isNYT;

    for(int i = 0; i < R; i++){
        if(freq[i] > 0){
            nodes[count++] = createNode((char)i, freq[i], NULL, NULL);
        }
    }

    qsort(nodes, count, sizeof(HuffmanNode*), cmp);
    
    while(count > 1){
        HuffmanNode *left = nodes[0];
        HuffmanNode *right = nodes[1];
        HuffmanNode *parent = createNode('0', left->freq + right->freq, left, right);

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


int findCode(HuffmanNode* root, char c, char* path, char* result) {
    if (root == NULL) return 0;

    if (isLeaf(root) && root->c == c) {
        strcpy(result, path);
        return 1;
    }

    if (root->left) {
        char newPath[R];
        sprintf(newPath, "%s0", path);
        if(findCode(root->left, c, newPath, result)) return 1;
    }
    if (root->right) {
        char newPath[R];
        sprintf(newPath, "%s1", path);
        if(findCode(root->right, c, newPath, result)) return 1;
    }
    
    return 0;
}

void encode(char* filepathin, char *filepathout){
    HuffmanNode *root = NULL;

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

    uint32_t size = 0;

    fwrite(&size, sizeof(uint32_t), 1, fo);

    uint8_t buf;
    int freq[R] = {0};
    int c;
    int code[R];
    int code_index = 0;

    while((c = fgetc(fi)) != EOF){
        freeTree(root);
        root = rebuildTree(freq);
        freq[c]++;

        int isNYT = 0;

        char s[R] = {0};
        if(freq[c] == 1){
            isNYT = 1;
            findCode(root, '\0', "", s);
        }
        else{
            findCode(root, c, "", s);
        }
        for(int i = 0; s[i] != '\0'; i++){
            code[code_index++] = (s[i] == '1');
        }
        if(isNYT){
            for(int i = 0; i < 8; i++){
                code[code_index++] = (c >> (7 - i)) & 1;
            }
        }
            
        while(code_index > 8){
            buf = 0;
            for (int i = 0; i < 8; i++) {
                buf = (buf << 1) | code[i];
            }
            code_index -= 8;
            size += 8;
            fwrite(&buf, sizeof(uint8_t), 1, fo);
            for(int i = 0; i < code_index; i++){
                code[i] = code[i + 8];
            }
        }     
    }

    buf = 0;
    for(int i = 0; i < code_index; i++){
        buf = (buf << 1) | code[i];
    }
    if(code_index > 0){
        buf <<= 8-code_index;
        size += code_index;
        fwrite(&buf, sizeof(uint8_t), 1, fo);
    }

    rewind(fo);
    fwrite(&size, sizeof(uint32_t), 1, fo);
    freeTree(root);

    fclose(fi);
    fclose(fo);
}


void decode(char *filepathin, char *filepathout){
    FILE *fi = fopen(filepathin, "rb");
    if (!fi) {
        perror("Cannot open the input file\n");
        exit(1);
    }

    FILE *fo = fopen(filepathout, "w");
    if (!fo) {
        perror("Cannot open the output file\n");
        exit(1);
    }

    uint32_t read_size;
    fread(&read_size, sizeof(read_size), 1, fi);

    uint8_t buffer = 0;
    int bits_left = 0;

    int freq[R] = {0};
    HuffmanNode *root = rebuildTree(freq);
    HuffmanNode *node = root;

    while (read_size > 0) {
        if (bits_left == 0) {
            if (fread(&buffer, sizeof(buffer), 1, fi) != 1) break;
            bits_left = 8;
        }

        if (isLeaf(node) && node->c == '\0') {
            uint8_t c = 0;
            for (int i = 0; i < 8; i++) {
                if (bits_left == 0) {
                    if (fread(&buffer, sizeof(buffer), 1, fi) != 1) break;
                    bits_left = 8;
                }
                int bit = (buffer >> (bits_left - 1)) & 1;
                bits_left--;

                c = (c << 1) | bit;
                read_size--;
            }
            printf("%d\n", c);
            fputc(c, fo);
            freq[c]++;
            freeTree(root);
            root = rebuildTree(freq);
            node = root;
            continue;
        }

        int bit = (buffer >> (bits_left - 1)) & 1;
        bits_left--;
        read_size--;

        node = (bit == 0) ? node->left : node->right;

        if (isLeaf(node) && node->c != '\0') {
            fputc(node->c, fo);
            freq[node->c]++;
            freeTree(root);
            root = rebuildTree(freq);
            node = root;
        }
    }

    freeTree(root);
    fclose(fi);
    fclose(fo);
}



long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    ftell(file);
    long fileSize = ftell(file);

    fclose(file);

    return fileSize;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: <input_file> <-c|-d> <output_file>\n");
        return 1;
    }

    if (strcmp(argv[2], "-c") == 0) {
        printf("Encoding...\n");
        encode(argv[1], argv[3]);
        long o_file_size = get_file_size(argv[1]);
        long c_file_size = get_file_size(argv[3]);
        printf("Original file size: %ld\nCompressed file size: %ld\n", o_file_size, c_file_size);
        printf("Saved %ldB of space; Size reduced with %.2f%%\n", o_file_size - c_file_size, (100.0 - (float)(c_file_size * 100)/o_file_size));
        printf("Encoding complete.\n");
    }
    else if (strcmp(argv[2], "-d") == 0) {
        printf("Decoding...\n");
        decode(argv[1], argv[3]);
        printf("Decoding complete.\n");
    }
    else {
        printf("Unknown command: %s\n", argv[2]);
        return 1;
    }

    return 0;
}
