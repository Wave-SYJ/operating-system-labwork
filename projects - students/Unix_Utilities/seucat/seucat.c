#include <stdio.h>
#include <stdlib.h>

void printFile(char* path) {
	FILE* pFile = fopen(path, "r");
	if (pFile == NULL) {
		printf("cannot open file\n");
		exit(1);
	}

	char buffer[255];
	while (fgets(buffer, 255, pFile) != NULL)
		printf("%s", buffer);
	fclose(pFile);
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++)
		printFile(argv[i]);

	return 0;
}
