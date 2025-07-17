#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  Cmd cmd = {0};
  cmd_append(&cmd, "clang++");
  cmd_append(&cmd, "-Wall", "-Wextra");
  cmd_append(&cmd, "-I/opt/homebrew/Cellar/opencv/4.11.0_1/include/opencv4/");
  cmd_append(&cmd, "-o", "main");
  cmd_append(&cmd, "main.cpp");
  cmd_append(&cmd, "-L/opt/homebrew/Cellar/opencv/4.11.0_1/lib/");
  cmd_append(&cmd, "-lopencv_core", "-lopencv_videoio", "-lopencv_highgui");
  if (!cmd_run_sync_and_reset(&cmd)) return 1;
  return 0;
}

