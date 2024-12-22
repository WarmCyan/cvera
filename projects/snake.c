#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>

#include "snake_core.c"

#define MAX_W 100
#define MAX_H 50

#define BORDER_COLOR_BG 44
#define BORDER_COLOR_FG 30

#define SNAKE_COLOR_BG 42

#define FOOD_COLOR_FG 33

static char cells[MAX_H][MAX_W];
static int term_w, term_h;
static int game_w, game_h;
static int food_x, food_y;

static struct termios original_term_attrs;


/* https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c */
void get_term_size() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    term_w = ws.ws_col;
    term_h = ws.ws_row;

    /* establish game size (-2 to account for borders) */
    game_w = (MAX_W < term_w-2) ? (MAX_W) : (term_w-2);
    game_h = (MAX_H < term_h-3) ? (MAX_H) : (term_h-3);
}

/* https://codereview.stackexchange.com/questions/292621/get-terminal-size-enable-and-disable-terminal-raw-mode-without-ncurses */
int start_term_raw_mode() {
    /* save old attributes */
    if (tcgetattr(STDIN_FILENO, &original_term_attrs) == -1) {
        return -1;
    }

    struct termios new_attrs = original_term_attrs;

    /* input modes - no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    new_attrs.c_iflag &= ~(0u | IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | ICRNL | 
                    INLCR | IGNCR | IXON);

    /* local modes - echoing off, canonical off, no extended functions, 
     * no signal chars (^Z, ^C) NOTE: acschually no, keep ^C */
    /* new_attrs.c_lflag &= ~(0u | ISIG | IEXTEN | ECHO | ICANON); */
    new_attrs.c_lflag &= ~(0u | ECHO | ICANON);

    /* control modes - set 8 bit chars. */
    new_attrs.c_cflag |= (0u | CS8);

    /* output modes - disable post processing. */
    new_attrs.c_oflag &= ~(0u | OPOST);

    /* control chars - set return condition: min number of bytes and timer.
     * We want read(2) to return every single byte, without timeout. */
    new_attrs.c_cc[VMIN] = 1;         /* 1 byte */
    new_attrs.c_cc[VTIME] = 0;        /* No timer */ /* TODO: we might actually want this? */

    /* Change attributes when output has drained; also flush pending input. */
    return tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_attrs) == -1
        ? -2 
        : 0;
}

int stop_term_raw_mode() {
    return tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term_attrs) == -1
        ? -2 
        : 0;
}

void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y+1, x+1);
    /* printf("%d;%dH", y, x); */
}

void clear_screen() {
    move_cursor(0, 0);
    printf("\033[0J");
}

void draw_boundaries() {
    move_cursor(0, 0);

    printf("\033[%d;%dm", BORDER_COLOR_FG, BORDER_COLOR_BG); 
    printf(" Vera snek");
    for (int i = 10; i <= game_w+2; i++) {
        printf(" ");
    }

    for (int i = 1; i < game_h+2; i++) {
        move_cursor(0, i);
        printf(" ");
        move_cursor(game_w+2, i);
        printf(" ");
    }

    move_cursor(0, game_h+2);
    for (int i = 0; i <= game_w+2; i++) {
        printf(" ");
    }
    printf("\033[0m");
    return;
}

void draw_snake_head() {
    /* cells[snek_y][snek_x] = 0; */
    move_cursor(snek_x + 1, snek_y + 1);
    printf("\033[%dm \033[0m", SNAKE_COLOR_BG);
}

void erase_snake_tail() {
    move_cursor(snek_tail_x + 1, snek_tail_y + 1);

    if (cells[snek_tail_y][snek_tail_x] == 1) snek_tail_x++;
    else if (cells[snek_tail_y][snek_tail_x] == 2) snek_tail_y++;
    else if (cells[snek_tail_y][snek_tail_x] == 3) snek_tail_x--;
    else if (cells[snek_tail_y][snek_tail_x] == 4) snek_tail_y--;
    
    printf("\033[0m ");
}

void generate_food() {
    int valid = 0;
    while (!valid) {
        food_x = rand() % game_w;
        food_y = rand() % game_h;
        valid = !cells[food_y][food_x];
    }
    move_cursor(food_x + 1, food_y + 1);
    printf("\033[%dm#\033[0m", FOOD_COLOR_FG);
}

void check_food() {
    if (snek_x == food_x && snek_y == food_y) _i_food_get_happy_snek = 1;
    else _i_no_food_get_sad_snek = 1;
}

void check_ohnoes_ate_self() {
    if (cells[snek_y][snek_x]) _i_OHNOES_ate_self_bad_snek = 1;
    else _i_no_eat_self_good_snek = 1;
}

int main() {
    int steps = 0;
    srand((unsigned)clock());
    while (!_o_DED) {
        step();
        steps++;
        if (_o_clear_screen) clear_screen();
        if (_o_find_term_size) get_term_size();
        if (_o_prepare_term_for_input) start_term_raw_mode();
        if (_o_render_boundaries) draw_boundaries();
        if (_o_render_snek) draw_snake_head();
        if (_o_generate_food) generate_food();
        if (_o_check_food) check_food();
        if (_o_check_ohnoes_ate_self) check_ohnoes_ate_self();

        /* the cells need to record which direction to go next so we can track
         * where to move tail, so we'll just listen in for these */
        if (move_snek && snek_right) cells[snek_y][snek_x] = 1;
        if (move_snek && snek_down) cells[snek_y][snek_x] = 2;
        if (move_snek && snek_left) cells[snek_y][snek_x] = 3;
        if (move_snek && snek_up) cells[snek_y][snek_x] = 4;

        break;
        if (steps > 100) { break; }
    }
    /* printout(); */
    move_cursor(game_w+2, game_h+2);
    stop_term_raw_mode();
}
