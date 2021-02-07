#include "window.h"
#include <stdio.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    char typed[MAX_CHARS];
    int index;
  } Buffer;

typedef struct {
  int mistakes;
  volatile bool finished;
  float accuracy;
} Pstate;

void render_scr(char *target_text, char *typed_text, Pstate *pstate);

int main() {
  int error, av_wpm;
  Buffer buff;
  Pstate pstate;
  char target[MAX_CHARS];
  int counter = 0;
  time_t start_timer;
  int n_samples = get_mode_option();
  if(n_samples == - 1) {
    exit(EXIT_SUCCESS);
  }
  while(counter < n_samples) {
    int p;
    if((error = setup()) == -1) {
      exit(EXIT_FAILURE);
      } 
    buff.index = 0;
    for(int i = 0; i < MAX_CHARS; i++) {
      buff.typed[i] = '\0';
    }
    for(int i = 0; i < MAX_CHARS; i++) {
      target[i] = '\0';
    }
    get_target_text(target);
    (void) system("clear");
    render_scr(target, buff.typed, &pstate);
    start_timer = time(NULL);
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
      else if(ch >= CH_SPACE && ch <= CH_FSLASH) {
        p = buff.index;
        buff.typed[p] = ch;
        buff.index++;
        render_scr(target, buff.typed, &pstate);
      }
      else if(ch >= CH_COLON && ch <= CH_AT) {
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
      
      }
    }
    counter++;
    time_t elapsed = time(NULL) - start_timer;
    int av_wpm = update_av_wpm((int) elapsed, get_wc(target));
    //int update_acc = update_accuracy(pstate.accuracy);
  }
  endwin();
  exit(EXIT_SUCCESS);
}

void set_cursor_pos(int x_offset, int y_offset) {
  int x, y;
  getyx(stdscr, y, x);
  wmove(stdscr, y + y_offset, x + x_offset);
  wrefresh(stdscr);
}

void render_scr(char *target_text, char *typed_text, Pstate *pstate) {
  int i, j, x, y;
  int len_typed= 0;
  int correct = 0;
  char *top_message = "Press 9 to ESC";
  for(i = 0; i < MAX_CHARS; i++) {
    if(typed_text[i] != '\0'){
      len_typed++;
      continue;
    }
  }
  wmove(stdscr, 0, 0);
  addstr("Press 9 to ESC");
  wmove(stdscr, 2, 0);
  wrefresh(stdscr);
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
    pstate->accuracy = correct / (pstate->mistakes + correct);
  }
  if(pstate->finished == FALSE) {
    getyx(stdscr, y, x);
    wmove(stdscr, y, 0);
    set_cursor_pos(0, 2);
    for(i = 0; i < len_typed; i++) {
      addch(typed_text[i]);
    }
    
  }
  wrefresh(stdscr);
}

void get_target_text(char* buffer) {
    size_t buffsize = MAX_CHARS;
    int q = rand() % (NUM_TEXTS - 1);
    FILE* file;
    if((file = fopen("quotedata.txt", "r")) == NULL) {
      fprintf(stderr, "Error: Could not read from file\n");
    }
    for(int i = 0; i < q; i++) {
      getline(&buffer, &buffsize, file);
    }
    (void) fclose(file);
}

int setup(void) {
  srand(time(NULL));
  initscr();
  cbreak();
  start_color();
  use_default_colors();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
  noecho();
  idlok(stdscr, TRUE);
  keypad(stdscr, TRUE);
  return(0);
}

int update_av_wpm(int elapsed, int num_words) {
    int wpm = (int) ((num_words / elapsed) * 60);
    FILE *file;
    if((file = fopen("stats.txt", "r")) == NULL) {
      fprintf(stderr, "Error: Could not read file\n");
      exit(EXIT_FAILURE);
    }
    char *filebuff;
    size_t bufflen = MAX_CHARS;
    getline(&filebuff, &bufflen, file);
    int prev_av_wpm = atoi(filebuff);
    getline(&filebuff, &bufflen, file);
    int occurences = atoi(filebuff);
    int av_wpm = (int) (((prev_av_wpm * occurences) + wpm) / (occurences + 1));
    (void) fclose(file);
    if((file = fopen("stats.txt", "w")) == NULL) {
      fprintf(stderr, "Error: Could not write to file\n");
      exit(EXIT_FAILURE);
    }
    fprintf(file, "%d\n%d", av_wpm, occurences + 1);
    (void) fclose(file);
    return av_wpm;
}

int update_accuracy(float acc) {
    FILE *file;
    if((file = fopen("stats.txt", "r")) == NULL) {
      fprintf(stderr, "Error: Could not read file\n");
      exit(EXIT_FAILURE);
    }
    char *filebuff;
    size_t bufflen = MAX_CHARS;
    getline(&filebuff, &bufflen, file);
    int rewrite_wpm = atoi(filebuff);
    getline(&filebuff, &bufflen, file);
    int rewrite_wpm_occurences = atoi(filebuff);
    getline(&filebuff, &bufflen, file);
    float prev_acc = atoi(filebuff);
    getline(&filebuff, &bufflen, file);
    int occurences = atoi(filebuff);
    float av_acc = (((prev_acc * occurences) + acc) / (occurences + 1));
    (void) fclose(file);
    if((file = fopen("stats.txt", "w")) == NULL) {
      fprintf(stderr, "Error: Could not write to file\n");
      exit(EXIT_FAILURE);
    }
    fprintf(file, "%d\n%d\n%f\n%d", rewrite_wpm, rewrite_wpm_occurences, av_acc, occurences + 1);
    (void) fclose(file);
    return av_acc;
}
int get_wc(char *str) {
  int wc = 0;
  for(int i = 0; i < strlen(str); i++){
    if(str[i] == ' '){
      wc++;
    }
  }
  return wc;
}

int get_mode_option(void) {
  char* get_option;
  size_t option_buff_len = MAX_CHARS;
  printf("Practice (P) or view stats (S)?\n");
  while(TRUE) {
    getline(&get_option, &option_buff_len, stdin);
    if(strcmp(get_option, "P\n") == 0 || strcmp(get_option,"p\n") == 0 || strcmp(get_option,"practice\n") == 0) {
      printf("How many typing samples?\n");
      while(TRUE) {
        getline(&get_option, &option_buff_len, stdin);
        int n_samples = atoi(get_option);
        if(n_samples >= 0 && n_samples <= 1001) {
          return n_samples;
        }
        printf("Please choose number between 1 and 1000\n");
      }
    }
    else if(strcmp(get_option,"S\n") == 0 || strcmp(get_option,"s\n") == 0 || strcmp(get_option,"stats\n") == 0){
      show_stats();
      return(-1);
    }
    else{
      printf("Invalid option: choose P for practice session or S to view stats\n");
      printf("You chose: %s\n", get_option);
    }
  }
  return(-1);
}

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
    printf("AVERAGE WPM: %d\n", wpm);
    //printf("AVERAGE ACCURACY: %0.2f%%\n", acc);
  }
  else{
    printf("Could not open stats file\n");
  }
}