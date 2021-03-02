#pragma once
#include <stdbool.h>
#define CH_DEL 127
#define CH_SPACE 32
#define CH_FSLASH 47
#define CH_FWD 261
#define CH_BACK 260
#define CH_COLON 58
#define CH_AT 64
#define MAX_CHARS 2000
#define NUM_TEXTS 6000
#define EXIT_MAIN -1
#define PRACTICE_MIN 1
#define PRACTICE_MAX 1000
#define QUOTES "quotedata.txt"
#define STATS "stats.txt"
#define STATS_FLAG "-S"
#define RESET_FLAG "-R"

typedef struct {
    char *typed;
    int index;
  } Buffer;

typedef struct {
  float mistakes;
  bool finished;
  float accuracy;
} Pstate;

void render_scr(char *target_text, char *typed_text, Pstate *pstate);
void set_cursor_offset(int x_offset, int y_offset);
void get_target_text(char* buffer);
int setup(void);
int get_wc(char *str);
int get_mode_option(void);
void show_stats(void);
void try_reset_stats();
void update_av_wpm_accuracy(int elapsed, int num_words, float acc); 
bool check_str_mathches(char *str, const char **comparisons, int n);