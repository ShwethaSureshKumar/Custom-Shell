/* ---- hash Shell ---- */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <utmp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <glob.h>
#include <time.h>
#include <libgen.h>

#define hash_TOKEN_BUFFERSIZE 64
#define hash_TOKEN_DELIMITER " "
#define BUFFER_SIZE 1024
#define MAX_SONGS 5
#define SONGS_DIR "/home/shwetha/project/songs"

char cwd[1024];
int usage = 0; // counter variable to store the number of external commands used
int count_lines = 0, count_words = 0, count_bytes = 0;

char *builtin_command[] = {
	"cd", "pwd", "mkdir", "ls","nano",
	"touch", "who", "sleep", "echo" , "rm",
	"head", "tail", "date", "cat", "cp",
	"mv", "playlist","man", "wc", "image"};

char* songs[MAX_SONGS] = {
    "Kesariya.mp3",
    "Dandelions.mp3",
    "Venmegam.mp3",
    "melliname.mp3",
    "NewyorkNagaram.mp3"
};

/* shell commands */
int hash_cd(char **internal_command);
int hash_pwd(char **internal_command);
int hash_mkdir(char **internal_command);
int hash_ls(char **internal_command);
int hash_nano(char **internal_command);
int hash_who(char **internal_command);
int hash_sleep(char **internal_command);
int hash_touch(char **internal_command);
int hash_remove(char **internal_command);
int hash_head(char **internal_command);
int hash_echo(char **internal_command);
int hash_tail(char **internal_command);
int hash_date(char **internal_command);
int hash_cat(char **internal_command);
int hash_cp(char **internal_command);
int hash_mv(char **internal_command);
int hash_playlist();
int hash_man(char **internal_command);
int hash_wc(char **internal_command);
int hash_image(char **internal_command);


int (*execute_builtin_command[])(char **) = {
	&hash_cd,
	&hash_pwd,
	&hash_mkdir,
	&hash_ls,
	&hash_nano,
	&hash_touch,
	&hash_who,
	&hash_sleep,
	&hash_echo,
	&hash_remove,
	&hash_head,
	&hash_tail,
	&hash_date,
	&hash_cat,
	&hash_cp,
	&hash_mv,
	&hash_playlist,
	&hash_man,
	&hash_wc,
	&hash_image};

int hash_total_builtin_command()
{
	return sizeof(builtin_command) / sizeof(char *);
}

void sigint_handler(int sig_num)
{
	signal(SIGINT, SIG_DFL);
	printf("\nCtrl+D (EOF) detected. Exiting...\n");
	exit(0);
}

/* Implementation of internal command (cd) */
int hash_cd(char **internal_command)
{
	char *home_dir;
	struct passwd *pwd;
	pwd = getpwuid(getuid());
	home_dir = pwd->pw_dir;

	if (internal_command[1] == NULL)
	{
		chdir(home_dir);
	}
	else if ((strcmp(internal_command[1], "~") == 0) || (strcmp(internal_command[1], "~/") == 0))
	{
		chdir(home_dir);
	}
	else if (strcmp(internal_command[1],"..")==0)
	{
		if(strcmp(getcwd(cwd,sizeof(cwd)),home_dir)==0)
		{
			printf("In Root Directory!!\n");
		}
		else
		{
			chdir("..");
		}
	}
	else if(chdir(internal_command[1]) < 0)
	{
		printf("hash: cd: %s: No such file or directory\n", internal_command[1]);
	}
}

int hash_execute(char **single_command, int background)
{
	int i;

	if (single_command[0] == NULL)
	{
		return 1;
	}
	//check if there are internal commands
	for (i = 0; i < hash_total_builtin_command(); i++)
	{
		if (strcmp(single_command[0], builtin_command[i]) == 0)
		{
			return (*execute_builtin_command[i])(single_command);
		}
	}
}

int hash_pwd(char **internal_command)
{
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
	}
	else
	{
		perror("getcwd() error");
	}
}

int hash_mkdir(char **internal_command)
{
    //char **my_argv;
    //my_argv[0] = internal_command[1];
    //my_argv[1] = NULL;
    if(internal_command[1] == NULL)
    {
        printf("Usage : mkdir missing arg\n");
    }
    else
    {
        int check;
        check = mkdir(internal_command[1],0777);
    // check if directory is created or not
        if (!check)
            printf("Directory created\n");
        else {
            printf("Unable to create directory\n");
            exit(1);
        }
    }
}

