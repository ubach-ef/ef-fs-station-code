struct winsize {
        unsigned short ws_row;
        unsigned short ws_col;
        unsigned short ws_xpixel;
        unsigned short ws_ypixel;
};

#define tIOC ('t'<<8)
#define _TIOC ('T'<<8)
#define TIOCSPGRP (tIOC|21) /* setp pgrp of tty */
#define TIOCSWINSZ (_TIOC|103)
#define TCGETS (_TIOC|13)
#define TCSETS (_TIOC|14)
