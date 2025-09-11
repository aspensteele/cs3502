#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
	int pipe1[2];  // Parent to child
	int pipe2[2]; // Child to parent
	pid_t pid; 
	char parent_msg[] = "Hello from parent!";
	char child_msg[] = "Hello from child!";
	char buffer[100];

	//Create both pipes
	if (pipe(pipe1) == -1) {
	   printf("An error occured with opening pipe 1.\n");
	   return 1; 
	}	
	if (pipe(pipe2) == -1) {
	  printf("An error occured with opening pipe 2.\n");
	  return 1;
	}
	 pid = fork();
	
	if (pid == 0) { //in the child process
	    close(pipe1[1]);
	    close(pipe2[0]); 
	    
            read(pipe1[0], buffer, sizeof(buffer));
	    printf("Child received: %s\n", buffer);

	   //send response to parent
	   write(pipe2[1], child_msg, strlen(child_msg) + 1);

	  //clean up
	  close(pipe1[0]);
	  close(pipe2[1]);
	  exit(0);
 }

	else {
	 //parent process
	//close unused ends
	close(pipe1[0]);
	close(pipe2[1]);
	
	//send message to child
	write(pipe1[1], parent_msg, strlen(parent_msg) + 1);

	//read respomse
	read(pipe2[0], buffer, sizeof(buffer));
	printf("Parent received: %s\n", buffer);

	//clean up
	close(pipe1[1]);
	close(pipe2[0]);

	//wait for child to finish 
	wait(NULL);
	}

	
	return 0; 
} 
