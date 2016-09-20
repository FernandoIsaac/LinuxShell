#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/procfs.h>
#include <libgen.h>
#include <pwd.h>
#include <fstream>
#include <sys/utsname.h>
#include <math.h>
#include <signal.h>
#ifdef _WIN32
#include <windows.h>
#define chdir _chdir

#else
#include <unistd.h>
#endif

#define MAX_LENGTH 1024
#define DELIMS " \t\r\n"


#define MAX_BUF 1024
#define INT_SIZE_BUF 6
#define PID_LIST_BLOCK 32
#define UP_TIME_SIZE 1
#define token_buffer_size 64
#define token_delim " \t\r\n\a"

using namespace std;
void shell_loop();
char ** shell_split_pipeParams(char *);
char **shell_split_line(char*);
int shell_execute_pipe(char **, char**);
int shell_execute(char **);
int shell_launch(char **);
int shell_num_builtins();
int shell_ls(char**);
int shell_cd(char**);
int shell_mkdir(char**);
int shell_chmod(char**);
int shell_rmdir(char**);
int shell_rm(char**);
int shell_cat(char**);
int shell_ln(char**);
int shell_ps(char**);
int shell_uname(char(**));
int shell_kill(char**);
int shell_help(char**);
int shell_exit(char**);
static int
one (const struct dirent *unused) {
    return 1;
}

static char perms_buff[30];

const char *get_perms(mode_t mode) {
    char ftype = '?';

    if (S_ISREG(mode)) ftype = '-';
    if (S_ISLNK(mode)) ftype = 'l';
    if (S_ISDIR(mode)) ftype = 'd';
    if (S_ISBLK(mode)) ftype = 'b';
    if (S_ISCHR(mode)) ftype = 'c';
    if (S_ISFIFO(mode)) ftype = '|';

    sprintf(perms_buff, "%c%c%c%c%c%c%c%c%c%c %c%c%c", ftype,
            mode & S_IRUSR ? 'r' : '-',
            mode & S_IWUSR ? 'w' : '-',
            mode & S_IXUSR ? 'x' : '-',
            mode & S_IRGRP ? 'r' : '-',
            mode & S_IWGRP ? 'w' : '-',
            mode & S_IXGRP ? 'x' : '-',
            mode & S_IROTH ? 'r' : '-',
            mode & S_IWOTH ? 'w' : '-',
            mode & S_IXOTH ? 'x' : '-',
            mode & S_ISUID ? 'U' : '-',
            mode & S_ISGID ? 'G' : '-',
            mode & S_ISVTX ? 'S' : '-');

    return (const char *)perms_buff;
}
const char *getUserName(int uid)
{
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        return pw->pw_name;
    }

    return "";
}
int check_if_number (char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit (str[i]))
        {
            return 0;
        }
    }
    return 1;
}