int hash_ls(char **internal_command)
{
    int count = 0, i =0, show_hidden = 0;
    DIR *dir;
    struct dirent *dir_name;
    struct stat st;
    struct passwd *pw;
    struct group *gr;
    char datestring[256];
    char cwd[1024];
    while(internal_command[i]!=NULL)
    {
        count++;
        i++;
    }
    if(count == 1)
    {
        dir = opendir(".");
        while((dir_name = readdir(dir)) != NULL)
        {
            if(show_hidden==0 && dir_name->d_name[0]=='.')
                continue;
            printf("%s\t",dir_name->d_name);
        }
        printf("\n");
    }
    else if(count == 2)
    {
        if(internal_command[1][0] == '-')
        {
            dir = opendir(".");
            if(strcmp(internal_command[1],"-a") == 0)
                show_hidden = 1;
        }
        else
        {
            dir = opendir(internal_command[1]);
        }
    }
    else
    {
        dir = opendir(internal_command[2]);
        if(strcmp(internal_command[1],"-a") == 0)
        {
            show_hidden = 1;
        }
    }
    if(dir == NULL)
    {
        perror("opendir");
        return 1;
    }
    while((dir_name = readdir(dir)) != NULL && count != 1)
    {
        if(show_hidden==0 && dir_name->d_name[0]=='.')
            continue;
        if(strcmp(internal_command[1],"-a") == 0)
            printf("%s\t",dir_name->d_name);
        else if(strcmp(internal_command[1],"-l") == 0)
        {
            if (stat(dir_name->d_name, &st) == -1)
            {
                perror("stat");
                exit(EXIT_FAILURE);
            }
            printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
            printf( (st.st_mode & S_IRUSR) ? "r" : "-");
            printf( (st.st_mode & S_IWUSR) ? "w" : "-");
            printf( (st.st_mode & S_IXUSR) ? "x" : "-");
            printf( (st.st_mode & S_IRGRP) ? "r" : "-");
            printf( (st.st_mode & S_IWGRP) ? "w" : "-");
            printf( (st.st_mode & S_IXGRP) ? "x" : "-");
            printf( (st.st_mode & S_IROTH) ? "r" : "-");
            printf( (st.st_mode & S_IWOTH) ? "w" : "-");
            printf( (st.st_mode & S_IXOTH) ? "x" : "-");
            printf(" ");
            // print number of hard links
            printf("%ld ", (long) st.st_nlink);
            // print owner name and group name
            pw = getpwuid(st.st_uid);
            gr = getgrgid(st.st_gid);
            printf("%s %s ", pw->pw_name, gr->gr_name);
            // print file size
            printf("%5ld ", (long) st.st_size);
            // print modification time
            strftime(datestring, sizeof(datestring), "%b %d %H:%M", localtime(&st.st_mtime));
            printf("%s ", datestring);
            // print filename
            printf("%s\n", dir_name->d_name);
        }
        else
            printf("%s\t",dir_name->d_name);
    }
    printf("\n");
    closedir(dir);
}

int hash_nano(char **internal_command)
{
	char *filename = internal_command[1];
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("read");
        return 0;
    }

    char *new_contents = NULL;
    size_t new_size = 0;

    signal(SIGINT, sigint_handler);

    printf("\nEnter text (press Ctrl-D to save and exit):\n");
    while ((bytes_read = getline(&new_contents, &new_size, stdin)) != -1) {
        write(fd, new_contents, bytes_read);
        free(new_contents);
        new_contents = NULL;
        new_size = 0;
    }
    free(new_contents);
    close(fd);
    return 1;
}

