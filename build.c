#include "build.h"

#define TARGET "jsx_parser"

int main(int argc, char **argv) {
    BuildCtx ctx = build_init();
    build_set_src_dir(&ctx, "src");
    build_set_build_dir(&ctx, "build");
    build_set_cflags(&ctx, "-Wall -O2");
    build_set_ldflags(&ctx, "-lm");

    if (build_has_arg(argc, argv, 1, "clean")){
        BUILD_RUN_CMD("rm", "-rf", ctx.build_dir);
        return 0;
    }

    build_make_dir(ctx.build_dir);

    if (build_has_arg(argc, argv, 3, "release", "r", "-r"))
        build_set_release(&ctx, "-O3");

    build_add_entry_point(&ctx, "main.c", TARGET);
    build_add_static_lib(&ctx, "lib"TARGET".a");

    build_compile(&ctx, "*.c");
    build_link_all(&ctx);

    if (build_has_arg(argc, argv, 1, "run"))
        BUILD_RUN_CMD("./build/"TARGET);

    return 0;
}
