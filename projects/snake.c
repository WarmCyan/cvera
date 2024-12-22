#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "snake_core.c"

#define MAX_W 100
#define MAX_H 50

#define BORDER_COLOR_BG_GAMEOVER 41
#define BORDER_COLOR_BG 44
#define BORDER_COLOR_FG 30
#define SNAKE_COLOR_BG 42
#define FOOD_COLOR_FG 33

#define MSEC_PER_FRAME 50

static char cells[MAX_H][MAX_W];
static int term_w, term_h;
static int game_w, game_h;
static int food_x, food_y;
static int last_frame;

static int steps = 0;
static int last_rule = -1;

static struct termios original_term_attrs;

void printout() {
	if (snek_right) printf("snek_right ");
	/* if (snek_x) printf("snek_x "); */
	/* if (snek_y) printf("snek_y "); */
	/* if (snek_bigness) printf("snek_bigness "); */
	if (setup) printf("setup ");
	/* if (snek_tail_x) printf("snek_tail_x "); */
	/* if (snek_tail_y) printf("snek_tail_y "); */
	if (_o_clear_screen) printf("_o_clear_screen ");
	if (_o_find_term_size) printf("_o_find_term_size ");
	if (_o_prepare_frame_timer) printf("_o_prepare_frame_timer ");
	if (_o_prepare_term_for_input) printf("_o_prepare_term_for_input ");
	if (_o_generate_food) printf("_o_generate_food ");
	if (_o_render_boundaries) printf("_o_render_boundaries ");
	if (_o_render_snek) printf("_o_render_snek ");
	if (frame_rendered) printf("frame_rendered ");
	if (running) printf("running ");
	if (_i_next_frame) printf("_i_next_frame ");
	if (move_snek) printf("move_snek ");
	if (_o_check_food) printf("_o_check_food ");
	if (_i_food_get_happy_snek) printf("_i_food_get_happy_snek ");
	if (embiggen_snek) printf("embiggen_snek ");
	if (_i_no_food_get_sad_snek) printf("_i_no_food_get_sad_snek ");
	if (_o_clear_last_tail) printf("_o_clear_last_tail ");
	if (_o_check_ohnoes_ate_self) printf("_o_check_ohnoes_ate_self ");
	if (_o_check_snek_run_away) printf("_o_check_snek_run_away ");
	if (_i_snek_still_here) printf("_i_snek_still_here ");
	if (_i_where_snek_go) printf("_i_where_snek_go ");
	if (_i_no_eat_self_good_snek) printf("_i_no_eat_self_good_snek ");
	if (_i_OHNOES_ate_self_bad_snek) printf("_i_OHNOES_ate_self_bad_snek ");
	if (ded_snek) printf("ded_snek ");
	if (_o_DED) printf("_o_DED ");
	if (input_processing_loop) printf("input_processing_loop ");
	if (_i_input_received) printf("_i_input_received ");
	if (reorient_snek) printf("reorient_snek ");
	if (_) printf("_ ");
	if (ORIENTATION_LOGIC) printf("ORIENTATION_LOGIC ");
	if (no_change_if_dumb_snek) printf("no_change_if_dumb_snek ");
	if (snek_left) printf("snek_left ");
	if (_i_input_left) printf("_i_input_left ");
	if (_i_input_right) printf("_i_input_right ");
	if (snek_up) printf("snek_up ");
	if (_i_input_up) printf("_i_input_up ");
	if (_i_input_down) printf("_i_input_down ");
	if (snek_down) printf("snek_down ");
	if (yes_change_if_big_brain_snek) printf("yes_change_if_big_brain_snek ");
	if (MOVEMENT_LOGIC) printf("MOVEMENT_LOGIC ");
	if (_1_movements) printf("_1_movements ");
	if (snek_moved) printf("snek_moved ");
	if (minus_1_movements) printf("minus_1_movements ");
	if (minus_snek_x) printf("minus_snek_x ");
	if (minus_snek_y) printf("minus_snek_y ");
}


/* https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c */
void get_term_size() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    term_w = ws.ws_col;
    term_h = ws.ws_row;

    /* establish game size (-2 to account for borders) */
    game_w = (MAX_W < term_w-2) ? (MAX_W) : (term_w-2);
    game_h = (MAX_H < term_h-6) ? (MAX_H) : (term_h-6);
}

/* https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html */
/* https://codereview.stackexchange.com/questions/292621/get-terminal-size-enable-and-disable-terminal-raw-mode-without-ncurses */
int start_term_raw_mode() {
    /* save old attributes */
    if (tcgetattr(STDOUT_FILENO, &original_term_attrs) == -1) {
        return -1;
    }

    struct termios new_attrs = original_term_attrs;

    /* input modes - no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    new_attrs.c_iflag &= ~(0u | IGNBRK | BRKINT | PARMRK | INPCK | ISTRIP | /*ICRNL | 
                    INLCR | */ IGNCR | IXON);

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
    new_attrs.c_cc[VMIN] = 0;
    new_attrs.c_cc[VTIME] = 1;

    /* Change attributes when output has drained; also flush pending input. */
    return tcsetattr(STDOUT_FILENO, TCSAFLUSH, &new_attrs) == -1
        ? -2 
        : 0;
}