int hash_touch(char **internal_command)
{
    int i, count =0 ;
    while(internal_command[i]!=NULL)
    {
        count++;
        i++;
    }
    for (int i = 1; i < count; i++)
    {
        // Check if file exists
        int fd = open(internal_command[i], O_RDONLY);
        if (fd == -1)
        {
            // File doesn't exist, create it
            fd = creat(internal_command[i], 0666);
            if (fd == -1)
            {
                perror("creat");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // File already exists, close the file descriptor
            close(fd);
        }
        // Update modification time to current time
        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t now = tv.tv_sec;
        struct utimbuf times = {now, now};
        if (utime(internal_command[i], &times) == -1)
        {
            perror("utime");
            exit(EXIT_FAILURE);
        }
    }
}
int hash_echo(char **internal_command)
{
    int i, nflag = 0, count=0;
    while(internal_command[i]!=NULL)
    {
        count++;
        i++;
    }
    // Check for -n flag
    if (count > 1 && strcmp(internal_command[1], "-n") == 0) {
        nflag = 1;
        i = 2;
    } else {
        i = 1;
    }

    for (; i < count; i++) {
        glob_t results;
        int ret;

        // Check if argument contains a wildcard
        if (strchr(internal_command[i], '*') != NULL) {
            // Use glob to find matching file names
            ret = glob(internal_command[i], 0, NULL, &results);
            if (ret == 0) {
                // Print all matching file names
                for (size_t j = 0; j < results.gl_pathc; j++) {
                    printf("%s ", results.gl_pathv[j]);
                }
                globfree(&results);
            }
        } else {
            // If argument doesn't contain a wildcard, print it as it is
            printf("%s", internal_command[i]);
            if (i < count-1) {
                printf(" ");
            }
        }
    }

    // If -n flag is not set, print a newline
    if (!nflag) {
        printf("\n");
    }

    return 0;
}

int hash_who(char **internal_command)
{
    struct utmp *utmp_entry;
    int utmp_fd;
    if ((utmp_fd = open("/var/run/utmp", O_RDONLY)) == -1)
    {
        perror("open");
        return 1;
    }
    while ((utmp_entry = getutent()) != NULL)
    {
        if (utmp_entry->ut_type == USER_PROCESS)
        {
            printf("%-8.8s %-8.8s %10ld (%s)\n", utmp_entry->ut_user, utmp_entry->ut_line, (long int)utmp_entry->ut_tv.tv_sec, utmp_entry->ut_host);
        }
    }
    endutent();
    close(utmp_fd);
}

int hash_sleep(char **internal_command)
{
    int count = 0 , i = 0;
    while(internal_command[i]!=NULL)
    {
        count++;
        i++;
    }
    for (i = 1; i < count; i++)
    {
        int len = strlen(internal_command[i]);
        char suffix = internal_command[i][len - 1];
        int value = atoi(internal_command[i]);
        if (suffix == 's')
        {
            sleep(value);
        }
        else if (suffix == 'm')
        {
            sleep(value*60);
        }
        else
        {
            sleep(value);
        }
    }
    return 0;
}

// Check if a file exists
int file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

int remove_file(const char* file_path, int interactive, int recursive, int force) {
    int ret = 0;

    if (interactive) {
        // Prompt the user before deleting the file
        printf("remove %s? (y/n) ", file_path);
        char answer = getchar();
        if (answer != 'y' && answer != 'Y') {
            printf("Not removed.\n");
            return 0;
        }
    }

    if (unlink(file_path) == -1) {
        if (errno == EISDIR && recursive) {
            // The file is a directory and the -r option is enabled
            DIR* dir;
            struct dirent* dirent;

            // Open the directory
            if ((dir = opendir(file_path)) == NULL) {
                perror("opendir");
                return 1;
            }

            // Recursively remove all files in the directory
            while ((dirent = readdir(dir)) != NULL) {
                char new_file_path[BUFFER_SIZE];
                if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
                    continue;
                }
                snprintf(new_file_path, BUFFER_SIZE, "%s/%s", file_path, dirent->d_name);
                ret |= remove_file(new_file_path, interactive, recursive, force);
            }

            // Close the directory
            if (closedir(dir) == -1) {
                perror("closedir");
                return 1;
            }

            // Remove the directory itself
            if (rmdir(file_path) == -1) {
                if (!force) {
                    perror("rmdir");
                    ret |= 1;
                }
            }
        } else {
            // The file is not a directory or the -r option is not enabled
            if (!force) {
                perror("unlink");
                ret |= 1;
            }
        }
    }

    return ret;
}

int hash_remove(char **internal_command)
{
    int j = 0, ret = 0, i, count = 0;
    int interactive = 0, recursive = 0, force = 0;
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    // Parse command-line arguments
    for (i = 1; i < count; i++)
    {
        if (strcmp(internal_command[i], "-i") == 0)
        {
            interactive = 1;
        }
        else if (strcmp(internal_command[i], "-r") == 0 || strcmp(internal_command[i], "-R") == 0)
        {
            recursive = 1;
        }
        else if (strcmp(internal_command[i], "-f") == 0 || strcmp(internal_command[i], "--force") == 0)
        {
            force = 1;
        }
        else
        {
            break;
        }
    }
    if (i == count) {
        fprintf(stderr, "Usage: %s [-i] [-r|-R] [-f|--force] file1 [file2 ...]\n", internal_command[0]);
        return 1;
    }
    // Remove each file
    for (; i < count; i++)
    {
        ret |= remove_file(internal_command[i], interactive, recursive, force);
    }

    return 0;
}

void head(int n, FILE *fp)
 {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int count = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        if (++count > n) {
            break;
        }
        printf("%s", line);
    }

    free(line);
}

