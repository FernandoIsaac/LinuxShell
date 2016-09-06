#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>

//This method looks up the input command against the lookup for 
//validity purposes
bool FunctionLookUp(char *argv, char *Lookup[])
{
	bool match = false;
	for(int i = 0; i < 10; i++){
		if(*Lookup[i] == *argv){
			match = true;
			break;}
	}
	return match;
}
void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
}

bool CheckForwardSlash(char temp[], int size)
{
      bool present = false;
      for(int i = 0; i < size; i++){
          if(temp[i] == '/'){  
           present = true;
           break;
           }
      }
      return present;
}
void  execute(char **argv)
{
     pid_t  pid;
     int    status;
     
     if ((pid = fork()) < 0) {     /* fork a child process           */
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {          /* for the child process:         */
          if (execvp(*argv, argv) < 0) {     /* execute the command  */
               printf("*** ERROR: exec failed\n");
               exit(1);
          }
     }
     else {                                  /* for the parent:      */
          while (wait(&status) != pid)       /* wait for completion  */
               ;
     }
}

  
void  main(void)
{
     char  line[1024];                         // The input line                 
     char  *argv[3];                           // An array of pointers to the tokens (the tokens were parsed 
											   // from the input line)
     bool check1, check2 = false;

	 //A look-up table that contains some of the valid commands
	 char *LookUp[11] = {"emacs", "kill", "bye", "jobs", "fg", "chmod", "cd", "help", "cat", "cp"};
     
     while (1) {                              // repeating....         
          printf("tish >> ");                 // the prompt             
          gets(line);                         // read in the command line     
          printf("\n");
          parse(line, argv);                  // parse the line  
       
          if (strcmp(argv[0], "bye") == 0)    // exit if the user enters bye
               {exit(0);}  

					  //Input Validation//
					  ///////////////////

          //If the input is just a white space character, continue with the next iteration of the loop
          if (isspace(argv[0]))
           continue;    //If it is some sort of a white-space character, 
						//skip the current iteration of the while loop
	  
			
          //Call a function that checks for the validity of the input command
		  check2 = FunctionLookUp(argv, LookUp);
		  if(check2 == false){
			  fprintf("Invalid Command\n");
			  continue;
		  }

          //Test argv[0] for invalid internal commands. Check for Letters and Negative numbers.
          if(strcmp(argv[0], "kill") == 0){
           if(isaplha(argv[1]) || atoi(argv[1]) < 0){
           fprintf("Invald PID entered");
           fprintf("Enter a positive numeric number");
           continue;
           }
           }
           
                                                     
        
         int size = sizeof(argv[1]) + 1;
         char temp[size];
		 strcpy(temp,agrv[1]);
         check1 = CheckForwardSlash(temp, size);
		 //If true is returned by the CheckForwardSlash method, skip the rest of the while loop
         if(check1 == true){                
         printf("Invalid file format\n");
         printf("Avoid Forward Slashes in your file name\n");
         continue;
         }  

					  //Input Validation Ends//
        ///////////////*********************//////////////////

		//Signals to catch the Ctrl-C and Ctrl/ combination
        signal(SIGINT, SIG_IGN);       	        //The instructions said to ignore the SIGINT signal
        signal(SIGTERM, SIG_DFL);               //SIGTERM signal must be caught. 
        execute(argv);                          //Finally, execute the command
   }
}
