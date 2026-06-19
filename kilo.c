#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

struct termios orig_termios;

void die(const char* s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    /* 
     * "at one time or another, switching them off was considered 
     * (by someone) to be part of enabling “raw mode”, so we carry on the tradition 
     * (of whoever that someone was) in our program. 
     * As far as I can tell: 
     *
     * 1. When BRKINT is turned on, a break condition will cause a SIGINT signal to 
     * be sent to the program, like pressing Ctrl-C. 
     *
     * 2. INPCK enables parity checking, which doesn’t seem to apply to modern terminal 
     * emulators. 
     *
     * 3. ISTRIP causes the 8th bit of each input byte to be stripped, meaning it 
     * will set it to 0. This is probably already turned off. 
     *
     * 4. CS8 is not a flag, it is a bit mask with multiple bits, which we set using 
     * the bitwise-OR (|) operator unlike all the flags we are turning off. It sets the 
     * character size (CS) to 8 bits per byte. On my system, it’s already set that way."
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tsetattr");
}

int main() {
    enableRawMode();
    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}
