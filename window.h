#define CH_DEL 127
#define CH_SPACE 32
#define CH_FSLASH 47
#define CH_FWD 261
#define CH_BACK 260
#define CH_COLON 58
#define CH_AT 64
#define MAX_CHARS 2000
#define NUM_TEXTS 6000

void set_cursor_pos(int x_offset, int y_offset);
void get_target_text(char* buffer);
int setup(void);
int get_wc(char *str);
int update_av_wpm(int elapsed, int num_words);
int update_accuracy(float acc);
int get_mode_option(void);
void show_stats(void);