int hash_head(char **internal_command)
{
    FILE *fp;
    int n = 10, i = 0, count = 0;
    while(internal_command[i]!=NULL)
    {
        count++;
        i++;
    }
    if (count > 1 && strcmp(internal_command[1], "-n") == 0 && count > 2) {
        n = atoi(internal_command[2]);
        internal_command += 2;
        count -= 2;
    }

    if (count == 1)
    {
        // read from standard input
        head(n, stdin);
    }
    else
    {
        // read from file
        fp = fopen(internal_command[1], "r");
        if (fp == NULL)
        {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        head(n, fp);
        fclose(fp);
    }
    return 0;
}

void print_tail(FILE* fp, int n)
{
    char** lines = (char**) malloc(n * sizeof(char*));
    if (lines == NULL)
    {
        printf("Error: memory allocation failed.\n");
        return;
    }
    for (int i = 0; i < n; i++)
    {
        lines[i] = NULL;
    }
    int line_count = 0;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        if (lines[line_count % n] != NULL)
        {
            free(lines[line_count % n]);
        }
        lines[line_count % n] = strdup(buffer);
        line_count++;
    }
    int start = line_count < n ? 0 : line_count % n;
    for (int i = start; i < start + n && i < line_count; i++)
    {
        printf("%s", lines[i % n]);
    }
    for (int i = 0; i < n; i++)
    {
        free(lines[i]);
    }
    free(lines);
}

int hash_tail(char **internal_command)
{
    FILE* fp;
    int n = 10;
    int i = 1, count = 0, j = 0;
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    // Parse command line arguments
    while (i < count && internal_command[i][0] == '-') {
        if (internal_command[i][1] == 'n' && i + 1 < count) {
            n = atoi(internal_command[i+1]);
            if (n <= 0) {
                printf("Invalid argument: %s\n", internal_command[i+1]);
                return 1;
            }
            i += 2;
        } else {
            printf("Invalid option: %s\n", internal_command[i]);
            return 1;
        }
    }
    if (i == count) {
        // Read from standard input
        fp = stdin;
        print_tail(fp, n);
    } else {
        // Read from file(s)
        for (int j = i; j < count; j++) {
            fp = fopen(internal_command[j], "r");
            if (fp == NULL) {
                printf("Error: could not open file %s\n", internal_command[j]);
                continue;
            }
            printf("==> %s <==\n", internal_command[j]);
            print_tail(fp, n);
            fclose(fp);
        }
    }
    return 0;
}

int hash_date(char **internal_command)
{
    time_t current_time;
    struct tm* time_info;
    char time_string[80];
    int j, count = 0;
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    // Set the default format string
    const char* format_string = "%a %b %d %H:%M:%S %Z %Y";
    // Parse command line arguments
    for (int i = 1; i < count; i++)
    {
        if (strcmp(internal_command[i], "-u") == 0)
        {
            // Use UTC time instead of local time
            current_time = time(NULL);
            time_info = gmtime(&current_time);
            strftime(time_string, sizeof(time_string), format_string, time_info);
            printf("%s\n", time_string);
            return 0;
        }
        else if (strcmp(internal_command[i], "+%s") == 0)
        {
            // Print the timestamp as seconds since the epoch
            current_time = time(NULL);
            printf("%ld\n", current_time);
            return 0;
        }
        else if (strncmp(internal_command[i], "+", 1) == 0)
        {
            // Use the specified format string
            if (strlen(internal_command[i]) > 1 && internal_command[i][1] == '+') {
                // format string starts with "+", skip the first char
                format_string = internal_command[i] + 1;
            } else {
                format_string = internal_command[i];
            }
        }
        else
        {
            // Invalid option
            fprintf(stderr, "Invalid option: %s\n", internal_command[i]);
            return 1;
        }
    }
    // Get the current time and format it
    current_time = time(NULL);
    time_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), format_string, time_info);
    printf("%s\n", time_string);
    return 0;
}

