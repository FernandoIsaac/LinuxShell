#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/procfs.h>
#include <ctype.h>
#include <libgen.h>
#include <pwd.h>
#include <sys/utsname.h>
#ifdef _WIN32
#include <windows.h>
#define chdir _chdir

#else
#include <unistd.h>
#endif

#define MAX_LENGTH 1024
#define DELIMS " \t\r\n"

int check_if_number (char *str)
{
  int i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (!isdigit (str[i]))
    {
      return 0;
    }
  }
  return 1;
}
 
#define MAX_BUF 1024
#define INT_SIZE_BUF 6
#define PID_LIST_BLOCK 32
#define UP_TIME_SIZE 10
const char *getUserName(int uid)
{
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}
 
void pidaux ()
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  char path[MAX_BUF], read_buf[MAX_BUF],temp_buf[MAX_BUF];
  char uid_int_str[INT_SIZE_BUF]={0},*line;
  char uptime_str[UP_TIME_SIZE];
  char *user,*command;
  size_t len=0;
  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
         perror ("Fail");
         exit(0);
  } 
  strcpy(path,"/proc/");
  strcat(path,"uptime");
 
  fp=fopen(path,"r");
  if(fp!=NULL)
  {
	getline(&line,&len,fp);
	sscanf(line,"%s ",uptime_str);
  }
  long uptime=atof(uptime_str);
  long Hertz=sysconf(_SC_CLK_TCK); 
  strcpy(path,"/proc/");
  strcat(path,"meminfo");

  fp=fopen(path,"r");
  unsigned long long total_memory;
  if(fp!=NULL)
  {
	getline(&line,&len,fp);
	sscanf(line,"MemTotal:        %llu kB",&total_memory);
  }	

  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/status");
	unsigned long long memory_rss;
	fp=fopen(path,"r");
	unsigned long long vmsize;

	if(fp!=NULL)
	{
		vmsize=0;
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		sscanf(line,"Uid:    %s ",uid_int_str);
		getline(&line,&len,fp);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
		getline(&line,&len,fp);
                getline(&line,&len,fp);
		sscanf(line,"VmSize:    %llu kB",&vmsize);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
		getline(&line,&len,fp);
		sscanf(line,"VmRSS:     %llu kB",&memory_rss);
		
	}
	else
	{
		fprintf(stdout,"FP is NULL\n");
	}
	float memory_usage=100*memory_rss/total_memory;
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/stat");
	fp=fopen(path,"r");
	getline(&line,&len,fp);
	char comm[10],state;
	unsigned int flags;
	int pid,ppid,pgrp,session,tty_nr,tpgid;
	unsigned long minflt,cminflt,majflt,cmajflt,utime,stime;
	unsigned long long starttime;
	long cutime,cstime,priority,nice,num_threads,itreavalue;
	sscanf(line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu",&pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime,&cutime,&cstime,&priority,&nice,&num_threads,&itreavalue,&starttime);
	unsigned long total_time=utime+stime;
	total_time=total_time+(unsigned long)cutime+(unsigned long)cstime;
	float seconds=uptime-(starttime/Hertz);
	float cpu_usage=100*((total_time/Hertz)/seconds);
	if(isnan(cpu_usage))
	{
		cpu_usage=0.0;
	}
	if(isnan(memory_usage))
	{
		memory_usage=0.0;
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
     	if(strlen(userName)<9)
      	{
		user=userName;	
      	}
      	else
      	{
		user=uid_int_str;
      	}
      	fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
     	 
    }
  } 
  closedir (dirp);
}

int removedir(char path[500]){
        int counter =1;
        DIR *pdir = NULL;
        struct dirent *pent = NULL;
        struct stat eStat;
        pdir = opendir(path);
        char x[500];
        if ( pdir == NULL){
                return -1;
        }
        while ( (pent=readdir(pdir)) != NULL ){
                if((strcmp((pent->d_name),".")==0)||(strcmp((pent->d_name),"..")==0)){
                        continue;
                }
                else{
                        strcpy(x,path);
                        path=strcat(path,"/");
                        path=strcat(path,pent->d_name);

                        if(stat(path, &eStat)){
                                printf("ERROR: %s... Meaning it can be a file(Most certainly)\n", strerror(errno));
                        }
                        else{
                                if(S_ISDIR(eStat.st_mode)){
                                        removedir(path);
                                        strcpy(path,x);
                                }
                                else{
                                        unlink(path);
                                        strcpy(path,x);
                                }
                        }
                }
        }
        if (!rmdir(path)) return -1; // delete the directory */
}


