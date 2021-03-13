#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void searchStrFromFile(char* str, FILE* pFile) {
	char* buffer = NULL;
    size_t size = 0;

	while (getline(&buffer, &size, pFile) != -1) {
		if (strstr(buffer, str))
			printf("%s", buffer);

		free(buffer);
		buffer = NULL;
		size = 0;
	}
	free(buffer);
}

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("searchterm [file ...]\n");
		return 1;
	}
	if (argc == 2) {
		searchStrFromFile(argv[1], stdin);
		return 0;
	}

	for (int i = 2; i < argc; i++) {	
		FILE* pFile = fopen(argv[i], "r");
		if (pFile == NULL) {
			printf("cannot open file\n");
			exit(1);
		}
		searchStrFromFile(argv[1], pFile);
		fclose(pFile);
	}
	return 0;
}