int hash_cat(char **internal_command)
{
    FILE *fp;
    int c, n, option, squeezed, escape, j=0;
    char buffer[BUFFER_SIZE];
    option = 0;
    squeezed = 0;
    escape = 0;
    int count = 0;
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    if (count > 1 && internal_command[1][0] == '-') {
        if (strcmp(internal_command[1], "-n") == 0) {
            option = 1;
        } else if (strcmp(internal_command[1], "-s") == 0) {
            squeezed = 1;
        } else if (strcmp(internal_command[1], "-E") == 0) {
            escape = 1;
        } else {
            printf("Invalid option\n");
            return 1;
        }
        count--;
        internal_command++;
    }
    // Check for file names
    if (count < 2) {
        printf("Usage: %s [-n] [-s] [-E] file1 [file2 ...]\n", internal_command[0]);
        return 1;
    }
    // Print contents of each file
    for (int i = 1; i < count; i++) {
        fp = fopen(internal_command[i], "r");
        if (fp == NULL) {
            printf("Error: Could not open file %s\n", internal_command[i]);
            continue;
        }
        // Print line numbers if -n option is specified
        n = 1;
        if (option) {
            printf("%6d ", n);
        }
        // Print file contents
        while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
            if (squeezed && buffer[0] == '\n')
            {
                continue; // skip empty lines
            }
            if (option)
            {
                printf("%6d ", ++n);
            }
            if (escape)
            {
                for (int i = 0; i < strlen(buffer); i++)
                {
                    switch (buffer[i])
                    {
                        case '\n':
                            printf("$\n");
                            break;
                        case '\t':
                            printf("^I");
                            break;
                        case '\b':
                            printf("^H");
                            break;
                        default:
                            printf("%c", buffer[i]);
                    }
                }
            }
            else
            {
                printf("%s", buffer);
            }
        }
        fclose(fp);
    }
    return 0;
}

void copy_file(const char *src_path, const char *dst_path)
{
    int input_fd, output_fd;
    ssize_t num_read;
    char buffer[BUFSIZ];

    // open input file
    if ((input_fd = open(src_path, O_RDONLY)) == -1) {
        perror(src_path);
        //exit(EXIT_FAILURE);
    }

    // open output file
    if ((output_fd = creat(dst_path, 0644)) == -1) {
        perror(dst_path);
        //exit(EXIT_FAILURE);
    }

    // copy file contents
    while ((num_read = read(input_fd, buffer, BUFSIZ)) > 0) {
        if (write(output_fd, buffer, num_read) != num_read) {
            perror("Write error");
            //exit(EXIT_FAILURE);
        }
    }

    // close files
    if (close(input_fd) == -1) {
        perror("Input file close error");
        //exit(EXIT_FAILURE);
    }
    if (close(output_fd) == -1) {
        perror("Output file close error");
        //exit(EXIT_FAILURE);
    }
}

void copy_dir(const char *src_path, const char *dst_path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat src_stat;
    char src_file[PATH_MAX];
    char dst_file[PATH_MAX];

    // create destination directory if it doesn't exist
    if (stat(dst_path, &src_stat) == -1)
    {
        mkdir(dst_path, 0755);
    }

    // open source directory
    if ((dir = opendir(src_path)) == NULL)
    {
        perror(src_path);
        //exit(EXIT_FAILURE);
    }

    // copy directory contents
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(src_file, PATH_MAX, "%s/%s", src_path, entry->d_name);
        snprintf(dst_file, PATH_MAX, "%s/%s", dst_path, entry->d_name);

        if (lstat(src_file, &src_stat) == -1) {
            perror(src_file);
            continue;
        }

        if (S_ISREG(src_stat.st_mode)) {
            copy_file(src_file, dst_file);
        } else if (S_ISDIR(src_stat.st_mode)) {
            copy_dir(src_file, dst_file);
        }
    }
    // close directory
    closedir(dir);
}

void move_file(const char *src_path, const char *dst_path)
{
    if (rename(src_path, dst_path) == -1)
    {
        perror("Error moving file");
        //exit(EXIT_FAILURE);
    }
}

void move_dir(const char *src_path, const char *dst_path)
{
    if (rename(src_path, dst_path) == -1)
    {
        perror("Error moving directory");
        //exit(EXIT_FAILURE);
    }
}

