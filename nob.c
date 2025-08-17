#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

bool compile_macos(Cmd cmd, bool is_static)
{
  NOB_ASSERT(!is_static && "MacOS doesn't support static linking");
  char *opencv_path = "/opt/homebrew/Cellar/opencv/4.12.0_4";
  cmd_append(&cmd, "clang++");
  cmd_append(&cmd, "-Wall", "-Wextra", "-std=c++23");
  cmd_append(&cmd, temp_sprintf("-I%s/include/opencv4/", opencv_path));
  cmd_append(&cmd, "-o", "main");
  cmd_append(&cmd, "main.cpp");
  cmd_append(&cmd, temp_sprintf("-L%s/lib/", opencv_path));
  cmd_append(&cmd, "-lopencv_core", "-lopencv_videoio", "-lopencv_highgui", "-lopencv_imgproc", "-lopencv_dnn");
  return cmd_run_sync_and_reset(&cmd);
}

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  shift(argv, argc); // program name
  bool static_build = false;
  while (argc > 0) {
    char *arg = shift(argv, argc);
    if (strcmp("-static", arg) == 0) static_build = true;
    else nob_log(WARNING, "Unknown argument `%s`, ignoring it\n", arg);
  }
  Cmd cmd = {0};
  if (!compile_macos(cmd, static_build)) return 1;
  return 0;
}

