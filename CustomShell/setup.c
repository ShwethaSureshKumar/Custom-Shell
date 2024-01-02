#include <ncurses.h>
#include <stdlib.h>

int main()
{
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();

    // Set up the progress bar
    int row = LINES / 2;
    int col = COLS / 2 - 10;
    int width = 20;
    int progress = 0;

    // Display the text and progress bar
    mvprintw(row - 2, col, "Loading HASH shell...");
    while (progress <= 100)
    {
        clear();
        mvprintw(row - 2, col, "Loading HASH shell...");
        mvprintw(row, col, "[");
        for (int i = 0; i < width; i++)
        {
            if (i < (progress * width) / 100)
            {
                addch('=');
            }
            else
            {
                addch(' ');
            }
        }
        mvprintw(row, col + width + 1, "]");
        refresh();
        progress++;
        napms(50);  // wait for 50 milliseconds
    }

    // Clean up and exit
    endwin();
    system("gnome-terminal \
                --window \
                --title='HASH Shell Terminal' \
                --profile=HASHProfile \
                --command='canberra-gtk-play --file=/usr/share/sounds/gnome/default/alerts/drip.ogg' \
                -e './hash'");
    return 0;
}
