#include "typr.h"

config_t config;

void init_config()
{
      strcpy(config.cursor, "underline");

      FILE *fp = fopen("files/config.txt", "r");
      if (!fp) return;

      char line[20];
      memset(line, 0, sizeof(line));
      fscanf(fp, "cursor_type=%[^\n]\n", line);

      if (strcmp(line, "block") == 0 || strcmp(line, "underline") == 0)
            strncpy(config.cursor, line, 10);

      if (config.words > 500) config.words = 500;

      fscanf(fp, "words_in_file=%d\n", &config.words);
      
      fclose(fp);
}

void print_help()
{
      printf(\
GREEN"\n> "WHITE"./typr "GREEN"[words / command]\n\n\n"\
CYAN"List of controls:\n\n"\
WHITE"RIGHT"BLACK" -> "YELLOW"Get new text\n"\
WHITE" END "BLACK" -> "YELLOW"Exit the program\n\n\n"\
CYAN"List of commands:\n\n"\
BLACK"--"WHITE"help             "BLACK" -> "YELLOW"Show this help menu\n"\
BLACK"--"WHITE"toggle-cursor    "BLACK" -> "YELLOW"Toggles the cursor type\n"\
BLACK"--"WHITE"update-word-count"BLACK" -> "YELLOW"Updates the word count\n\n");
}

int count_words_file()
{
      FILE *fp = fopen("words.txt", "r");
      if (!fp) return 0;
 
      int words = 0;

      for (char ch = getc(fp); ch != EOF; ch = getc(fp))
            if (ch == '\n')
                  words++;
 
      fclose(fp);
      return words;
}

bool first_boot()
{
      DIR *dir = opendir("files");
      if (!dir)
      {
            mkdir("files", 0777);

            FILE *fp = fopen("files/records.txt", "w");
            fclose(fp);

            fp = fopen("files/config.txt", "w");
            fprintf(fp, "cursor_type=underline\n");
            fprintf(fp, "words_in_file=%i\n", count_words_file());
            fclose(fp);

            return true;
      }

      closedir(dir);
      return false;
}

int validate_args(int argc, char **argv)
{
      if (first_boot())
      {
            print_help();
            return 1;
      }

      if (argc >= 2)
      {
            if (strncmp(argv[1], "--", 2) == 0) {
                  if (strcmp(argv[1], "--help") == 0)
                  {
                        print_help();
                        return 1;
                  }

                  else if (strcmp(argv[1], "--toggle-cursor") == 0)
                  {
                        strcmp(config.cursor, "block") == 0 ?
                        strcpy(config.cursor, "underline") :
                        strcpy(config.cursor, "block");

                        FILE *fp = fopen("files/config.txt", "w");
                        if (!fp) return 1;

                        fprintf(fp, "cursor_type=%s\n", config.cursor);
                        fprintf(fp, "words_in_file=%i\n", config.words);
                        fclose(fp);

                        printf("\nCursor type changed to "CYAN"%s\n\n"WHITE, config.cursor);
                        return 1;
                  }

                  else if (strcmp(argv[1], "--update-word-count") == 0)
                  {
                        int words = count_words_file();
                        
                        FILE *fp = fopen("files/config.txt", "w");
                        if (!fp)
                        {
                              printf("\n"RED"Error: "WHITE"Couldn't open the config file\n\n");
                              return 1;
                        }

                        fprintf(fp, "cursor_type=%s\n", config.cursor);
                        fprintf(fp, "words_in_file=%i\n", words);
                        fclose(fp);

                        printf("\nWord count updated to "CYAN"%i\n\n"WHITE, words);

                        return 1;
                  }

                  else {
                        printf("\n"RED"Error: "WHITE"Invalid command\n\n");
                        printf("Type "CYAN"%s --help"WHITE" for a list of commands\n\n", argv[0]);
                        return 1;
                  }

            }
      }

      return 0;
}

