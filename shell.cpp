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
#define token_buffer_size 64
#define token_delim " \t\r\n\a"

using namespace std;
void shell_loop();
char ** shell_split_pipeParams(char *);
char **shell_split_line(char*);
int shell_execute_pipe(char **,char**);
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
int shell_kill(char**);
int shell_help(char**);
int shell_exit(char**);

char const *builtin_str[] = {"ls","cd","mkdir","chmod","rmdir","rm","cat","ln","ps","help","exit"};
int (*builtin_func[])(char**) = {
    &shell_ls,
    &shell_cd,
    &shell_mkdir,
    &shell_chmod,
    &shell_rmdir,
    &shell_rm,
    &shell_cat,
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
void shell_loop(){
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
        for(int i = 0; i < num_params; i = i+2){
            if(i+1 == num_params){
                tokens1 = shell_split_line(params[i]);
                status = shell_execute(tokens1);
            }else{

                tokens1 = shell_split_line(params[i]);
                tokens2 = shell_split_line(params[i+1]);
                
                status = shell_execute_pipe(tokens1,tokens2);
                
            }
           
            
        }
        //tokens = shell_split_line(line);
       
       // status = shell_execute(tokens);

        free(line);
        free(params);
    }while(status);
}  
char ** shell_split_pipeParams(char* line){
    int buffer_size = token_buffer_size;
    int p_position = 0;
    char **params = (char **) malloc(buffer_size * sizeof(char *));
    char *param;
    if(!params){
        cout << "Error de asignacion de memoria" << endl;
        exit(EXIT_FAILURE);
    } 
    param = strtok(line, "|");
    while(param != NULL){
        
        params[p_position] = param;
        p_position++;
        if(p_position >= buffer_size){
            buffer_size += token_buffer_size;
            params = (char **) realloc(NULL, buffer_size * sizeof(char *));
            if(!params){
                cout << "Error de asignacion de memoria" << endl;
                exit(EXIT_FAILURE);
            }
        }
        param = strtok(NULL,"|");
    }
    params[p_position] = NULL;
    num_params = p_position;
    return params;
}
char ** shell_split_line(char* line){
    int buffer_size = token_buffer_size;
    int t_position = 0;
    char **tokens = (char **) malloc(buffer_size * sizeof(char *));
    char *token;
    if(!tokens){
        cout << "Error de asignacion de memoria" << endl;
        exit(EXIT_FAILURE);
    }
    token = strtok(line,token_delim);
    while(token != NULL){
        
        tokens[t_position] = token;
        t_position++;
        if(t_position >= buffer_size){
            buffer_size += token_buffer_size;
            tokens = (char **) realloc(NULL,buffer_size * sizeof(char *));
            if(!tokens){
                cout << "Error de asignacion de memoria" << endl;
                exit(EXIT_FAILURE);
            }

        }
        token = strtok(NULL,token_delim);
    }
    tokens[t_position] = NULL;
    return tokens;
}
int shell_launch(char** tokens){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if(pid == 0){
        if(execvp(tokens[0],tokens) == -1){
            perror("mi_sh");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0){
        perror("mi_sh");
    }else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
int shell_execute(char** tokens){
    if(tokens[0] == NULL){
        return 1;
    }
    for (int i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(tokens[0], builtin_str[i]) == 0) {
          return (*builtin_func[i])(tokens);
        }
    }

  return shell_launch(tokens);
}
int shell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}
int shell_ls(char** tokens){


    char cwd[1024];
    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;
   if (getcwd(cwd, sizeof(cwd)) != NULL){
       fprintf(stdout, "Current working dir: %s\n", cwd);
       mydir = opendir(cwd);
        while((myfile = readdir(mydir)) != NULL)
        {
            
            cout <<  myfile->d_name << endl;  
            
        }
        closedir(mydir);

   }else
       perror("mi_sh");

    return 1;
}
int shell_cd(char** tokens){
    if(tokens[1] == NULL){
        cout << "mi_sh: argumento esperado para \"cd\"" << endl;
    }else{
        if(chdir(tokens[1]) != 0){
            perror("mi_sh");
        }
    }
    return 1;
}   
int shell_mkdir(char** tokens){

    if(tokens[1] == NULL){
        cout << "mi_sh: argumento esperado para \"mkdir\"" << endl;
    }else{
        if(mkdir(tokens[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
            perror("mi_sh");
        }
    }
    return 1;
}
int shell_chmod(char** tokens){
    if(tokens[1] == NULL){
        cout << "mi_sh: argumento esperado para \"chmod\"" << endl;
    }else{
        if(tokens[2] == NULL){
            cout << "mi_sh: argumento esperado para \"chmod\"" << endl;
        }else{
            int mode = strtol(tokens[1],0,8);

            if(chmod(tokens[2],mode) != 0){
                perror("mi_sh");
            }
        }
    }
    return 1;

}
int shell_rmdir(char** tokens){
    
    if(tokens[1] == NULL){
        cout << "mi_sh: argumento esperado para \"rmdir\"" << endl;
    }else{
        if(rmdir(tokens[1]) != 0){
            perror("mi_sh");
        }
    }
    return 1;

}
int shell_rm(char** tokens){
    // if(tokens[1] == NULL){
    //     cout << "mi_sh: argumento esperado para \"rm\"" << endl;
    // }else{
    //     if(rm(tokens[1]) != 0){
    //         perror("mi_sh");
    //     }
    // }
    return 1;
}
int shell_cat(char** tokens){

}
int shell_ln(char** tokens){

}
int shell_ps(char** tokens){

}
int shell_help(char** tokens){
    int i;
    cout << "Proyecto 2, Sistemas Operativos I" << endl;
    cout << "Grupo: Fernando Reyes, Kathia Barahona" << endl;
    cout << "Puede utilizar los siguientes comandos: " <<endl;
    for(i = 0 ; i < shell_num_builtins(); i++){
        cout << builtin_str[i] << endl;
    }
    return 1;

}
int shell_exit(char** tokens){
    return 0 ;
}
int shell_execute_pipe(char** tokens1, char** tokens2){
    int fd[2];
    pipe(fd);
    pid_t childpid = fork();

    if(childpid == -1){
        perror("mi_sh");
        exit(EXIT_FAILURE);
    }
    if(childpid == 0){
        close(fd[1]);
        dup(fd[0]);
        shell_execute(tokens1);
    }else{
        close(fd[0]);
        dup(fd[1]);
        shell_execute(tokens2);

    }
    return 1;
}