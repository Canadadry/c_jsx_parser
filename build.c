#include "build.h"

#define TARGET "jsx_parser"

void build_lib(){
    BuildCtx ctx = build_init();
    build_set_src_dir(&ctx, "src");
    build_set_build_dir(&ctx, "build");
    build_set_cflags(&ctx, "-Wall -O2");
    // build_set_ldflags(&ctx, "-lm");

    build_make_dir(ctx.build_dir);

    build_add_static_lib(&ctx, "lib"TARGET".a");
    build_add_entry_point(&ctx, "main.c",TARGET);

    build_compile(&ctx, "*.c");
    build_link_all(&ctx);
}

void build_test(){
    BuildCtx ctx = build_init();
    build_set_src_dir(&ctx, "test");
    build_set_build_dir(&ctx, "build/tests");
    build_set_cflags(&ctx, "-Wall -O2");
    build_set_ldflags(&ctx, "-Lbuild -l"TARGET);

    build_make_dir(ctx.build_dir);

    build_add_entry_point(&ctx, "main.c", "run_tests");

    build_compile(&ctx, "*.c");
    build_link_all(&ctx);
}

int main(int argc, char **argv) {

    if (build_has_arg(argc, argv, 1, "clean")){
        BUILD_RUN_CMD("rm", "-rf", "build");
    }

    build_lib();
    if (build_has_arg(argc, argv, 1, "test")){
        build_test();
        BUILD_RUN_CMD("./build/tests/run_tests");
    }

    if (build_has_arg(argc, argv, 1, "run")){
        BUILD_RUN_CMD("./build/"TARGET,"example.jsx");
    }

    return 0;
}