int stop_term_raw_mode() {
    return tcsetattr(STDOUT_FILENO, TCSAFLUSH, &original_term_attrs) == -1
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

    int bg_color = _o_DED ? BORDER_COLOR_BG_GAMEOVER : BORDER_COLOR_BG;

    printf("\033[%d;%dm", BORDER_COLOR_FG, bg_color); 
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
    
    int old_snek_tail_y = snek_tail_y;
    int old_snek_tail_x = snek_tail_x;
    
    if (cells[snek_tail_y][snek_tail_x] == 1) snek_tail_x++;
    else if (cells[snek_tail_y][snek_tail_x] == 2) snek_tail_y++;
    else if (cells[snek_tail_y][snek_tail_x] == 3) snek_tail_x--;
    else if (cells[snek_tail_y][snek_tail_x] == 4) snek_tail_y--;

    cells[old_snek_tail_y][old_snek_tail_x] = 0;
    
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

void check_snek_at_boundary() {
    if (snek_x < 0 || snek_x > game_w || snek_y < 0 || snek_y > game_h) 
        _i_where_snek_go = 1;
    else _i_snek_still_here = 1;
}

int time_ms() {
    struct timespec now;
    timespec_get(&now, TIME_UTC);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/* https://stackoverflow.com/questions/17167949/how-to-use-timer-in-c */
int is_time_for_next_frame() {
    int diff = time_ms() - last_frame; 
    /* int msec = diff * 1000*1000) / CLOCKS_PER_SEC; */
    /* printf("%d,",msec); */
    if (diff > MSEC_PER_FRAME) {
        /* printf("NEW"); */
        _i_next_frame = 1;
        return 1;
    }
    return 0;
}

/* returns nonzero if we should stop polling input */
int decipher_input(char* input) {
    if (input[0] == 27 && input[2] == 0) {
        /* esc was pressed, stop */
        stop_term_raw_mode();
        exit(0);
    }
    /* wsad, nehr, up down left right */
    else if (input[0] == 119 || input[0] == 110 || (input[0] == 27 && input[2] == 65))
        _i_input_up = 1;
    else if (input[0] == 115 || input[0] == 101 || (input[0] == 27 && input[2] == 66))
        _i_input_down = 1;
    else if (input[0] == 97 || input[0] == 104 || (input[0] == 27 && input[2] == 68))
        _i_input_left = 1;
    else if (input[0] == 100 || input[0] == 114 || (input[0] == 27 && input[2] == 67))
        _i_input_right = 1;
    else
        return 0;
    _i_input_received = 1;
    return 1;
}

void poll_input() {
    char input[3];
    while (!is_time_for_next_frame())
    {
        input[0] = 0;
        input[1] = 0;
        input[2] = 0;
        read(STDIN_FILENO, &input, 1); /* we need 3 bytes for things like arrows */
        if (input[0]) {
            if (input[0] == 27) {
                read(STDIN_FILENO, &input[1], 2); /* we need 3 bytes for things like arrows */
            }
            /* printf("%d,%d,%d", input[0], input[1], input[2]); */
            if (decipher_input(&input[0])) return;
        }
    }
}

void debug_lines() {
    move_cursor(0, game_h + 5);
    printf("\033[0K");
    move_cursor(0, game_h + 4);
    printf("\033[0K");
    move_cursor(0, game_h + 3);
    printf("\033[0K");
    printf("%d ", steps);
    printf("(%d) ", last_rule);
    printout();
}

int convert_number(char*s) {
    int value = 0;
    while (*s && *s >= '0' && *s <= '9') {
        value *= 10;
        value += *s - '0';
        s++;
    }
    return value;
}

int main(int argc, char* argv[]) {
    srand((unsigned)clock());

    int a = 1;
    int max_steps = -1; /* --steps [NUM] */
    int debug = 0; /* --debug */
    /* cli arg parsing */
    while (a < argc) {
        if (strcmp(argv[a], "--steps") == 0) {
            a++;
            max_steps = convert_number(argv[a]);
        }
        else if (strcmp(argv[a], "--debug") == 0) {
            debug = 1;
        }
        a++;
    }

    setbuf(stdout, NULL);
    /* setbuf(stdin, NULL); */
    printf("\033[?25l");

    while (!_o_DED) {
        last_rule = step();
        if (debug)
            debug_lines();
        steps++;
        if (_o_clear_screen) clear_screen();
        if (_o_find_term_size) get_term_size();
        if (_o_prepare_term_for_input) start_term_raw_mode();
        if (_o_render_boundaries) draw_boundaries();
        if (_o_render_snek) draw_snake_head();
        if (_o_generate_food) generate_food();
        if (_o_check_food) check_food();
        if (_o_check_ohnoes_ate_self) check_ohnoes_ate_self();
        if (_o_check_snek_run_away) check_snek_at_boundary();
        if (_o_clear_last_tail) erase_snake_tail();

        if (frame_rendered) last_frame = time_ms();

        /* the cells need to record which direction to go next so we can track
         * where to move tail, so we'll just listen in for these */
        if (move_snek && snek_right) cells[snek_y][snek_x] = 1;
        if (move_snek && snek_down) cells[snek_y][snek_x] = 2;
        if (move_snek && snek_left) cells[snek_y][snek_x] = 3;
        if (move_snek && snek_up) cells[snek_y][snek_x] = 4;
        
        /* weird edge case I think because vera nums can't go negative */
        if ((snek_y == 0 && minus_snek_y) || (snek_x == 0 && minus_snek_x)) {
            _i_where_snek_go = 1;
            _o_check_snek_run_away = 1;
        }

        if (input_processing_loop) poll_input();
        if (debug)
            debug_lines();
        /* break; */
        if (max_steps != -1 && steps >= max_steps) { break; }
    }
    draw_boundaries();
    /* printout(); */
    move_cursor(game_w+2, game_h+4);
    stop_term_raw_mode();
    printf("\033[?25h");
}
