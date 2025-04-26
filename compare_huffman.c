#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WORDS_SIZE (sizeof(words)/sizeof(words[0]))

const char* words[] = {
    "lorem", "ipsum", "dolor", "sit", "amet", 
    "consectetur", "adipiscing", "elit", "sed", "do",
    "eiusmod", "tempor", "incididunt", "ut", "labore",
    "et", "dolore", "magna", "aliqua", "zoly", "wuxi"
};

void generate_words(char *filepath, int word_count){
    FILE *fout = fopen(filepath, "w");
    if (!fout) {
        perror("Cannot open file for writing");
        exit(1);
    }

    srand(time(NULL));

    for (long i = 0; i < word_count; i++) {
        fprintf(fout, "%s", words[rand() % WORDS_SIZE]);
        fputc(' ', fout);
    }

    fclose(fout);
}

long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    ftell(file);
    long fileSize = ftell(file);

    fclose(file);

    return fileSize;
}

double compile_and_run(char* source_file, char* exec_file, char* fi, char* fo) {
    char compile_command[256];
    snprintf(compile_command, sizeof(compile_command), "gcc -Wall -o  %s %s", exec_file, source_file);
    
    if (system(compile_command) != 0) {
        printf("Compilation failed for %s\n", source_file);
        exit(1);
    }

    char run_command[256];
    snprintf(run_command, sizeof(run_command), "./%s %s %s %s  > /dev/null 2>&1", exec_file, fi, "-c", fo);
    
    clock_t start_time = clock();
    if (system(run_command) != 0) {
        printf("Execution failed for %s\n", exec_file);
        exit(1);
    }
    clock_t end_time = clock();
    
    double time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    //printf("Execution time for %s: %.6f seconds\n", exec_file, time);
    return time;
}

int main() {
    for(int i = 100; i < 10000000; i *= 10){
        char* fi = "text.txt";
        char* of1 = "huffman_tree.txt";
        char* of2 = "adaptive_huffman_tree.txt";

        generate_words(fi, i);
        
        double time_h = compile_and_run("huffman_tree.c", "huffman_tree", fi, of1);
        printf("\n");
        double time_ah = compile_and_run("adaptive_huffman_tree.c", "adaptive_huffman_tree", fi, of2);
        printf("\n");

        long o_file_size = get_file_size(fi);
        long static_file_size = get_file_size(of1);
        long adaptive_file_size = get_file_size(of2);

        float static_compression = (100.0 - (float)(static_file_size * 100)/o_file_size);
        float adaptive_compression = (100.0 - (float)(adaptive_file_size * 100)/o_file_size);

        printf("For %ldB size, the statistics afe:\n", o_file_size);

        printf("For static Huffman encoding:\n");
        printf("Original file size: %ld\nCompressed file size: %ld\n", o_file_size, static_file_size);
        printf("Saved %ldB of space; Size reduced with %.2f%%\n\n", o_file_size - static_file_size, static_compression);

        printf("For adaptive Huffman encoding:\n");
        printf("Original file size: %ld\nCompressed file size: %ld\n", o_file_size, adaptive_file_size);
        printf("Saved %ldB of space; Size reduced with %.2f%%\n", o_file_size - adaptive_file_size, adaptive_compression);

        if(time_h < time_ah)        printf("Static Huffman is faster with %fs\n", time_ah - time_h);
        else if(time_h > time_ah)   printf("Adaptive Huffman is faster with %fs\n", time_h - time_ah);
        else                        printf("Same time for both algorithms\n");

        if(static_compression > adaptive_compression)   
            printf("Static Huffman compress better than Adaptive Huffman with %f%%", static_compression - adaptive_compression);
        else if(static_compression < adaptive_compression)
            printf("Adaptive Huffman compress better than Static Huffman with %f%%\n", adaptive_compression - static_compression);
        else 
            printf("Both algorithms have the same compression rate\n");

    }
    return 0;
}