int hash_cp(char **internal_command)
{
    struct stat src_stat, dst_stat;
    char *src_file_name;
    char *dst_file_name;
    char dst_file[PATH_MAX];
    int src_is_file = 0;
    int dst_is_file = 0;
    int count = 0, j=0;
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    if (count < 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", internal_command[0]);
        //exit(EXIT_FAILURE);
    }

    // get source and destination file names
    src_file_name = internal_command[1];
    dst_file_name = internal_command[2];

    // get source and destination file stats
    if (lstat(src_file_name, &src_stat) == -1)
    {
        perror(src_file_name);
        //exit(EXIT_FAILURE);
    }
    if (lstat(dst_file_name, &dst_stat) != -1)
    {
        // check if destination exists and is a directory or a file
        if (S_ISREG(dst_stat.st_mode))
        {
            dst_is_file = 1;
        }
        else if (S_ISDIR(dst_stat.st_mode))
        {
            dst_is_file = 0;
        }
        else
        {
            fprintf(stderr, "Destination is not a file or directory\n");
            //exit(EXIT_FAILURE);
        }
    }
    else
    {
        // destination doesn't exist
        dst_is_file = 0;
    }

    if (S_ISREG(src_stat.st_mode)) {
        src_is_file = 1;
    } else if (S_ISDIR(src_stat.st_mode)) {
        src_is_file = 0;
    } else {
        fprintf(stderr, "Source is not a file or directory\n");
        //exit(EXIT_FAILURE);
    }

    if (src_is_file) {
        if (dst_is_file) {
            // copy file to file
            copy_file(src_file_name, dst_file_name);
        } else {
            // copy file to directory
            snprintf(dst_file, PATH_MAX, "%s/%s", dst_file_name, basename(src_file_name));
            copy_file(src_file_name, dst_file);
        }
    } else {
        if (dst_is_file) {
            fprintf(stderr, "Cannot copy directory to file\n");
            //exit(EXIT_FAILURE);
        } else {
            // copy directory to directory
            copy_dir(src_file_name, dst_file_name);
        }
    }

    // move file or directory
    if (count > 3 && strcmp(internal_command[3], "-m") == 0) {
        if (src_is_file) {
            if (dst_is_file) {
                // move file to file
                move_file(src_file_name, dst_file_name);
            } else {
                // move file to directory
                snprintf(dst_file, PATH_MAX, "%s/%s", dst_file_name, basename(src_file_name));
                move_file(src_file_name, dst_file);
            }
        } else {
            if (dst_is_file) {
                fprintf(stderr, "Cannot move directory to file\n");
               // exit(EXIT_FAILURE);
            } else {
                // move directory to directory
                move_dir(src_file_name, dst_file_name);
            }
        }
    }
    //exit(EXIT_SUCCESS);
}

int hash_mv(char **internal_command)
{
    int force = 0; // -f option
    int verbose = 0; // -v option
    int i = 1, j = 0, count = 0; // starting index of filenames
    // Parse options
    while(internal_command[j]!=NULL)
    {
        count++;
        j++;
    }
    while (i < count && internal_command[i][0] == '-') {
        if (strcmp(internal_command[i], "-f") == 0) {
            force = 1;
        } else if (strcmp(internal_command[i], "-v") == 0) {
            verbose = 1;
        } else {
            printf("Invalid option: %s\n", internal_command[i]);
            return 1;
        }
        i++;
    }
    // Check if source and destination files are provided
    if (count - i != 2) {
        printf("Usage: mv [-f] [-v] [source_file] [destination_file]\n");
        return 1;
    }
    // Move file
    int status = rename(internal_command[i], internal_command[i+1]);

    if (status == 0)
    {
        if (verbose)
        {
            printf("'%s' -> '%s'\n", internal_command[i], internal_command[i+1]);
        }
    }
    else
    {
        if (!force)
        {
            printf("Unable to move '%s' to '%s'.\n", internal_command[i], internal_command[i+1]);
            return 1;
        }
        else
        {
            unlink(internal_command[i+1]); // delete existing file
            status = rename(internal_command[i], internal_command[i+1]); // try again
            if (status == 0 && verbose)
            {
                printf("'%s' -> '%s'\n", internal_command[i], internal_command[i+1]);
            }
            else
            {
                printf("Unable to move '%s' to '%s'.\n", internal_command[i], internal_command[i+1]);
                return 1;
            }
        }
    }

    return 0;
}

void play_audio(const char* file_path)
{
    char command[1000];
    sprintf(command, "mpg123 \"%s/%s\"", SONGS_DIR, file_path);
    system(command);
}

void play_song(int song_number) {
    if (song_number < 1 || song_number > MAX_SONGS)
    {
        printf("Invalid song number\n");
    }
    else
    {
        char* song_path = songs[song_number - 1];
        play_audio(song_path);
    }
}

int hash_playlist()
{
    printf("Available songs:\n");
    int i;
    for (i = 0; i < MAX_SONGS; i++)
    {
        printf("%d. %s\n", i + 1, songs[i]);
    }
    printf("Enter song number to play (1-%d) or 0 to exit: ", MAX_SONGS);
    int song_number;
    scanf("%d", &song_number);
    if (song_number == 0)
    {
        return 0;
    }
    else
    {
        play_song(song_number);
        hash_playlist();
    }
}

int hash_image(char **internal_command)
{
    char filename[100];
    char command[200];
    sprintf(command, "xdg-open %s", internal_command[1]);  // Format the command string with the filename
    system(command);  // Execute the command
    return 0;
}

