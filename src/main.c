#include "jsx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char* read_full(const char* filename)
{
	FILE *f;
	char *s;
	int n, t;

	f = fopen(filename, "rb");
	if (!f) {
		printf("cannot open file '%s': %s", filename, strerror(errno));
		return NULL;
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		fclose(f);
		printf("cannot seek in file '%s': %s", filename, strerror(errno));
		return NULL;
	}

	n = ftell(f);
	if (n < 0) {
		fclose(f);
		printf("cannot tell in file '%s': %s", filename, strerror(errno));
		return NULL;
	}

	if (fseek(f, 0, SEEK_SET) < 0) {
		fclose(f);
		printf("cannot seek in file '%s': %s", filename, strerror(errno));
		return NULL;
	}

	s = malloc(n + 1);
	if (!s) {
		fclose(f);
		printf("out of memory");
		return NULL;
	}

	t = fread(s, 1, n, f);
	if (t != n) {
		free(s);
		fclose(f);
		printf("cannot read data from file '%s': %s", filename, strerror(errno));
		return NULL;
	}
	s[n] = 0;
	fclose(f);
	return s;
}

void* fn_realloc(void* userdata,void* ptr, size_t size){
    return realloc(ptr, size);
}

int main(int argc,char ** argv){
    if(argc <=1){
        printf("usage: %s filename\n",argv[0]);
        return 1;
    }

    char* fcontent = read_full(argv[1]);
    if(fcontent==NULL){
        return 1;
    }
    Compiler c = {0};
    c.createElem = slice_from("React.createElement(");
    c.in_buf = fcontent;
    c.in_buf_count = strlen(fcontent);
    c.in_buf_capacity = strlen(fcontent);
    c.realloc_fn =fn_realloc;
    CompileResult result = compile(&c);
    if (result.type != OK) {
        printf("compile jsx failed: %d\n", result.value.err);
        return 1;
    }else{
        printf("%.*s\n",result.value.ok.len,result.value.ok.start);
    }
    return 0;
}
