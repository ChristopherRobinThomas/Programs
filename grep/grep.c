#include <alloca.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fnmatch.h>

static char* pattern = NULL;
static int32_t options = 0;

static void dirOrFile(char* name); 
static char* readParameter(int32_t argc, char** argv); 
static void traverseFile(char* file); 
static void traverseDir(char* dir); 

enum OPTIONS{
	RECURSIVE = 1 << 1,
	REPLACE = 1 << 2,
	EXACT = 1 << 3,
};

static void err(char* msg){
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void die(char* msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

static char* readParameter(int32_t argc, char** argv){
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
				case 'e': options |= EXACT; break;
				case 'd': err("depthmax: TBD"); break;
				default: err("Argument not known");
			}
		}
		else if(!pattern_set){
			pattern = argv[i];
			pattern_set = true;
		}
		else if(NULL == dir){
			dir = argv[i];
		}else{
			err("usage: grep <Pattern> <Directory>");
		}
	}
	if(!(EXACT & options)){
		int32_t l = strlen(pattern);
		char* tmp = malloc(l + 3);
		if(NULL == tmp) die("malloc");
		strcpy(tmp + 1, pattern);
		tmp[0] = '*';
		tmp[l + 1] = '*';
		tmp[l + 2] = '\0';
		pattern = tmp;
	}
	return dir;
}
static void traverseDir(char* dir){
	DIR* d = opendir(dir);
	if(NULL == d){
		die("opendir");
	}
	int32_t cap = 256;
	char* name = alloca(cap);
	while (true) {
		errno = 0;
		struct dirent* entry = readdir(d);
		if(entry == NULL) break;
		if(strcmp("..", entry ->d_name) == 0 || strcmp(".", entry ->d_name) == 0){
			continue;
		}
		int32_t size = strlen(dir) + strlen(entry->d_name) + 1 + 1;
		if(size > cap){
			cap *= 2;
			char* name = alloca(cap);
		}
		if(dir[strlen(dir) - 1] == '/'){
			dir[strlen(dir) - 1] = '\0';
		}
		snprintf(name, size, "%s/%s", dir, entry->d_name);
		dirOrFile(name);
		
	}
	if(errno != 0) die("readdir");
	if(closedir(d) != 0){
		die("closedir");
	}
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
		if(line[line_len - 1] != '\n'){
			while(true){
				if(cap <= line_len - 1){
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
		if(line[line_len - 1] == '\n'){
			line[line_len - 1] = '\0';
			line_len--;
		}
		int32_t ret = fnmatch(pattern, line, 0)	;
		if(0 == ret){
			if(printf("%s:%d: %s\n", file, line_number, line) < 0){
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

static void dirOrFile(char* name){
	struct stat sb;
	if(-1 == stat(name, &sb)){
		die("stat");
	}
	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR: traverseDir(name); break;
		case S_IFREG: traverseFile(name); break;
		default: err("Not useful filetype");
	}
}

int main(int argc, char** argv){
	char* start = readParameter(argc, argv);
	dirOrFile(start)	;

	if(!(EXACT & options)){
		free(pattern);
	}
	exit(EXIT_SUCCESS);
}