static char perms_buff[30];

const char *get_perms(mode_t mode){
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

char pathname[MAXPATHLEN];

void die(char *msg){
  perror(msg);
  exit(0);
}

static int
one (const struct dirent *unused){
  return 1;
}


void shell_loop();
int main(int argc, char *argv[]) {
  shell_loop();
  return EXIT_SUCCESS;
}

void shell_loop(){
	
  char *cmd;
  char line[MAX_LENGTH];

  while (1) {
    printf("mi_sh> ");
    if (!fgets(line, MAX_LENGTH, stdin)) break;

    // Parse and execute command
    if ((cmd = strtok(line, DELIMS))) {
      // Clear errors
      errno = 0;

      if (strcmp(cmd, "cd") == 0) {
        char *arg = strtok(0, DELIMS);

        if (!arg) fprintf(stderr, "cd missing argument.\n");
          else chdir(arg);

      } else if (strcmp(cmd, "exit") == 0) {
          break;

      } else if(strcmp(cmd, "clear") ==0){
	 system("clear"); 
      } else if (strcmp(cmd, "mkdir") == 0){
	  char* arg = strtok(0, DELIMS);
	  if (!arg) fprintf(stderr, "mkdir missing argument.\n");
	  else mkdir(arg, 0700);
      } else if (strcmp(cmd, "ls")== 0){
	  char* arg = strtok(0, DELIMS);
	  if(!arg){
		char *curr_dir = NULL;
    		DIR *dp = NULL;
    		struct dirent *dptr = NULL;
    		unsigned int count = 0;

    		curr_dir = getenv("PWD");
    		if(NULL == curr_dir){
        		printf("\n ERROR : Could not get the working directory\n");
       			 break;
    		}

    		dp = opendir((const char*)curr_dir);
    		if(NULL == dp){
        		printf("\n ERROR : Could not open the working directory\n");
        		 break;
    		}

    		printf("\n");
    		for(count = 0; NULL != (dptr = readdir(dp)); count++){
        		printf("%s  ",dptr->d_name);
    		}
    		//printf("\n %u ", count);
		printf("\n");
	
	  }else if(strcmp(arg, "-m")==0){
		char *curr_dir = NULL;
    		DIR *dp = NULL;
    		struct dirent *dptr = NULL;
    		unsigned int count = 0;

    		curr_dir = getenv("PWD");
    		if(NULL == curr_dir){
        		printf("\n ERROR : Could not get the working directory\n");
       			 break;
    		}

    		dp = opendir((const char*)curr_dir);
    		if(NULL == dp){
        		printf("\n ERROR : Could not open the working directory\n");
        		 break;
    		}

    		printf("\n");
    		for(count = 0; NULL != (dptr = readdir(dp)); count++){
        		printf("%s , ",dptr->d_name);
    		}
    		//printf("\n %u ", count);
		printf("\n");
	
	
	  }else if(strcmp(arg, "-l")==0){
		int count,i;
  		struct direct **files;
  		struct stat statbuf;
  		char datestring[256];
  		struct passwd pwent;
  		struct passwd *pwentp;
  		struct group grp;
  		struct group *grpt;
  		struct tm time;
  		char buf[1024];

  		if(!getcwd(pathname, sizeof(pathname))){
			printf("Error");
			break;
		}
    			//die("Error getting pathnamen");

  		count = scandir(pathname, &files, one, alphasort);
		if(count > 0){
    			printf("total %d\n",count);

    			for (i=0; i<count; ++i){
      				if (stat(files[i]->d_name, &statbuf) == 0){
        				printf("%10.10s", get_perms(statbuf.st_mode));
        				//printf(" %d", statbuf.st_nlink);

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
	 else if (strcmp(cmd, "rmdir") == 0){
	  char* arg = strtok(0, DELIMS);
	  if(!arg) fprintf(stderr, "rmdir missing argument.\n");
	  else if(strcmp(arg, "-R")==0){
		char* directory = strtok(0, DELIMS);
		if(!directory) fprintf(stderr, "rm -R missing argument. \n");
		else{
			int ret;
			char *path_dir=malloc(500);
			//char *path_dir=directory;
			//char path[500];
			//printf("enter Directory name\n");
			//scanf("%s",path);
			strcpy(path_dir,directory);
			ret=removedir(path_dir);
			//printf("\n %d", ret);
		}	
	  }
	  else rmdir(arg);
      }
	else if(strcmp(cmd, "rm") == 0){
	  char* arg = strtok(0, DELIMS);
	  if(!arg) fprintf(stderr, "rm missing argument.\n");
	  else remove(arg);
	}
	else if(strcmp(cmd, "ps")==0){
	  char* arg = strtok(0, DELIMS);
	  if(!arg){
		
          }
	  else if (strcmp(arg, "-e")==0){
		  pidaux();
	  }
	}
	else if (strcmp(cmd, "cat") == 0){
	  char* arg = strtok(0, DELIMS);
	  if(!arg) fprintf(stderr, "cat missing argument.\n");
	  else{
	       int controlador = 0;
	       int i = 0;
	       printf("%s \n",arg);
	       do{
			if(controlador == 0 || arg[i] == ' '){
	      			FILE *file;
               			char line[100];
               			file = fopen(arg,"r");
               			while(fscanf(file,"%[^\n]\n",line)!=EOF){
                        		printf("%s\n", line);
               			}
               			fclose(file);
	       			arg = strtok(NULL, DELIMS);
			}
			printf("%d \n", controlador);
			i++;
			controlador++;
	       }while(arg != NULL);
	} 
      }
     else if (strcmp(cmd, "ln")==0){
	char* arg = strtok(0, DELIMS);
	if(!arg) fprintf(stderr, "ln missing argument. \n");
	else if(strcmp(arg, "-s")==0){
		char* fileTolink = strtok(0, DELIMS);
		int i;
		struct stat s;
		int argc;
		if(!fileTolink) fprintf(stderr, "ln too few arguments \n");
			else{
				char* newLink = strtok(0, DELIMS);
				if(!newLink) fprintf(stderr, "ln too few arguments \n");
				else{
					if(link(fileTolink,newLink)<0){
	                        		perror("ERROR:Unable to create the Link");
        	                		break;	
                			}

				}
			}
		}
	}
     else if(strcmp(cmd, "uname")==0){
	char* arg = strtok(0, DELIMS);
	struct utsname buffer;
	errno = 0;
	if (uname(&buffer) != 0) {
	      	perror("uname");
		break;
   	}

	if(!arg||strcmp(arg, "-s")==0){
		printf("%s \n", buffer.sysname);	
	}
	else if(strcmp(arg, "-r")==0){
		printf("%s \n", buffer.release);	
	}
	else if(strcmp(arg, "-m")==0){
		printf("%s \n", buffer.machine);	
	}
	else if(strcmp(arg, "-v")==0){
		printf("%s \n", buffer.version);	
	}
	else if(strcmp(arg, "-n")==0){
		printf("%s \n", buffer.nodename);	
	}
	else if(strcmp(arg, "-a")==0){
		printf("%s ", buffer.sysname);	
		printf("%s ", buffer.nodename);	
		printf("%s ", buffer.release);	
		printf("%s ", buffer.version);	
		printf("%s ", buffer.machine);
		printf("\n");	
	}
     }
     else if (strcmp(cmd, "chmod") == 0){
        char* arg = strtok(0, DELIMS);
	if(!arg) fprintf(stderr, "chmod missing argument .\n");
	else{
		int controlador = 0;
		int i = 0;
		int mode;
		do{
			if(arg[i]==' '){
				controlador++;
				if(controlador==1){
					mode = strtol(arg, 0, 8);
				}
				if(controlador==2){
					if(chmod(arg, mode) != 0){
						perror("Command failed");
					}
				}
				arg = strtok(NULL, DELIMS);
			}
			i++;
		}while(arg != NULL);	
		if(controlador<2){
			fprintf(stderr, "chmod missing argument .\n");
		}
	}
      }else /*system(line);
      if (errno)*/ printf("Not found \n");
    }
  }
}