void pidaux ()
{
    DIR *dirp;
    FILE *fp;
    struct dirent *entry;
    char path[MAX_BUF], read_buf[MAX_BUF], temp_buf[MAX_BUF];
    char uid_int_str[INT_SIZE_BUF] = {0}, *line;
    char uptime_str[UP_TIME_SIZE];
    char *user, *command;
    size_t len = 0;
    dirp = opendir ("/proc/");
    if (dirp == NULL)
    {
        perror ("Fail");
        exit(0);
    }
    strcpy(path, "/proc/");
    strcat(path, "uptime");

    fp = fopen(path, "r");
    if (fp != NULL)
    {
        getline(&line, &len, fp);
        sscanf(line, "%s ", uptime_str);
    }
    long uptime = atof(uptime_str);
    long Hertz = sysconf(_SC_CLK_TCK);
    strcpy(path, "/proc/");
    strcat(path, "meminfo");

    fp = fopen(path, "r");
    unsigned long long total_memory;
    if (fp != NULL)
    {
        getline(&line, &len, fp);
        sscanf(line, "MemTotal:        %llu kB", &total_memory);
    }

    while ((entry = readdir (dirp)) != NULL)
    {
        if (check_if_number (entry->d_name))
        {
            strcpy(path, "/proc/");
            strcat(path, entry->d_name);
            strcat(path, "/status");
            unsigned long long memory_rss;
            fp = fopen(path, "r");
            unsigned long long vmsize;

            if (fp != NULL)
            {
                vmsize = 0;
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                sscanf(line, "Uid:    %s ", uid_int_str);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                sscanf(line, "VmSize:    %llu kB", &vmsize);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                getline(&line, &len, fp);
                sscanf(line, "VmRSS:     %llu kB", &memory_rss);

            }
            else
            {
                fprintf(stdout, "FP is NULL\n");
            }
            float memory_usage = 100 * memory_rss / total_memory;
            strcpy(path, "/proc/");
            strcat(path, entry->d_name);
            strcat(path, "/stat");
            fp = fopen(path, "r");
            getline(&line, &len, fp);
            char comm[10], state;
            unsigned int flags;
            int pid, ppid, pgrp, session, tty_nr, tpgid;
            unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
            unsigned long long starttime;
            long cutime, cstime, priority, nice, num_threads, itreavalue;
            sscanf(line, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu", &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime, &cutime, &cstime, &priority, &nice, &num_threads, &itreavalue, &starttime);
            unsigned long total_time = utime + stime;
            total_time = total_time + (unsigned long)cutime + (unsigned long)cstime;
            float seconds = uptime - (starttime / Hertz);
            float cpu_usage = 100 * ((total_time / Hertz) / seconds);
            if (isnan(cpu_usage))
            {
                cpu_usage = 0.0;
            }
            if (isnan(memory_usage))
            {
                memory_usage = 0.0;
            }
            strcpy (path, "/proc/");
            strcat (path, entry->d_name);
            strcat (path, "/comm");

            fp = fopen (path, "r");
            if (fp != NULL)
            {
                fscanf (fp, "%s", read_buf);
                fclose(fp);
            }
            //char *userName=getUserName(atoi(uid_int_str));
            char userName[1000];
            //userName=getUserName(atoi(uid_int_str));
            strcpy(userName, getUserName(atoi(uid_int_str)));
            if (strlen(userName) < 9)
            {
                user = userName;
            }
            else
            {
                user = uid_int_str;
            }
            fprintf(stdout, "%s %s %0.1f %0.1f %llu %llu %c %s\n", user, entry->d_name, cpu_usage, memory_usage, vmsize, memory_rss, state, read_buf);

        }
    }
    closedir (dirp);
}

int removedir(char path[500]) {
    int counter = 1;
    DIR *pdir = NULL;
    struct dirent *pent = NULL;
    struct stat eStat;
    pdir = opendir(path);
    char x[500];
    if ( pdir == NULL) {
        return -1;
    }
    while ( (pent = readdir(pdir)) != NULL ) {
        if ((strcmp((pent->d_name), ".") == 0) || (strcmp((pent->d_name), "..") == 0)) {
            continue;
        }
        else {
            strcpy(x, path);
            path = strcat(path, "/");
            path = strcat(path, pent->d_name);

            if (stat(path, &eStat)) {
                printf("ERROR: %s... Meaning it can be a file(Most certainly)\n", strerror(errno));
            }
            else {
                if (S_ISDIR(eStat.st_mode)) {
                    removedir(path);
                    strcpy(path, x);
                }
                else {
                    unlink(path);
                    strcpy(path, x);
                }
            }
        }
    }
    if (!rmdir(path)) return -1; // delete the directory */
}


char pathname[MAXPATHLEN];
char const *builtin_str[] = {"ls", "cd", "mkdir", "chmod", "rmdir", "rm", "cat", "uname", "kill", "ln", "ps",  "help", "exit"};
int (*builtin_func[])(char**) = {
    &shell_ls,
    &shell_cd,
    &shell_mkdir,
    &shell_chmod,
    &shell_rmdir,
    &shell_rm,
    &shell_cat,
    &shell_uname,
    &shell_kill,
    &shell_ln,
    &shell_ps,
    &shell_help,
    &shell_exit
};
int num_params = 0;
int num_tokens = 0;
int main(int argc, char const *argv[])
{
    shell_loop();
    return EXIT_SUCCESS;
}
void shell_loop() {
    char *line;
    size_t buffer_size;
    char **tokens1;
    char **tokens2;
    char **params;
    int status;
    do {
        cout << "mi_sh> ";
        line = NULL;
        buffer_size = 0;
        getline(&line, &buffer_size, stdin);//obtiene la linea de comando

        params = shell_split_pipeParams(line);
        for (int i = 0; i < num_params; i = i + 2) {
            if (i + 1 == num_params) {
                tokens1 = shell_split_line(params[i]);
                status = shell_execute(tokens1);
            } else {

                tokens1 = shell_split_line(params[i]);
                tokens2 = shell_split_line(params[i + 1]);

                status = shell_execute_pipe(tokens1, tokens2);

            }


        }

        free(line);
        free(params);
    } while (status);
}
char ** shell_split_pipeParams(char* line) {
    int buffer_size = token_buffer_size;
    int p_position = 0;
    char **params = (char **) malloc(buffer_size * sizeof(char *));
    char *param;
    if (!params) {
        cout << "Error de asignacion de memoria" << endl;
        exit(EXIT_FAILURE);
    }
    param = strtok(line, "|");
    while (param != NULL) {

        params[p_position] = param;
        p_position++;
        if (p_position >= buffer_size) {
            buffer_size += token_buffer_size;
            params = (char **) realloc(NULL, buffer_size * sizeof(char *));
            if (!params) {
                cout << "Error de asignacion de memoria" << endl;
                exit(EXIT_FAILURE);
            }
        }
        param = strtok(NULL, "|");
    }
    params[p_position] = NULL;
    num_params = p_position;
    return params;
}
char ** shell_split_line(char* line) {
    int buffer_size = token_buffer_size;
    int t_position = 0;
    char **tokens = (char **) malloc(buffer_size * sizeof(char *));
    char *token;
    if (!tokens) {
        cout << "Error de asignacion de memoria" << endl;
        exit(EXIT_FAILURE);
    }
    token = strtok(line, token_delim);
    while (token != NULL) {

        tokens[t_position] = token;
        t_position++;
        if (t_position >= buffer_size) {
            buffer_size += token_buffer_size;
            tokens = (char **) realloc(NULL, buffer_size * sizeof(char *));
            if (!tokens) {
                cout << "Error de asignacion de memoria" << endl;
                exit(EXIT_FAILURE);
            }

        }
        token = strtok(NULL, token_delim);
    }
    tokens[t_position] = NULL;
    return tokens;
}
int shell_launch(char** tokens) {
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
            perror("mi_sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("mi_sh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
int shell_execute(char** tokens) {
    if (tokens[0] == NULL) {
        return 1;
    }
    for (int i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(tokens[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(tokens);
        }
    }

    return shell_launch(tokens);
}
int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
int shell_ls(char** tokens) {
    int tokenCount = 0;
    while (tokens[tokenCount] != NULL) {
        tokenCount++;
    }
    tokenCount--;
    if (tokenCount == 0) {
        char cwd[1024];
        char *curr_dir = NULL;
        DIR *dp = NULL;
        struct dirent *dptr = NULL;
        unsigned int count = 0;

        curr_dir = getcwd(cwd, sizeof(cwd));

        if (curr_dir == NULL) {
            perror("\n ERROR : Could not get the working directory\n");

        }

        dp = opendir((const char*)curr_dir);
        if (dp == NULL) {
            perror("\n ERROR : Could not open the working directory\n");

        }

        printf("\n");
        for (count = 0; NULL != (dptr = readdir(dp)); count++) {
            printf("%s  ", dptr->d_name);
        }

        printf("\n");
    } else {
        if (strcmp(tokens[tokenCount], "-m") == 0 ) {
            char cwd[1024];
            char *curr_dir = NULL;
            DIR *dp = NULL;
            struct dirent *dptr = NULL;
            unsigned int count = 0;

            curr_dir = getcwd(cwd, sizeof(cwd));
            if (NULL == curr_dir) {
                perror("\n ERROR : Could not get the working directory\n");
            }

            dp = opendir((const char*)curr_dir);
            if (NULL == dp) {
                perror("\n ERROR : Could not open the working directory\n");
            }

            printf("\n");
            for (count = 0; NULL != (dptr = readdir(dp)); count++) {
                printf("%s , ", dptr->d_name);
            }
            printf("\n");
        } else {
            if (strcmp(tokens[tokenCount], "-l") == 0) {
                int count, i;
                struct direct **files;
                struct stat statbuf;
                char datestring[256];
                struct passwd pwent;
                struct passwd *pwentp;
                struct group grp;
                struct group *grpt;
                struct tm time;

                char buf[1024];

                if (!getcwd(pathname, sizeof(pathname))) {
                    perror("Error");

                }

                count = scandir(pathname, &files, one, alphasort);
                if (count > 0) {
                    printf("total %d\n", count);

                    for (i = 0; i < count; ++i) {

                        if (stat(files[i]->d_name, &statbuf) == 0) {
                            printf("%10.10s", get_perms(statbuf.st_mode));
                            if (!getpwuid_r(statbuf.st_uid, &pwent, buf, sizeof(buf), &pwentp))
                                printf(" %s", pwent.pw_name);
                            else
                                printf(" %d", statbuf.st_uid);

                            if (!getgrgid_r (statbuf.st_gid, &grp, buf, sizeof(buf), &grpt))
                                printf(" %s", grp.gr_name);
                            else
                                printf(" %d", statbuf.st_gid);
                            printf(" %5d", (int)statbuf.st_size);
                            localtime_r(&statbuf.st_mtime, &time);
                            strftime(datestring, sizeof(datestring), "%F %T", &time);
                            printf(" %s %s\n", datestring, files[i]->d_name);
                        }

                        free (files[i]);
                    }

                    free(files);
                }
            } else {
                if (strcmp(tokens[tokenCount], "-t") == 0) {
                    int count, i;
                    struct direct **files;
                    struct direct *temp;
                    struct stat statbuf;
                    char datestring[256];
                    struct passwd pwent;
                    struct passwd *pwentp;
                    struct group grp;
                    struct group *grpt;
                    struct tm time;

                    char buf[1024];

                    if (!getcwd(pathname, sizeof(pathname))) {
                        perror("Error");

                    }

                    count = scandir(pathname, &files, one, alphasort);
                    if (count > 0) {
                        struct stat statbuf1;
                        struct stat statbuf2;
                        for (i = 0; i < count; i++) {
                            for (int j = 0; j < count - i; j++) {
                                if (stat(files[j]->d_name, &statbuf1) == 0 && stat(files[j + 1]->d_name, &statbuf2) == 0) {
                                    if (statbuf1.st_mtime < statbuf2.st_mtime) {
                                        direct *temp = files[j];
                                        files[j] = files[j + 1];
                                        files[j + 1] = temp;
                                    }
                                }

                            }
                        }
                        printf("total %d\n", count);

                        for (i = 0; i < count; ++i) {

                            if (stat(files[i]->d_name, &statbuf) == 0) {


                                printf("%10.10s", get_perms(statbuf.st_mode));
                                if (!getpwuid_r(statbuf.st_uid, &pwent, buf, sizeof(buf), &pwentp))
                                    printf(" %s", pwent.pw_name);
                                else
                                    printf(" %d", statbuf.st_uid);

                                if (!getgrgid_r (statbuf.st_gid, &grp, buf, sizeof(buf), &grpt))
                                    printf(" %s", grp.gr_name);
                                else
                                    printf(" %d", statbuf.st_gid);
                                printf(" %5d", (int)statbuf.st_size);
                                localtime_r(&statbuf.st_mtime, &time);
                                strftime(datestring, sizeof(datestring), "%F %T", &time);
                                printf(" %s %s\n", datestring, files[i]->d_name);
                            }

                            free (files[i]);
                        }

                        free(files);
                    }

                }
            }
        }
    }

    return 1;
}
int shell_cd(char** tokens) {
    if (tokens[1] == NULL) {
        cout << "mi_sh: \"cd\" missing argument." << endl;
    } else {

        if (chdir(tokens[1]) != 0) {
            perror("mi_sh");
        }
    }
    return 1;
}
int shell_mkdir(char** tokens) {

    if (tokens[1] == NULL) {
        cout << "mi_sh: \"mkdir\" missing argument" << endl;
    } else {
        if (mkdir(tokens[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            perror("mi_sh");
        }
    }
    return 1;
}
int shell_chmod(char** tokens) {
    if (tokens[1] == NULL) {
        cout << "mi_sh: argumento esperado para \"chmod\"" << endl;
    } else {
        if (tokens[2] == NULL) {
            cout << "mi_sh: argumento esperado para \"chmod\"" << endl;
        } else {
            int mode = strtol(tokens[1], 0, 8);

            if (chmod(tokens[2], mode) != 0) {
                perror("mi_sh");
            }
        }
    }
    return 1;

}
int shell_rmdir(char** tokens) {

    if (tokens[1] == NULL) {
        cout << "mi_sh: \"rmdir\" missing argument.\n" << endl;
    } else {
        if (rmdir(tokens[1]) != 0) {
            perror("mi_sh");
        }
    }
    return 1;

}
int shell_rm(char** tokens) {
    int tokenCount = 0;
    while (tokens[tokenCount] != 0 ) {
        tokenCount++;
    }
    tokenCount--;
    if (tokenCount == 0) {
        perror("\"rm\" missing argument.\n");
    } else {
        if (tokenCount == 1) {
            if (remove(tokens[tokenCount]) != 0) {
                perror("mi_sh");
            }

        } else {
            if (strcmp(tokens[1], "-R") == 0) {
                char* directory = tokens[tokenCount];
                if (!directory) perror("rm -R missing argument. \n");
                else {
                    int ret;
                    char *path_dir = (char *) malloc((500 * sizeof(char *)));
                    strcpy(path_dir, directory);
                    ret = removedir(path_dir);
                }
            }
        }
    }
    return 1;
}
int shell_cat(char** tokens) {
    if (tokens[1] == NULL) {
        perror("\"cat\" missing argument.\n");

    } else {
        if (strcmp(tokens[1], ">") == 0) {
            if (tokens[2]== NULL) {
                perror("\"cat\" missing argument.\n");
            } else {
                char *line;
                size_t buffer_size;
                line = NULL;
                buffer_size = 0;
                string str;
                
                ofstream file(tokens[2]);
                do {
                    getline(cin, str);
                    file  << str << "\n";
                }while(!cin.eof());
               
                

            }
        } else {
            int controlador = 0;
            int tokenCount = 1;
            int i = 0;

            do {
                if (controlador == 0 || tokens[tokenCount][i] == 0) {
                    FILE *file;
                    char line[100];
                    file = fopen(tokens[tokenCount], "r");
                    while (fscanf(file, "%[^\n]\n", line) != EOF) {

                        printf("%s\n", line);
                    }
                    fclose(file);
                    tokenCount++;
                    i = 0;
                }

                i++;
                controlador++;
            } while (tokens[tokenCount] != NULL);
        }

    }
    return 1;
}
int shell_ln(char** tokens) {
    int tokenCount = 0;
    while (tokens[tokenCount] != NULL) {
        tokenCount++;
    }
    tokenCount--;
    if (tokenCount == 0) {
        perror("\"ln\" missing argument. \n");
    } else {
        if (strcmp(tokens[1], "-s") == 0) {
            char* fileTolink = tokens[2];
            int i;
            struct stat s;
            int argc;
            if (!fileTolink) perror("ln too few arguments \n");
            else {
                char* newLink = tokens[tokenCount];
                if (!newLink) perror("ln too few arguments \n");
                else {
                    if (link(fileTolink, newLink) < 0) {
                        perror("ERROR:Unable to create the Link");

                    }

                }
            }

        }
    }
    return 1;
}
int shell_ps(char** tokens) {
    int tokenCount = 0;
    while (tokens[tokenCount] != NULL) {
        tokenCount++;
    }
    tokenCount--;
    if (tokenCount == 0) {



    } else {
        pidaux();
    }
    return 1;
}
int shell_uname(char** tokens) {
    int tokenCount = 0;
    while (tokens[tokenCount] != NULL) {
        tokenCount++;
    }
    tokenCount--;
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        perror("uname");

    }

    if (tokenCount == 0) {
        printf("%s \n", buffer.sysname);
    } else {
        if (strcmp(tokens[tokenCount], "-s") == 0) {
            printf("%s \n", buffer.sysname);
        }
        if (strcmp(tokens[tokenCount], "-r") == 0) {
            printf("%s \n", buffer.release);
        }
        if (strcmp(tokens[tokenCount], "-m") == 0) {
            printf("%s \n", buffer.machine);
        }
        if (strcmp(tokens[tokenCount], "-v") == 0) {
            printf("%s \n", buffer.version);
        }
        if (strcmp(tokens[tokenCount], "-n") == 0) {
            printf("%s \n", buffer.nodename);
        }
        if (strcmp(tokens[tokenCount], "-a") == 0) {
            printf("%s ", buffer.sysname);
            printf("%s ", buffer.nodename);
            printf("%s ", buffer.release);
            printf("%s ", buffer.version);
            printf("%s ", buffer.machine);
            printf("\n");
        }
    }
    return 1;
}
int shell_kill(char** tokens) {
    int tokenCount = 0 ;

    while (tokens[tokenCount] != NULL) {
        tokenCount++;
    }
    tokenCount--;
    if (tokenCount != 2) {
        perror("\"kill\" missing arguments\n");
    } else {

        kill(atoi(tokens[tokenCount]), 9);
    }
    return 1;
}
int shell_help(char** tokens) {
    int i;
    cout << "Proyecto 2, Sistemas Operativos I" << endl;
    cout << "Grupo: Fernando Reyes, Kathia Barahona" << endl;
    cout << "Puede utilizar los siguientes comandos: " << endl;
    for (i = 0 ; i < shell_num_builtins(); i++) {
        cout << builtin_str[i] << endl;
    }
    return 1;

}
int shell_exit(char** tokens) {
    return 0 ;
}
int shell_execute_pipe(char** tokens1, char** tokens2) {
    int fd[2];
    pipe(fd);
    pid_t childpid = fork();

    if (childpid == -1) {
        perror("mi_sh");
        exit(EXIT_FAILURE);
    }
    if (childpid == 0) {
        close(fd[1]);
        dup(fd[0]);
        shell_execute(tokens1);
    } else {
        close(fd[0]);
        dup(fd[1]);
        shell_execute(tokens2);

    }
    return 1;
}
