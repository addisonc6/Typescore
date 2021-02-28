#include "window.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int main(int argc, char **argv) {
  if(argc > 2) {
    fprintf(stdout, "Usage: ./typescore [practice samples]\n");
    exit(EXIT_FAILURE);
  }
  
  srand(time(NULL));
  int error, av_wpm;
  Buffer buff;
  Pstate pstate;
  buff.typed = (char*) calloc(MAX_CHARS, sizeof(char));
  char *target = (char*) calloc(MAX_CHARS, sizeof(char));
  int practice_counter = 0;
  int n_samples;
  if(argv[1] == NULL) {
    n_samples = get_mode_option();
    if(n_samples == EXIT_MAIN) {
    exit(EXIT_SUCCESS);
    }
  }
  else {
    n_samples = atoi(argv[1]);
  }
  if((error = setup()) != 0) {
      exit(EXIT_FAILURE);
      } 
  while(practice_counter < n_samples) {
    int p;
    buff.index = 0;
    memset(buff.typed, 0, sizeof(char) * MAX_CHARS);
    memset(target, 0, sizeof(char) * MAX_CHARS);
    get_target_text(target);
    (void) system("clear");
    clear();
    wmove(stdscr, 0, 0);
    printw("Press 9 to ESC : %d/%d", practice_counter + 1, n_samples);
    render_scr(target, buff.typed, &pstate);
    time_t start_timer = time(NULL);
    pstate.mistakes = 0;
    pstate.finished = FALSE;
    while(pstate.finished == FALSE) {
      wrefresh(stdscr);
      int ch = getch();
      if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
        p = buff.index;
        buff.typed[p] = ch;
        buff.index++;
        render_scr(target, buff.typed, &pstate);
      }
      else if((ch >= CH_SPACE && ch <= CH_FSLASH) || (ch >= CH_COLON && ch <= CH_AT)) {
        p = buff.index;
        buff.typed[p] = ch;
        buff.index++;
        render_scr(target, buff.typed, &pstate);
      }
      else if(ch == '9') {
        endwin();
        exit(EXIT_SUCCESS);
      }
      else {
        //ignore keypress
      }
    }
    practice_counter++;
    time_t elapsed = time(NULL) - start_timer;
    update_av_wpm_accuracy((int) elapsed, get_wc(target), pstate.accuracy);
  }
  endwin();
  free(buff.typed);
  free(target);
  exit(EXIT_SUCCESS);
}

//set cursor offset from current position
void set_cursor_offset(int y_offset, int x_offset) {
  int x, y;
  getyx(stdscr, y, x);
  wmove(stdscr, y + y_offset, x + x_offset);
}

//redraw screen to update match between paragraph and typed text
void render_scr(char *target_text, char *typed_text, Pstate *pstate) {
  int i, x, y;
  int len_typed = 0;
  float correct = 0;
  for(i = 0; i < MAX_CHARS; i++) {
    if(typed_text[i] != '\0'){
      len_typed++;
      continue;
    }
  }
  wmove(stdscr, 4, 0);
  for(i = 0; i < strlen(target_text); i++) {
    if(target_text[i] == typed_text[i]) {
      addch(target_text[i] | COLOR_PAIR(2));
      correct++;
    }
    else if(i == strlen(typed_text)) {
      addch(target_text[i] | A_UNDERLINE);
    }
    else if(strlen(typed_text) < i + 1) {
      addch(target_text[i] | COLOR_PAIR(3));
    }
    else {
      addch(target_text[i] | COLOR_PAIR(1));
      pstate->mistakes++;
    }
  }
  if(strlen(typed_text) >= strlen(target_text) - 1) {
    pstate->finished = TRUE;
    pstate->accuracy = 100 * (correct / strlen(target_text));
  }
  if(pstate->finished == FALSE) {
    getyx(stdscr, y, x);
    wmove(stdscr, y, 0);
    set_cursor_offset(2, 0);
    for(i = 0; i < len_typed; i++) {
      addch(typed_text[i]);
    }
  }
}

//picks from a sample of the quotes
void get_target_text(char* buffer) {
    size_t buffsize = MAX_CHARS;
    int quote_num = rand() % (NUM_TEXTS - 1);
    FILE* file;
    if((file = fopen(QUOTES, "r")) == NULL) {
      fprintf(stderr, "Error: Could not read from file\n");
    }
    for(int i = 0; i < quote_num; i++) {
      getline(&buffer, &buffsize, file);
    }
    (void) fclose(file);
}

