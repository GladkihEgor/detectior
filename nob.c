#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  Cmd cmd = {0};
  const char *include = "-I/opt/homebrew/Cellar/opencv/4.11.0_1/include/opencv4/";
  const char *libs = "-L/opt/homebrew/Cellar/opencv/4.11.0_1/lib/";
  cmd_append(&cmd, "clang++", "-Wall", "-Wextra", include, libs, "-lopencv_core", "-lopencv_videoio", "-lopencv_highgui", "-o", "main", "main.cpp");
  if (!cmd_run_sync(cmd)) return 1;
  return 0;
}

