//Victor Abraham, Aman Khoja, SE 4348 Project 1 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "util.h"
#define LIMIT 256 
#define MAXLINE 1024

//Method used to welcome the user on the shell
void welcomeMessage(){
		printf("\t\t\t __________________________________________________ \n");
		printf("\t\t\t|  _____________________________________________   |\n");
    	printf("\t\t\t|  |  ____   __    __   ______   __      __     |  |\n");
	    printf("\t\t\t|  | |   _| |  |  |  | |    __| |  |    |  |    |  |\n");
	    printf("\t\t\t|  | |  |   |  |  |  | |   |    |  |    |  |    |  |\n");
	    printf("\t\t\t|  | |  |_  |  |__|  | |   |__  |  |    |  |    |  |\n");
    	printf("\t\t\t|  | |__  | |   __   | |    __| |  |    |  |    |  |\n");
		printf("\t\t\t|  |  __| | |  |  |  | |   |    |  |__  |  |__  |  |\n");
		printf("\t\t\t|  | |    | |  |  |  | |   |__  |     | |     | |  |\n");
		printf("\t\t\t|  | |____| |__|  |__| |______| |_____| |_____| |  |\n");
		printf("\t\t\t|  |             -- POWERED BY C --             |  |\n");
		printf("\t\t\t|  |        Victor Abraham & Aman Khoja         |  |\n");
		printf("\t\t\t|  |____________________________________________|  |\n");
		printf("\t\t\t|__________________________________________________|\n");
}

//login to the host and get the location for the shell prompt
void promptLine(){
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	printf(" _____________________________________________________________________________________________________________________________________________________________________________\n");
	printf("<< %s@%s:~$ %s >> ", getenv("LOGNAME"), hostn, getcwd(currentDir, 1024));
}//end method

//start the shell process and intialize
void startShell(){
        G_PID = getpid();
        G_INTERACTIVE = isatty(STDIN_FILENO);  

		if (G_INTERACTIVE) {
			while (tcgetpgrp(STDIN_FILENO) != (G_PGID = getpgrp()))
				kill(G_PID, SIGTTIN);             		
			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);
			
			setpgid(G_PID, G_PID); 
			G_PGID = getpgrp();
			if (G_PID != G_PGID) {
					printf("Error, the shell is not process group leader");
					exit(EXIT_FAILURE);
			}
			tcsetpgrp(STDIN_FILENO, G_PGID);  
			tcgetattr(STDIN_FILENO, &G_TMODES);
			currentDir = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}//end method

//changing directory using the cd command, if no directory found it will display error message.
int changeDir(char* args[]){
	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}
	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s: NO SUCH DIRECTORY\n", args[1]);
            return -1;
		}
	}
	return 0;
}

//wait for dead process
void signal_child(int p){
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
	printf("\n");
}//end method

//we send a SIGTERM signal to the child process
void signal_int(int p){
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		promptNum = 1;			
	}else{
		printf("\n");
	}
} //end method

//method to manage the environment varaibles
int environmentVAR(char * args[], int option){
	char **env_aux;
	switch(option){
        //this case will print the environment variables along with values
		case 0: 
			for(env_aux = environ; *env_aux != 0; env_aux ++){
				printf("%s\n", *env_aux);
			}
			break;
        //this case will set a value to the environment variables
		case 1: 
			if((args[1] == NULL) && args[2] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}            
            //outputs according to the result if the variable existed before or not
			if(getenv(args[1]) != NULL){
				printf("%s", "The variable has been overwritten\n");
			}else{
				printf("%s", "The variable has been created\n");
			}

			if (args[2] == NULL){
				setenv(args[1], "", 1);

			}else{
				setenv(args[1], args[2], 1);
			}
			break;

		case 2:
			if(args[1] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			if(getenv(args[1]) != NULL){
				unsetenv(args[1]);
				printf("%s", "The variable has been erased\n");
			}else{
				printf("%s", "The variable does not exist\n");
			}
		break;		
			
	}
	return 0;
}

//start a program which can run in background or foreground
void startProgram(char **args, int background){	 
	 int error = -1;
	 
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
    //child process
	if(pid==0){

		signal(SIGINT, SIG_IGN);
		
        // We set parent=<pathname>/myshell as an environment variable for the child
		setenv("parent",getcwd(currentDir, 1024),1);	
		
		if (execvp(args[0],args)==error){
			printf("Command not found");
			kill(getpid(),SIGTERM);
		}
	 }
	 
     // if background value is 0 then it is not requested to be in the background, wait for child to finish
	 if (background == 0){
		 waitpid(pid,NULL,0);
	 }else{

		 printf("Process created with PID: %d\n",pid);
	 }	 
}
 
//method to manage input output direction
void fileIO(char * args[], char* inputFile, char* outputFile, int option){
	int error = -1;
	int fileDescriptor;
	
	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0){
        //if the output involes redirection
		if (option == 0){	
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 		
			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);
		//if the input and output involves redirection
		}else if (option == 1){		
			fileDescriptor = open(inputFile, O_RDONLY, 0600); 			
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);	
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);		 
		}
		 
		setenv("parent",getcwd(currentDir, 1024),1);		
		if (execvp(args[0],args)==error){
			printf("error");
			kill(getpid(),SIGTERM);
		}		 
	}
	waitpid(pid,NULL,0);
}

