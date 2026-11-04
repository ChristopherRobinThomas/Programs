#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <wait.h>

static void die(char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

static void err(char* msg){
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
static void execute(char* cmd){
	pid_t pid = fork();
	if(-1 == pid){
		die("fork");
	}else if(pid == 0){
		system(cmd);
		exit(EXIT_SUCCESS);
	}else{
		int tmp;
		wait(&tmp);
		return;
	}
}
static void document(FILE* file, char* cmd){
	fputs(cmd, file);
	fputc('\n', file);
	fflush(file);
}
int main(int argc, char* argv[]){
	if(argc < 2) err("usage: replay <output file>");
	int32_t cap = 1024;
	char* line = malloc(cap);
	if(NULL == line){
		die("malloc");
	}

	FILE* output = fopen(argv[1], "w");
	if(NULL == output){
		die("fopen");
	}

	while(fgets(line, cap, stdin) != NULL){
		int32_t line_len = strlen(line);
		// Make sure to read whole line
		if(line[line_len - 1] != '\n'){
			while(true){
				if(cap <= line_len - 1){
					cap *= 2;
					line = realloc(line, cap);
					if(NULL == line){
						die("malloc");
					}
				}
				char c = getc(stdin);
				if(EOF == c || '\n' == c){
					break;
				}
				line[line_len] = c;
				line_len++;
			}
		}
		// Remove last line
		if(line[line_len - 1] == '\n'){
			line[line_len - 1] = '\0';
			line_len--;
		}
		execute(line);
		document(output, line);
	}
	free(line);
	fclose(output);
	return EXIT_SUCCESS;
}