int hash_man(char **internal_command)
{
	FILE *fp;
    	int c, n,j=0;
    	char buffer[BUFFER_SIZE];
    	int count = 0;
	const char *dir_path = "/home/harini/project/manual/";
        char filepath[256];
    	while(internal_command[j]!=NULL)
    	{
        	count++;
        	j++;
    	}
    	// Check for file names
    	if (count < 2)
	{
        	printf("Usage: man [command name]\n");
        	return 1;
    	}
    	// Print contents of each file
    	for (int i = 1; i < count; i++)
	{
		char *filename = internal_command[1];
        	sprintf(filepath,"%s%s.txt",dir_path,filename);
        	fp = fopen(filepath,"r");
        	if (fp == NULL) {
            		printf("Error: Could not find man %s\n", internal_command[i]);
            		continue;
        	}
       	 	// Print file contents
        	while (fgets(buffer, BUFFER_SIZE, fp) != NULL)
        	{
            		printf("%s", buffer);
        	}
		printf("\n\n");
        	fclose(fp);
    	}
    	return 0;
}

void print_counts(char *filename, int lines, int words, int bytes, int print_lines, int print_words, int print_bytes)
{
    if (print_lines) {
        printf("%4d ", lines);
    }

    if (print_words) {
        printf("%4d ", words);
    }

    if (print_bytes) {
        printf("%4d ", bytes);
    }

    printf("  %s\n", filename);
}

int hash_wc(char **internal_command)
{
    FILE *fp;
    int ch, in_word = 0, i, print_lines = 1, print_words = 1, print_bytes = 1, count = 0, j = 0;
    char *filename;
    while(internal_command[j]!=NULL)
	{
		j++;
		count++;
	}
    for (i = 1; i < count; i++)
    {
        if (internal_command[i][0] == '-')
        {
            if (strlen(internal_command[i]) < 2)
            {
                printf("Invalid option: %s\n", internal_command[i]);
                exit(EXIT_FAILURE);
            }
            switch(internal_command[i][1])
            {
                case 'l':
                    print_lines = 1;
                    print_words = 0;
                    print_bytes = 0;
                    break;
                case 'w':
                    print_words = 1;
                    print_lines = 0;
                    print_bytes = 0;
                    break;
                case 'c':
                    print_bytes = 1;
                    print_lines = 0;
                    print_words = 0;
                    break;
                default:
                    printf("Invalid option: %s\n", internal_command[i]);
                    exit(EXIT_FAILURE);
            }
        }
        else
        {
            filename = internal_command[i];
            fp = fopen(filename, "r");
            if (fp == NULL)
            {
                printf("Unable to open file: %s\n", filename);
                exit(EXIT_FAILURE);
            }

            while ((ch = getc(fp)) != EOF)
            {
                count_bytes++;
                if (ch == '\n')
                {
                    count_lines++;
                }
                if (isspace(ch))
                {
                    in_word = 0;
                }
                else if (!in_word)
                {
                    in_word = 1;
                    count_words++;
                }
            }
            print_counts(filename, count_lines, count_words, count_bytes, print_lines, print_words, print_bytes);
            count_lines = 0;
            count_words = 0;
            count_bytes = 0;
            fclose(fp);
        }
    }
    return 0;
}

void hash_prompt()
{
	char prompt_msg[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		strcpy(prompt_msg, "hash:~");
		strcat(prompt_msg, cwd);
		strcat(prompt_msg, "$ ");
		printf("%s", prompt_msg);
	}
	else
		perror("getcwd error..");
}

int disp_history()
{
	FILE *fptr;
	fptr = fopen("history.txt", "r");
	int count = 0;
	if (fptr != NULL)
	{
		char str[100];
		while (fgets(str, sizeof(str), fptr) != NULL)
		{
			printf("#%d - %s", count + 1, str);
			count = count + 1;
		}
	}
	if (count == 0)
		return 0;
	else
		return 1;
	fclose(fptr);
}

/* writes cmds into history file */
void write_history(char *command)
{
	FILE *fptr;
	fptr = fopen("history.txt", "a");
	if (fptr != NULL)
	{
		fprintf(fptr, "%s\n", command);
	}
	fclose(fptr);
}

/* returns the cmd from history corresponding to the history number */
char *return_history(int history_number)
{
	FILE *fptr;
	char str[100];
	char *history;
	fptr = fopen("history.txt", "r");
	int count = 0;
	if (fptr != NULL)
	{
		while (fgets(str, sizeof(str), fptr) != NULL)
		{
			count = count + 1;
			if (count == history_number)
			{
				fclose(fptr);
				strcpy(history, str);
				return history;
			}
		}
	}
	fclose(fptr);
	return NULL;
}

