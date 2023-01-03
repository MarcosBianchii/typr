#include "typr.h"

int main(int argc, char *argv[])
{
      init_config();

      if (validate_args(argc, argv) == 1)
            return 0;

      int n;
      if (argc < 2) n = 25;
      else n = atoi(argv[1]);

      if (n > MAX_WORDS) n = MAX_WORDS;
      if (n < 1) return 0;

      srand(time(NULL));
      init_curses();
rerun:

      if (typr_logic(n) == 1)
            goto rerun;

      endwin();
      return 0;
}