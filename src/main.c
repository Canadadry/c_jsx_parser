#include "buffer.h"
#include "jsx.h"
#include "parser.h"
#include "slice.h"
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

void fn_free(void* userdata,void* ptr){
    free(ptr);
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
    Compiler* c = new_compiler("React.createElement(", (Allocator){
        .realloc_fn=fn_realloc,
        .free_fn=fn_free,
    });
    CompileResult result = compile(c,slice_from(fcontent));
    if (result.type != OK) {
        printf("compile jsx failed at %d : %s\n", result.value.err.at,parser_error_to_string(result.value.err.code));
        free_compiler(c);
        return 1;
    }else{
        printf("%.*s\n",result.value.ok.len,result.value.ok.start);
        free_compiler(c);
        return 0;
    }
}