void init_curses()
{
      initscr();
      noecho();
      raw();
      start_color();
      use_default_colors();
      keypad(stdscr, TRUE);
      curs_set(0);
      init_pairs();
}

void init_pairs()
{
      init_pair(1, COLOR_BLUE,    -1);
      init_pair(2, COLOR_GREEN,   -1);
      init_pair(3, COLOR_RED,     -1);
      init_pair(4, COLOR_YELLOW,  -1);
      init_pair(5, COLOR_MAGENTA, -1);
      init_pair(6, COLOR_CYAN,    -1);
      init_pair(7, COLOR_WHITE,   -1);
      init_pair(8, COLOR_BLACK,   -1);
      init_pair(9, COLOR_BLACK, COLOR_GREEN);
      init_pair(10,COLOR_BLACK, COLOR_RED);
}

void shuffle(char **array, int read)
{
      for (size_t i = 0; i < read - 1; i++)
      {
            size_t j = i + rand() / (RAND_MAX / (read - i) + 1);
            char *t = array[j];
            array[j] = array[i];
            array[i] = t;
      }
}

char **get_words(int quan)
{
      if (quan < 1)
            return NULL;

      FILE *fp = fopen("words.txt", "r");
      if (!fp) return NULL;

      char **words = calloc(sizeof(char *), config.words);
      if (!words)
      {
            fclose(fp);
            return NULL;
      }

      size_t size = 0;
      size_t read = 0;
      char *line = calloc(256, sizeof(char));
      while (fscanf(fp, "%[^\n]\n", line) == 1 && read < config.words)
      {
            words[size] = calloc(sizeof(char), strlen(line) + 1);
            read++;
            if (!words[size])
            {
                  memset(line, 0, strlen(line));
                  continue;
            }

            strcpy(words[size++], line);
            memset(line, 0, strlen(line));
      }

      free(line);
      shuffle(words, read);
      
      fclose(fp);
      return words;
}

bool fill_typr(typr_t *typr, char** words, int quan)
{
      char *str = calloc(1, sizeof(char));
      if (!str) return false;

      for (int i = 0; i < quan; i++)
      {
            str = realloc(str, strlen(str) + strlen(words[i]) + 2);
            if (!str) return false;

            strcat(str, words[i]);

            if (i != quan - 1)
                  strcat(str, " ");
            else str[strlen(str)] = '\0';
      }

      typr->text = str;
      typr->length = strlen(str);
      typr->running = true;
      typr->words = quan;

      typr->flag_text = malloc(sizeof(int) * typr->length);
      if (!typr->flag_text) return false;

      for (int i = 0; i < typr->length; i++)
            typr->flag_text[i] = 0;

      return true;
}

int get_desplacement(char *str, int i, int length)
{
      for (int tmp = 0; i < length; i++)
            if (tmp++ >= 80 && str[i] == ' ')
                  return i;

      return length;
}

void print_screen(typr_t *typr)
{
      erase();
      addstr("\n");
      attron(A_BOLD);

      int row, col;
      getmaxyx(stdscr, row, col);
      int x = get_desplacement(typr->text, 0, typr->length);
      move(row / 2 - 1, (col - x) / 2);
      int line = 0;
      int tmp = 0;

      for (int i = 0; i < typr->length; i++)
      {
            if (typr->flag_text[i] == BLANK)
                  attron(COLOR_PAIR(8));
            else if (typr->flag_text[i] == GOOD)
                  attron(COLOR_PAIR(2));
            else if (typr->flag_text[i] == BAD)
                  attron(COLOR_PAIR(3));

            if (strcmp(config.cursor, "underline") == 0
            &&  i == typr->index)
                  attron(A_UNDERLINE);
            else if (strcmp(config.cursor, "block") == 0
            &&  i == typr->index)
                  attron(COLOR_PAIR(9));

            printw("%c", typr->text[i]);

            if (tmp++ >= 80 && typr->text[i] == ' ')
            {
                  tmp = 0;
                  line++;
                  move(row / 2 - 1 + line, (col - x) / 2);
            }

            attroff(A_UNDERLINE);
            attron(COLOR_PAIR(8));
      }

      attroff(A_BOLD);

      attron(COLOR_PAIR(8));
      mvprintw(row - 1, col / 2 - 2, "typr");
      attroff(COLOR_PAIR(8));

      refresh();
}

