#ifndef __TYPR_H__
#define __TYPR_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_WORDS 500

#define BLACK  "\033[0;30m"
#define RED    "\033[0;31m"
#define GREEN  "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN   "\033[0;36m"
#define WHITE  "\033[0;37m"

#define BLANK 0
#define GOOD  1
#define BAD   2

typedef struct Typr {
      char *text;
      int *flag_text;
      int length;
      int words;
      int index;
      double mistakes;
      double correct;
      double time;
      double wpm;
      double accuracy;
      bool running;
} typr_t;

typedef struct Records_Array {
      double wpm;
      double accuracy;
      double seconds;
      int words;
} records_array_t;

typedef struct Records {
      records_array_t *array;
      size_t size;
      bool update;
} records_t;

typedef struct Config {
      char cursor[10];
      int words;
} config_t;

void init_config();
int validate_args(int argc, char **argv);
void init_curses();
void init_pairs();
int typr_logic(int quan);
int rerun();

#endif // __TYPR_H__