void write_usage_log()
{
	FILE *fptr;
	char *home_dir;
	struct passwd *pwd;
	pwd = getpwuid(getuid());
	home_dir = pwd->pw_dir;
	fptr = fopen(strcat(home_dir, "/.usage.log"), "a");
	fprintf(fptr, "%d\n", usage);
	fclose(fptr);
}

char **hash_split_command(char *input_command, int *count)
{
	int buffer_size = hash_TOKEN_BUFFERSIZE, position = 0;
	char **tokens = malloc(buffer_size * sizeof(char *));
	char *token, **tokens_backup;

	if (!tokens)
	{
		fprintf(stderr, "hash: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(input_command, hash_TOKEN_DELIMITER);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;

		if (position >= buffer_size)
		{
			buffer_size += hash_TOKEN_BUFFERSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, buffer_size * sizeof(char *));

			if (!tokens)
			{
				free(tokens_backup);
				fprintf(stderr, "hash: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, hash_TOKEN_DELIMITER);
	}
	tokens[position] = NULL;
	*count = position;
	return tokens;
}

/* parses the commanfd and tokenizes the input string */
void hash_parse_input_command(char *input_command)
{
	// an array of single cmds / tokens are obtained
	int token_count;
	char **command = hash_split_command(input_command, &token_count);
	// here we implement the logic to check for redirection
	int type = 0;

	for (int i = 1; i < token_count; i++)
	{
		if (strcmp(command[i], "&") == 0)
		{
			// run as background process
			type = 1;
			command[i] = NULL;
			hash_execute(command,1);
		}
		else if ((strcmp(command[i], ">") == 0) || (strcmp(command[i], ">>") == 0))
		{
			// output redirection
			// stdout -> file
			type = 1;
			int saved_stdout = dup(1);
			int output_file;

			if (strcmp(command[i], ">") == 0)
			{
				FILE *fptr = fopen(command[i + 1], "w");
				fclose(fptr);
				output_file = open(command[i + 1], O_WRONLY);
			}
			else
			{
				FILE *fptr = fopen(command[i + 1], "a");
				fclose(fptr);
				output_file = open(command[i + 1], O_WRONLY | O_APPEND);
			}

			int redirect_stream = dup2(output_file, 1);
			command[i] = NULL;
			hash_execute(command,0);
			dup2(saved_stdout, 1);
			close(saved_stdout);
		}
		else if (strcmp(command[i], "<") == 0)
		{
			// input redirection
			// file -> stdin
			type = 1;
			int input_file = open(command[i + 1], O_RDONLY);
			if (input_file > 0)
			{
				int saved_stdin = dup(0);
				int redirect_stream = dup2(input_file, 0);
				command[i] = NULL;
				hash_execute(command,0);
				dup2(saved_stdin, 0);
				close(saved_stdin);
			}
			else
			{
				perror(strcat(command[i + 1], " not found"));
			}
		}
	}
	if (type == 0)
	{
		hash_execute(command,0);
	}
}

int main(int argv, char **argc)
{
	struct sigaction act;
	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGINT, &act, NULL);
	char new_line_checker[2] = {"\n"};
	char *input_command = NULL; // Stores the shell inputs
	ssize_t input_command_buffer = 0;
	int flag = 0, history_choice;
	int history_flag = 0;
	signal(SIGINT, sigint_handler);
	while (1)
	{
	    history_flag = 0;
		hash_prompt();
		getline(&input_command, &input_command_buffer, stdin); // accept input
		if (strcmp(input_command, new_line_checker) == 0)
		{
			continue;
		}
		input_command[strlen(input_command) - 1] = '\0';
		if (strcmp(input_command, "exit") == 0) // terminate shell on exit cmd
		{
			printf("exiting hash shell\n");
			flag = 1;
			write_history(input_command);
			write_usage_log();
			exit(0);
		}
		if (strcmp(input_command, "history") == 0)
		{
			int count = disp_history();
			if (count == 0)
			{
				printf("No Recent history!!\n");
				continue;
			}
			else
			{
				printf("\nEnter the history number : ");
				scanf("%d", &history_choice);
				input_command = return_history(history_choice);
				if (input_command == NULL)
				{
					printf("wrong input\n");
					continue;
				}
				write_history(input_command);
				input_command[strlen(input_command) - 1] = '\0';
				history_flag = 1;
			}
		}
		if (history_flag == 0)
		{
			write_history(input_command);
		}
		hash_parse_input_command(input_command); // handle and parse internal and external cmds
		input_command = NULL;
	}
	return EXIT_SUCCESS;
}