int make_input(typr_t *typr, int ch)
{
      if (ch == typr->text[typr->index])
      {
            typr->flag_text[typr->index] = GOOD;
            typr->correct++;
            typr->index++;
      }

      else if (ch == KEY_END)
            return 1;

      else if (ch == KEY_RIGHT)
            return 2;

      else if (ch == KEY_BACKSPACE)
      {
            typr->flag_text[typr->index] = BLANK;
            if (typr->index > 0)
            {
                  typr->index--;
                  typr->flag_text[typr->index] = BLANK;
            }
      }

      else
      {
            typr->flag_text[typr->index] = BAD;
            typr->mistakes++;
            typr->index++;
      }

      return 0;
}

bool check_end(typr_t *typr)
{
      if (typr->index == typr->length)
      {
            typr->running = false;
            return true;
      }

      return false;
}

int type_test(typr_t *typr)
{
      struct timeval start;
      while (typr->running)
      {
            print_screen(typr);
            int ch = getch();

            if (typr->index == 0)
                  gettimeofday(&start, NULL);

            int res = make_input(typr, ch);
            if (res == 1)
                  return 1;
            if (res == 2)
                  return 2;

            if (check_end(typr))
                  break;
      }

      struct timeval end;
      gettimeofday(&end, NULL);

      unsigned long start_time = 1000000 * start.tv_sec + start.tv_usec;
      unsigned long end_time = 1000000 * end.tv_sec + end.tv_usec;

      typr->time = (end_time - start_time) / 1000000.0;
      typr->wpm = (typr->correct/5) / (typr->time/60);
      typr->accuracy = (typr->correct / (typr->correct + typr->mistakes)) * 100;

      return 0;
}

void show_info(typr_t *typr, records_t* records)
{
      int row, col;
      getmaxyx(stdscr, row, col);
      move(row / 2, col / 2 - 8);

      attron(COLOR_PAIR(4) | A_BOLD); printw("%.2f  ", typr->wpm);
      attroff(A_BOLD);

      if (typr->wpm < 100)
            addch(' ');
      if (typr->wpm < 10)
            addch(' ');

      if (typr->accuracy < 100)
            addch(' ');
      if (typr->accuracy < 10)
            addch(' ');

      attron(COLOR_PAIR(7)); printw("%.2f%%\n\n", typr->accuracy);

      move(row / 2 + 1, col / 2 - 8);
      attron(COLOR_PAIR(4) | A_BOLD); printw("%.2f", typr->time);

      if (typr->time < 100)
            addch(' ');
      if (typr->time < 10)
            addch(' ');

      attroff(A_BOLD); attron(COLOR_PAIR(7)); printw("  SECONDS\n");

      attron(COLOR_PAIR(8));
      mvprintw(row - 1, col / 2 - 2, "typr");
      attroff(COLOR_PAIR(8));
}

