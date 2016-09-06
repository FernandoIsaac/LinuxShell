#include<stdio.h>

void main(int argc , char *argv[])
{
                FILE *file;
               char line[100];
            
               // remember file should exist
      
               file = fopen(argv[1],"r");
        
                printf("output of cat command\n");
            
               // reading file line by line entering to buffer line
               while(fscanf(file,"%[^\n]\n",line)!=EOF)
               {

                              printf("%s\n", line);
               }
               // file close
               fclose(file);
}