//method to handle pipe commands in our shell
void pipeCommand(char * args[]){
	int filedes[2]; 
	int filedes2[2];
	int num_cmds = 0;
	char *command[256];
	pid_t pid;
	int error = -1;
	int end = 0;
	
	//loop varaibles
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	
    // calculate total number of pipe commands count
	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;
	

	while (args[j] != NULL && end != 1){
		k = 0;
		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				end = 1;
				k++;
				break;
			}
			k++;
		}
		command[k] = NULL;
		j++;		
		if (i % 2 != 0){
			pipe(filedes); 
		}else{
			pipe(filedes2); 
		}		
		pid=fork();		
		if(pid==-1){			
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]); 
				}else{
					close(filedes2[1]);
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){
			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}

			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){ 
					dup2(filedes[0],STDIN_FILENO);
				}else{ 
					dup2(filedes2[0],STDIN_FILENO);
				}

			}else{ 
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==error){
				kill(getpid(),SIGTERM);
			}		
		}
				
		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){					
				close(filedes[0]);
			}else{					
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){					
				close(filedes2[0]);
				close(filedes[1]);
			}else{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}				
		waitpid(pid,NULL,0);	
		i++;	
	}
}

//method use my shell to handle standard input by command line			
int commandHandler(char * args[]){
	int i = 0;
	int j = 0;	
	int fileDescriptor;
	int standardOut;
	int aux;
	int background = 0;
	char *args_aux[256];
	
    //special characters
	while ( args[j] != NULL){
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
			break;
		}
		args_aux[j] = args[j];
		j++;
	}
	//'exit' exit the shell
	if(strcmp(args[0],"exit") == 0) exit(0);
    
    //'pwd' prints current working directory
 	else if (strcmp(args[0],"pwd") == 0){
		if (args[j] != NULL){
				if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
				fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 	
				standardOut = dup(STDOUT_FILENO); 														
				dup2(fileDescriptor, STDOUT_FILENO); 
				close(fileDescriptor);
				printf("%s\n", getcwd(currentDir, 1024));
				dup2(standardOut, STDOUT_FILENO);
			}
		}else{
			printf("%s\n", getcwd(currentDir, 1024));
		}
	} 
    //'clear' clears the current screen
	else if (strcmp(args[0],"clear") == 0) system("clear");
    // 'cd' change directory with arguments
	else if (strcmp(args[0],"cd") == 0) changeDir(args);
	//'environ' display environmental variables
	else if (strcmp(args[0],"environ") == 0){
		if (args[j] != NULL){
			
			if ( (strcmp(args[j],">") == 0) && (args[j+1] != NULL) ){
				fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600); 				
				standardOut = dup(STDOUT_FILENO); 														
				dup2(fileDescriptor, STDOUT_FILENO); 
				close(fileDescriptor);
				environmentVAR(args,0);
				dup2(standardOut, STDOUT_FILENO);
			}
		}else{
			environmentVAR(args,0);
		}
	}
    //'setenv' use to set envinronmental variables
	else if (strcmp(args[0],"setenv") == 0) environmentVAR(args,1);
    //'unsetenv' undefine environmental varaibles
	else if (strcmp(args[0],"unsetenv") == 0) environmentVAR(args,2);
	else{
		while (args[i] != NULL && background == 0){
			if (strcmp(args[i],"&") == 0){
				background = 1;
			}else if (strcmp(args[i],"|") == 0){
				pipeCommand(args);
				return 1;
			}else if (strcmp(args[i],"<") == 0){
				aux = i+1;
				if (args[aux] == NULL || args[aux+1] == NULL || args[aux+2] == NULL ){
					printf("Not enough input arguments\n");
					return -1;
				}else{
					if (strcmp(args[aux+1],">") != 0){
						printf("Usage: Expected '>' and found %s\n",args[aux+1]);
						return -2;
					}
				}
				fileIO(args_aux,args[i+1],args[i+3],1);
				return 1;
			}

			else if (strcmp(args[i],">") == 0){
				if (args[i+1] == NULL){
					printf("Not enough input arguments\n");
					return -1;
				}
				fileIO(args_aux,NULL,args[i+1],0);
				return 1;
			}
			i++;
		}
		args_aux[i] = NULL;
		startProgram(args_aux,background);	
	}
return 1;
}

//main method for our shell program
int main(int argc, char *argv[], char ** envp) {
	char line[MAXLINE]; 
	char * tokens[LIMIT]; 
	int numTokens;
	promptNum = 0; 					
	pid = -10; 
	startShell();
	welcomeMessage();
	environ = envp;
	setenv("shell",getcwd(currentDir, 1024),1);
	while(TRUE){
		if (promptNum == 0) promptLine();
		promptNum = 0;
		memset ( line, '\0', MAXLINE );
		fgets(line, MAXLINE, stdin);
		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;	
		numTokens = 1;
        //main loop to read commands 
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		commandHandler(tokens);
	}          
	exit(0);
}