records_t *get_records()
{
      FILE *fp = fopen("files/records.txt", "r");
      if (!fp) return NULL;
      
      records_t *records = malloc(sizeof(records_t));
      if (!records)
      {
            fclose(fp);
            return NULL;
      }

      records->update = false;
      records->size = 0;
      records->array = calloc(10, sizeof(records_array_t));
      if (!records->array)
      {
            fclose(fp);
            free(records);
            return NULL;
      }

      char wpm[10], accuracy[10], seconds[10], words[10];
      memset(wpm, 0, sizeof(wpm) + sizeof(accuracy) + sizeof(seconds));
      while (fscanf(fp, "wpm=%[^ ] acc=%[^ ] sec=%[^ ] wrd=%[^\n]\n", wpm, accuracy, seconds, words) == 4 && records->size < 10)
      {
            if (strlen(wpm) == 0 || strlen(accuracy) == 0 || strlen(seconds) == 0)
            {
                  records->update = true;
                  memset(wpm, 0, sizeof(wpm) + sizeof(accuracy) + sizeof(seconds) + sizeof(words));
                  continue;
            }

            double Dwpm = atof(wpm);
            double Daccuracy = atof(accuracy);
            double Dseconds = atof(seconds);
            int Iwords = atoi(words);

            if (Dwpm < 0 || Daccuracy < 0 ||  Dseconds < 0 || Daccuracy > 100 || Iwords < 0)
            {
                  records->update = true;
                  memset(wpm, 0, sizeof(wpm) + sizeof(accuracy) + sizeof(seconds) + sizeof(words));
                  continue;
            }

            records->array[records->size].wpm = Dwpm;
            records->array[records->size].accuracy = Daccuracy;
            records->array[records->size].seconds = Dseconds;
            records->array[records->size++].words = Iwords;
            memset(wpm, 0, sizeof(wpm) + sizeof(accuracy) + sizeof(seconds) + sizeof(words));
      }
      
      fclose(fp);
      return records;
}

bool invalid_record(records_array_t record)
{
      if (record.wpm < 0 || record.accuracy < 0 || record.seconds < 0 || record.accuracy > 100 || record.words < 0)
            return true;
      return false;
}

bool update_records(typr_t *typr, records_t *records)
{
      if (records->size < 10)
            records->size++;

      if (records->size == 0)
      {
            records->array[0].wpm = typr->wpm;
            records->array[0].accuracy = typr->accuracy;
            records->array[0].seconds = typr->time;
            records->array[0].words = typr->words;
            return true;
      }

      for (int i = 0; i < records->size; i++)
      {
            if (invalid_record(records->array[i]))
            {
                  for (int j = i; j < records->size - 1; j++)
                        records->array[j] = records->array[j + 1];

                  records->size--;
            }

            if (typr->wpm > records->array[i].wpm)
            {
                  for (int j = records->size - 1; j > i; j--)
                        records->array[j] = records->array[j - 1];

                  records->array[i].wpm = typr->wpm;
                  records->array[i].accuracy = typr->accuracy;
                  records->array[i].seconds = typr->time;
                  records->array[i].words = typr->words;
                  return true;
            }
      }

      return records->update;
}

void update_file(records_t *records)
{
      FILE *fp = fopen("files/records.txt", "w");
      if (!fp) return;

      for (int i = 0; i < records->size; i++)
            fprintf(fp, "wpm=%.2f acc=%.2f sec=%.2f wrd=%i\n",
            records->array[i].wpm,
            records->array[i].accuracy,
            records->array[i].seconds,
            records->array[i].words);

      fclose(fp);
}

bool compare_records(records_array_t a, records_array_t b)
{
      if (a.wpm == b.wpm && a.accuracy == b.accuracy && a.seconds == b.seconds && a.words == b.words)
            return true;
      return false;
}

