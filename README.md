# Custom-Shell
Custom shell implementation using C

This custom shell project aims to create a versatile command-line interface providing users with a range of functionalities for efficient navigation and execution of commands within the file system.

Basic Command Execution: Users can execute fundamental commands such as cd, pwd, mkdir, ls, nano, touch, who, sleep, echo, rm, head, tail, date, cat, cp, mv, man, wc.
Custom Commands: It introduces custom functionalities like playing songs using the playlist command and displaying images using the image command.
Command History Management: The shell maintains a command history, facilitating swift access to previously executed commands for enhanced user convenience.
ncurses-based Loading Screen: The loading screen employs the ncurses library to create a dynamic and visually appealing loading sequence, including a progress bar that visually represents the loading status of the shell.
Interactive Terminal Setup: Additionally, the project involves launching a new terminal window with predefined settings and executing a program named 'hash' post-loading, ensuring a seamless transition into the customized shell environment.
This setup can be initiated by executing the ./setup command, which orchestrates the loading sequence, including the progress bar, and finally opens the terminal window with the 'HASH Shell Terminal' title and predefined settings.
