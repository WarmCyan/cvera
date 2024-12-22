#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#include "snake_core.c"

static int term_w, term_h;

static struct termios original_term_attrs;


void printout() {
	printf("%d,", snek_right);
	printf("%d,", snek_x);
	printf("%d,", snek_y);
	printf("%d,", snek_bigness);
	printf("%d,", setup);
	printf("%d,", _o_find_term_size);
	printf("%d,", _o_prepare_frame_timer);
	printf("%d,", _o_prepare_term_for_input);
	printf("%d,", _o_generate_food);
	printf("%d,", _o_render_boundaries);
	printf("%d,", _o_render_snek);
	printf("%d,", _i_frame_rendered);
	printf("%d,", _i_begin);
	printf("%d,", _i_next_frame);
	printf("%d,", move_snek);
	printf("%d,", _o_check_food);
	printf("%d,", _o_check_ohnoes_ate_self);
	printf("%d,", _i_food_get_happy_snek);
	printf("%d,", embiggen_snek);
	printf("%d,", _i_no_food_get_sad_snek);
	printf("%d,", _o_clear_last_tail);
	printf("%d,", _i_no_eat_self_good_snek);
	printf("%d,", _i_OHNOES_ate_self_bad_snek);
	printf("%d,", ded_snek);
	printf("%d,", _o_DED);
	printf("%d,", input_processing_loop);
	printf("%d,", _i_input_received);
	printf("%d,", reorient_snek);
	printf("%d,", _);
	printf("%d,", ORIENTATION_LOGIC);
	printf("%d,", no_change_if_dumb_snek);
	printf("%d,", snek_left);
	printf("%d,", _i_input_left);
	printf("%d,", _i_input_right);
	printf("%d,", snek_up);
	printf("%d,", _i_input_up);
	printf("%d,", _i_input_down);
	printf("%d,", snek_down);
	printf("%d,", yes_change_if_big_brain_snek);
	printf("%d,", MOVEMENT_LOGIC);
	printf("%d,", _1_movements);
	printf("%d,", snek_moved);
	printf("%d,", minus_1_movements);
	printf("%d,", minus_snek_x);
	printf("%d,", minus_snek_y);
	printf("\n");
}

/* https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c */
void get_term_size() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    term_w = ws.ws_col;
    term_h = ws.ws_row;
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
    new_attrs.c_lflag &= ~(0u | IEXTEN | ECHO | ICANON);

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

int main() {
    while (!_o_DED) {
        step();
        if (_o_find_term_size) {
            get_term_size();
            printf("%d:%d\n", term_w, term_h);
        }
        if (_o_prepare_term_for_input) {
            int out = start_term_raw_mode();
            if (out == 0)
                printf("Yay!\n");
            else
                printf("Boo :(\n");
        }
        break;
    }
    printout();
    stop_term_raw_mode();
}