void print_records(records_t *records, records_array_t current)
{
      erase();

      int row, col;
      getmaxyx(stdscr, row, col);

      int record_pos = -1;
      for (int i = 0; i < records->size; i++)
            if (compare_records(records->array[i], current))
                  record_pos = i;

      if (record_pos != -1)
      {
            attron(COLOR_PAIR(2) | A_BOLD);
            mvprintw(row / 2 - records->size - 3, col / 2 - 5, "NEW RECORD!");
            attroff(COLOR_PAIR(2) | A_BOLD);
      }

      for (int i = 0; i < records->size; i++)
      {
            move(row / 2 - records->size - 1 + i, col / 2 - 19);

            attron(COLOR_PAIR(8));
            printw("%i:  ", i);

            if (records->array[i].wpm < 100)
                  addch(' ');
            if (records->array[i].wpm < 10)
                  addch(' ');

            attron(A_BOLD);
            if (record_pos != i)
                  attron(COLOR_PAIR(4));
            else  attron(COLOR_PAIR(2));

            printw("%.2f  ", records->array[i].wpm);
            if (record_pos != i) attroff(A_BOLD);

            if (records->array[i].accuracy < 100)
                  addch(' ');
            if (records->array[i].accuracy < 10)
                  addch(' ');

            if (record_pos != i) attron(COLOR_PAIR(7));
            printw("%.2f%%  ", records->array[i].accuracy);

            if (record_pos != i) attron(COLOR_PAIR(4));
            else attron(COLOR_PAIR(2));
            printw(" | ");
            if (record_pos != i) attron(COLOR_PAIR(7));

            if (records->array[i].seconds < 100)
                  addch(' ');
            if (records->array[i].seconds < 10)
                  addch(' ');

            printw("%.2f  ", records->array[i].seconds);

            if (records->array[i].words < 100)
                  addch(' ');
            if (records->array[i].words < 10)
                  addch(' ');

            if (record_pos != i) attron(COLOR_PAIR(4));
            printw("%i", records->array[i].words);
            attroff(A_BOLD);
      }
}

int rerun(typr_t *typr)
{
      records_t *records = get_records();
      if (update_records(typr, records))
            update_file(records);

      records_array_t current;
      current.wpm = typr->wpm;
      current.accuracy = typr->accuracy;
      current.seconds = typr->time;
      current.words = typr->words;
      print_records(records, current);
      show_info(typr, records);

      int row, col;
      getmaxyx(stdscr, row, col);
      move(row / 2 + 3, col / 2 - 12);

      attron(COLOR_PAIR(4)); printw("(");
      attron(COLOR_PAIR(7) | A_BOLD); printw("N");
      attroff(A_BOLD); attron(COLOR_PAIR(4)); printw(")ew  ");

      attron(COLOR_PAIR(4)); printw("(");
      attron(COLOR_PAIR(7) | A_BOLD); printw("R");
      attroff(A_BOLD); attron(COLOR_PAIR(4)); printw(")etry  ");

      attron(COLOR_PAIR(4)); printw("(");
      attron(COLOR_PAIR(7) | A_BOLD); printw("Q");
      attroff(A_BOLD); attron(COLOR_PAIR(4)); printw(")uit\n");

      free(records->array);
      free(records);

      for (int c = getch(); true; c = getch())
      {
            if (c == 'n' || c == 'N')
                  return 1;
            if (c == 'r' || c == 'R')
                  return 2;
            if (c == 'q' || c == 'Q')
                  return 3;
      }
}

void destroy_typr(typr_t *typr)
{
      free(typr->text);
      free(typr);
}

void destroy_words(char **words)
{
      for (int i = 0; i < config.words; i++)
            free(words[i]);
      free(words);
}

void restart_typr(typr_t *typr)
{
      typr->index = 0;
      typr->correct = 0;
      typr->mistakes = 0;
      typr->time = 0;
      typr->wpm = 0;
      typr->accuracy = 0;
      typr->running = true;

      memset(typr->flag_text, 0, sizeof(int) * typr->length);
}

int typr_logic(int quan)
{
      char **words = get_words(quan);
      if (!words) return 1;

      typr_t *typr = calloc(1, sizeof(typr_t));
      if (!typr) 
      {
            destroy_words(words);
            return 1;
      }

      fill_typr(typr, words, quan);

      int res_test = 0;
      int res_rerun = 0;
replay:
      res_test = type_test(typr);
      if (res_test == 1)
      {
            destroy_typr(typr);
            destroy_words(words);
            return 0;
      }

      if (res_test == 2)
      {
            restart_typr(typr);
            return 1;
      }

      res_rerun = rerun(typr);
      if (res_rerun == 2)
      {
            restart_typr(typr);
            goto replay;
      }

      destroy_typr(typr);
      destroy_words(words);

      if (res_rerun == 1)
            return 1;

      return 0;
}