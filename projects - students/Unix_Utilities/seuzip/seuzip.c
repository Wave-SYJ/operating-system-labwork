#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void printCompressed(char current) {
	static char previous = 0;
	static unsigned int count = 0;

	if ((previous == current || count == 0) && count != UINT_MAX) {
		count++;
		previous = current;
		return;
	}

	fwrite(&count, sizeof(unsigned int), 1, stdout);
	fwrite(&previous, sizeof(char), 1, stdout);
	count = 1;
	previous = current;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("seuzip: file1 [file2 ...]\n");
		return 1;
	}

	char current = 0;
	for (int i = 1; i < argc; i++) {
		FILE* pFile = fopen(argv[i], "r");
		if (pFile == NULL) {
			printf("cannot open file\n");
			exit(1);
		}
		while ((current = fgetc(pFile)) != EOF)
			printCompressed(current);
		fclose(pFile);
	}
	printCompressed(EOF);

	return 0;
}
