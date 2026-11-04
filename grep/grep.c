#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fnmatch.h>

static char* pattern = NULL;
static int32_t options = 0;

enum OPTIONS{
	RECURSIVE = 1 << 1,
	REPLACE = 1 << 2,
};
static void err(char* msg){
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void die(char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

static char* readParameter(int argc, char** argv){
	char* dir = NULL;
	bool pattern_set = false;
	if(argc < 3){
		err("usage: grep <Pattern> <Directory>");
	}
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			if(strlen(argv[i]) < 2){
				err("- ist not allowed to stand alone");
			}
			switch(argv[i][1]){
				case 'r': options |= RECURSIVE; break;
				case 's': options |= REPLACE; break;
				default: err("Argument not known");
			}
		}
		else if(!pattern_set){
			pattern = argv[i];
		}
		else if(NULL == dir){
			dir = argv[i];
		}else{
			err("usage: grep <Pattern> <Directory>");
		}
	}
	return dir;
}
static void traverseDir(char* dir){

}

static void traverseFile(char* file){
	FILE* f = fopen(file, "rw");
	if(NULL == f){
		die("fopen");
	}
	int32_t len = 0;
	int32_t line_number = 0;
	int32_t cap = 1024;
	char* line = malloc(cap);
	if(NULL == line){
		die("malloc");
	}
	while(fgets(line, cap, f) != NULL){
		line_number++;
		int32_t line_len = strlen(line);
		// Make sure to read whole line
		if(line[line_len] != '\n'){
			while(true){
				if(cap == line_len){
					cap *= 2;
					line = realloc(line, cap);
					if(NULL == line){
						die("malloc");
					}
				}
				char c = getc(f);
				if(EOF == c || '\n' == c){
					break;
				}
				line[line_len] = c;
				line_len++;
			}
		}
		// Remove last line
		if(line[line_len] == '\n'){
			line[line_len] = '\0';
			line_len--;
		}
		int32_t ret = fnmatch(pattern, line, 0)	;
		if(0 == ret){
			if(printf("%s:%d : %s", file, line_number, line) < 0){
				die("printf");
			}
		}
	}
	if(ferror(f)){
		die("fgets");
	}
	fclose(f);
	free(line);
}

int main(int argc, char** argv){
	char* start = readParameter(argc, argv);
	struct stat sb;
	if(-1 == stat(start, &sb)){
		die("stat");
	}

	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR: traverseDir(start); break;
		case S_IFREG: traverseFile(start); break;
		default: err("Not useful filetype");
	}

	exit(EXIT_SUCCESS);
}
