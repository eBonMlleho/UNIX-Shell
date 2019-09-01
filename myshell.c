#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
void chopN(char *str, size_t n)
{
    assert(n != 0 && str != 0);
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}



int main(int argc, char **argv) {
	/* take care of requirement #1 set prompt*/
	char* prompt = "";
	//$ ./shell -p "hello> " takes 3 command options and the second string should be "-p" length is 2
	if(argc == 3){
		if(strncmp("-p", argv[1],2) != 0)
		{
			printf("Invalid command line argument.\n");
		}
		prompt = argv[2];
	}else if(argc == 1){
 		prompt = "308sh> ";
	}else{
		printf("Invalid command line arguemnt. \n");
	}

	char command[50]; //store usr input
	char cwd[100]; //current working directory
	char directory[50];
	int i = 0;
	int k=0;
	int j=0;
	int numArguments = 0;
	int countCommand;
	int child;
	int child2;
	int backgroundProcess = 0;
	char setVariable[50][50];
	char getVariable[50];
	char value[50][50];
	int status;
	int count =0;
	int match = 0; 
	int is_set = 0;
	//hint to avoid cases such as " exit",  "  ppid" don not work

	printf("Note: make sure do not enter empty space before your command and use lower case\n");
	while(1){
		printf(prompt);
		//get user input and save it to variable command
		fgets(command, 50, stdin);
		//printf("%d", strlen(command));  //used for verify input command length
		
		//exit shell if command is "exit"
		if(strncmp("exit",command, 4) == 0){
			return 0; 	//	exit from the program
		}

		//display PID if command is "pid"
		else if(strncmp("pid", command, 3) == 0){
			printf("Process ID of this shell is : %d \n", getpid());
		}

		//display parent PID if command is "ppid"
		else if(strncmp("ppid",command, 4) == 0){
			printf("PPID of this shell is : %d \n", getppid());
		}


		
		//change directory if command is "cd"
		else if(strncmp("cd", command, 2) == 0){ 
			
			if(strlen(command) == 3){ //make sure length of "cd\0" is 3
				chdir(getenv("HOME"));

			}else{			
				//modify command by replace \n with null char so that correct command length
				if (command[strlen (command) - 1] == '\n'){
        				command[strlen(command) - 1] = '\0';
				}

				for(i = 0; i < strlen(command) - 3; i++){
					directory[i] = command[i+3];
					directory[i+1] = '\0';
				}
	
				if(chdir(directory)==-1){
					printf("Cannot find directory\n");				
				}

			}
						
		}

		//display current directory if command is "pwd"
		
		else if(strncmp("pwd", command,3) == 0){	
			if(getcwd(cwd, 100)==-1){
				printf("Cannot find current working directory");		
			}
			else{printf("%s\n", cwd);}

		}


		//set <var> <value> – sets an environment variable (which is visible in all future child processes). If there is only one argument, clears the variable.

		else if(strncmp("set", command, 3) == 0){
			is_set = 0;
			//clear the variable
			if(strncmp("set\n", command, 4) == 0){
				is_set = 1;
			}
			

			if(is_set == 0 ){
				//modify command by replace \n with null char so that correct command length			
				if ((strlen(command)>0) && (command[strlen (command) - 1] == '\n')){
        				command[strlen(command) - 1] = '\0';
				}
	
				/*printf(command);
				printf("\n");
				for(k=0;k<strlen(command);k++){
					printf("%c", command[k]);			
				}*/
				
				chopN(command, 3);
				char * pch;
				pch = strtok (command ," ,.-");
				
				strcpy(setVariable[count],pch);
				pch = strtok(NULL, " ,.-");
				strcpy(value[count],pch);
				if(setenv(setVariable[count], value[count], 1)==-1){
				printf("error\n");
				}
				/*for test case
				for(j=0;j<=count;j++){
				printf("%s value is %s\n", setVariable[j],value[j]);
				}
				*/
				count++;
			}
			else{
				for(k = 0; k <= count; k++){
					for(i = 0; i < 50 ; i++){					
						setVariable[k][i] = 0;
					}
				}
	
				printf("clear environment variable successfully\n");			
			}
		}


			

		//if user input is "get" then get the given environment variable
		//segmentation fault
		
		else if(strncmp("get", command, 3) == 0){

			//modify command by replace \n with null char so that correct command length
			if ((strlen(command)>0) && (command[strlen (command) - 1] == '\n')){
        			command[strlen(command) - 1] = '\0';
			}	
			if(is_set == 1){
				printf("Failed to find environment variable\n");
			}
			else{chopN(command, 3);
				char * pch;
				pch = strtok (command ," ,.-");
				strcpy(getVariable,pch);
	
				for(k=0;k<=count;k++){
					if(strcmp(setVariable[k], getVariable)==0){
						match = 1;
					}
				}
				
				if(match != 1){
					printf("Failed to find environment variable\n");				
	
				}
				else{	
					printf("%s\n", getenv(getVariable));
				}
			}
		}



	

		//executed command, record command info and pass to system call
		
		else
		{	//modify command by replace \n with null char so that correct command length
			if ((strlen(command)>0) && (command[strlen (command) - 1] == '\n')){
        			command[strlen(command) - 1] = '\0';
			}

			countCommand = 1; //start from 0 instead of 1 this basically make sure the size of command so that we can create char array accordingly
			for(i = 0; i < strlen(command); i++){
				if (command[i] == ' '){
					countCommand++;
				}
			}
			//whether the last argument is '&' character 
			if(command[strlen(command) - 1] == '&'){
				countCommand--;
				backgroundProcess = 1;
			}

			//fill args[] using temp string for each individual argument

			char temp[countCommand][50];
			char* args[countCommand + 1];
			k = 0;
			numArguments = 0;
			//go through each char in command char array
			for(i = 0; i < strlen(command)+1; i++){
				if (command[i] == ' ' || command[i] == '\0'){
					temp[numArguments][k] = '\0';
					args[numArguments] = temp[numArguments];
					numArguments++;
					k = 0;
				}
				else if (command[i] == '&' && i == strlen(command) - 1){
					i = strlen(command) + 1;
				}
				else{
					temp[numArguments][k] = command[i];
					k++;
				}
			}
			args[numArguments] = (char*) NULL;

			//requirement 7 the shell should block for each command.
			//in this case, child process has another child process
			if(backgroundProcess == 1){
				backgroundProcess = 0;
				child2 = fork();
				if(child2 == 0){ //in the child process
					child = fork();
					if(child == 0){ //in the child's child process
						//display process ID and name 
						printf("[%d] %s\n", getpid(), args[0]);
						execvp(args[0], args);
						perror("\0");
						return 0;
					}else{ //only 1 child process
						status = -1;
						waitpid(-1, &status, 0);
						//display pid, name and stats
						printf("\n[%d] %s Exit %d\n", child, args[0], WEXITSTATUS(status));
						printf(prompt);
						return 0;
					}
				}else{
					sleep(1);
				}
			}

			//not background process. wait for child to terminate and then continue terminal 
			else{
				//create child process to run command if child = 0 then we are in the child process
				child = fork();
				if(child == 0){ //child process
					printf("[%d] %s\n", getpid(), args[0]);
					execvp(args[0], args);
					perror("\0");
					return 0;
				}
				
				else{  //parent process
					sleep(1);
					status = -1;
					waitpid(child, &status, 0);
					printf("[%d] %s Exit %d\n", child, args[0], WEXITSTATUS(status));
				}
			}
		}




	}








}