//curses initializations
int setup(void) {
  int error = FALSE;
  (void) initscr();
  if((error = cbreak()) != 0) {
    return(error);
  }
  if(has_colors() != TRUE) {
    fprintf(stderr, "This program requires terminal color capability\n");
    endwin();
    exit(EXIT_FAILURE);
  }
  (void) start_color();
  (void) use_default_colors();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
  if((error = noecho()) != 0) {
    return(error);
  }
  (void) idlok(stdscr, TRUE);
  (void) keypad(stdscr, TRUE);
  return(error);
}

//update wpm and accuracy
void update_av_wpm_accuracy(int elapsed, int num_words, float acc) {
  int wpm = (int) (( (float) num_words / (float) elapsed) * 60);
  FILE *file;
  if((file = fopen("stats.txt", "r")) == NULL) {
    fprintf(stderr, "Error: Could not read file\n");
    exit(EXIT_FAILURE);
  }
  char *filebuff;
  size_t bufflen = MAX_CHARS;
  getline(&filebuff, &bufflen, file);
  int prev_wpm = atoi(filebuff);
  getline(&filebuff, &bufflen, file);
  int wpm_occurences = atoi(filebuff);
  getline(&filebuff, &bufflen, file);
  float prev_acc = atof(filebuff);
  getline(&filebuff, &bufflen, file);
  int acc_occurences = atoi(filebuff);
  int av_wpm = (int) ((((float) prev_wpm * wpm_occurences) + wpm) / (wpm_occurences + 1));
  float av_acc = (((prev_acc * acc_occurences) + acc) / (acc_occurences + 1));
  (void) fclose(file);
  if((file = fopen("stats.txt", "w")) == NULL) {
    fprintf(stderr, "Error: Could not write to file\n");
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%d\n%d\n%f\n%d", av_wpm, wpm_occurences + 1, av_acc, acc_occurences + 1);
  (void) fclose(file);
}

//gets word counter in a paragraph
int get_wc(char *str) {
  int wc = 0;
  for(int i = 0; i < strlen(str); i++){
    if(str[i] == ' '){
      wc++;
    }
  }
  return wc;
}

//get action from user at program start
int get_mode_option(void) {
  char* get_option;
  size_t option_buff_len = MAX_CHARS;
  fprintf(stdout, "Practice (P), or view Stats (S)?\n");
  while(TRUE) {
    getline(&get_option, &option_buff_len, stdin);
    if(strcmp(get_option, "P\n") == 0 || strcmp(get_option,"p\n") == 0 || strcmp(get_option,"practice\n") == 0) {
      printf("How many typing samples?\n");
      while(TRUE) {
        getline(&get_option, &option_buff_len, stdin);
        int n_samples = atoi(get_option);
        if(n_samples >= PRACTICE_MIN && n_samples <= PRACTICE_MAX) {
          return n_samples;
        }
        fprintf(stdout, "Please choose number between 1 and 1000\n");
      }
    }
    else if(strcmp(get_option,"S\n") == 0 || strcmp(get_option,"s\n") == 0 || strcmp(get_option,"stats\n") == 0){
      show_stats();
      return(EXIT_MAIN);
    }
    else{
      fprintf(stdout, "Invalid option: choose P for practice session or S to view stats\n");
      fprintf(stdout, "You chose: %s\n", get_option);
    }
  }
  return(EXIT_MAIN);
}

//print stats to stdout
void show_stats(void) {
  int wpm;
  int wpm_occurence;
  float acc;
  FILE *filep;
  if((filep = fopen("stats.txt", "rb")) != NULL) {
    fscanf(filep, "%d", &wpm);
    fscanf(filep, "%d", &wpm_occurence);
    fscanf(filep, "%f", &acc);
    (void) fclose(filep);
    fprintf(stdout, "##### STATS #####\n\n");
    fprintf(stdout, "AVERAGE WPM: %d\n", wpm);
    fprintf(stdout, "AVERAGE ACCURACY: %0.2f%%\n\n", acc);
    fprintf(stdout, "#################\n");
  }
  else{
    fprintf(stderr, "Could not open stats file\n");
  }
}

