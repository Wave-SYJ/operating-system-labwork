#include <stdio.h>
#include <stdlib.h>

void printDecompressed(char* filePath) {
    FILE* pFile = fopen(filePath, "r");
    if (pFile == NULL) {
        printf("cannot open file");
        exit(1);
    }

    unsigned int count = 0;
    char ch = 0;
    while (1) {
        fread(&count, 4, 1, pFile);
        fread(&ch, sizeof(char), 1, pFile);

        if (feof(pFile))
            break;

        for (int i = 0; i < count; i++)
            printf("%c", ch);
    }

    fclose(pFile);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("seuunzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++)
        printDecompressed(argv[i]);

    return 0;
}
