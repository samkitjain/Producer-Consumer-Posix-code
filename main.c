/* Name: Samkit Jain
*/

#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include "prodcon.h"
#include <sys/time.h>
#include <string.h>

main( int argc, char* argv[] ) {	
	int con_retvalue , prod1_retvalue , prod2_retvalue;
	int con_status , prod1_status , prod2_status;
	pid_t con,prod1,prod2;

	mode_t  perms;
	int     fdin, fdout;	// File descriptors
	perms = 0740;			// Permission
	 
	/* Shared memory setup and utilization code has been built with help of shared memory example given on course website :parent.c */
	int sm_id;
	struct smspace *sm_ptr;
	key_t key;   // key for accessing shared memory
	int flag ;   // sets r/w permisions etc.
	key = 1122;  // Key for sharing memory
	flag = 1023; // Set all flags
	if ((sm_id = shmget(key ,sizeof(struct smspace) , flag)) == -1){
	perror("PARENT:shmget Failed :\n");
	exit(1);
	}
	else {
   (void) fprintf(stderr, "shmget: shmget returned %d\n", sm_id);
  	}
	/* Attach shared memory to address space */
	sm_ptr = (void *)shmat(sm_id , (void *)NULL , sizeof(struct smspace));
	if(sm_ptr == (void *) -1){
	perror("PARENT:shmat failed\n");
	exit(2);
	}
	/* Initialize shared structure values */
	sm_ptr->count1 			= BUFSIZE ; 		// No. of empty spaces in buffer1 initialized to BUFSIZE
	sm_ptr->count2 			= BUFSIZE ;			// No. of empty spaces in buffer2 initialized to BUFSIZE
	sm_ptr->redprodhead 	= 0 ;
	sm_ptr->blueprodhead 	= 0 ;
	sm_ptr->redconhead 		= 0 ;
	sm_ptr->blueconhead 	= 0	;
	sm_ptr->blueproddone 	= 0	;
	sm_ptr->redproddone 	= 0	;
	sm_ptr->Totalredcount 	= 0	;
	sm_ptr->Totalbluecount 	= 0	;
	/* Mutex setup */
	pthread_mutexattr_t mutexattr ; 												// mutex1&2 attribute variable
	pthread_mutexattr_init(&mutexattr); 											// Initializing attributes for mutex1
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);  				// Making mutex1&2 sharable among all processes
	pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init( &sm_ptr->mutex1, &mutexattr); 								// Initialize mutex1
	pthread_mutex_init( &sm_ptr->mutex2, &mutexattr);				    			// Initialize mutex2
	/*Condition variable setup*/
	pthread_condattr_t conditionvariableattr; 										// condition variable attribute	
	pthread_condattr_init(&conditionvariableattr); 									// Initializing condition variable attribute
	pthread_condattr_setpshared(&conditionvariableattr, PTHREAD_PROCESS_SHARED);	// Making  condition variable sharable among all processes
	pthread_cond_init (&sm_ptr->SpaceAvailable1, &conditionvariableattr ); 			// Initialize SpaceAvailable1 condition variable
	pthread_cond_init (&sm_ptr->ItemAvailable, &conditionvariableattr ); 			// Initialize ItemAvailable condition variable
	pthread_cond_init (&sm_ptr->SpaceAvailable2, &conditionvariableattr ); 			// Initialize SpaceAvailable2  condition variable

	/* Fork child process */
	if((con = fork()) == -1){
		perror("Error in Forking Consumer :\n");
	}

	if(con == 0){
		/* CONSUMER CODE */
		char*  key_casted = "1122" ;
        execl("./prodcon" , "prodcon" , "0" , key_casted , NULL); // For Consumer code , 1st variable = 0 
	}
	else{

		if((prod1 = fork()) == -1){
			perror("Error in Forking red-producer :\n");
		}

		if(prod1 == 0){
			/*  RED PRODUCER CODE */
			char*  key_casted = "1122" ;
			execl("./prodcon" , "prodcon" , "1" , RED , key_casted , NULL); // For Producer code , 1st variable = 1 and 2nd variable = RED means it will produce red items

		}
		else{
			if((prod2 = fork()) == -1){
				perror("Error in Forking blue-producer :\n");
			}

			if(prod2 == 0){
				/* BLUE PRODUCER CODE */
				char*  key_casted = "1122" ;
				execl("./prodcon" , "prodcon" , "1" , BLUE , key_casted , NULL); // For Producer code , 1st variable = 1 and 2nd variable = BLUE means it will produce blue items
			}
			else{
				/* Parent Code */
				printf("PARENT: All childs have been successfully created !\n");
				/*Waiting for all childs to exit */
				while(con_retvalue != con || prod1_retvalue != prod1 || prod2_retvalue != prod2){
							/* blocking wait for RED producer */
            				prod1_retvalue = waitpid(prod1 , &prod1_status , 0);
            				if (WIFEXITED(prod1_status) ) {
                				printf("PARENT: redproducer exited with status :%d\n", WEXITSTATUS(prod1_status) );
                			if (prod1_status == 0)
                    				printf("PARENT: redproducer exited successfully!\n");
                			else
                    				printf("PARENT: redproducer Failed !\n");
            				}
							/* blocking wait for BLUE producer */
            				prod2_retvalue = waitpid(prod2 , &prod2_status , 0);
            				if (WIFEXITED(prod2_status) ) {
                				printf("PARENT: blueproducer exited with status :%d\n", WEXITSTATUS(prod2_status) );
                			if (prod2_status == 0)
                    				printf("PARENT: blueproducer exited successfully!\n");
                			else
                    				printf("PARENT: blueproducer Failed !\n");
            				}
							/* blocking wait for consumer */
            				con_retvalue = waitpid(con , &con_status , 0);
            				if (WIFEXITED(con_status) ) {
                				printf("PARENT: Consumer exited with status :%d\n", WEXITSTATUS(con_status) );
                			if (con_status == 0)
                    				printf("PARENT: Consumer exited successfully!\n");
                			else
                    				printf("PARENT: Consumer Failed !\n");
            				}


            				sleep(1);
        			}
					/* Detach shared memory */
				    if((shmdt((void*)sm_ptr)) == -1){
				        perror("PARENT:Deattaching failed:\n");
				        exit(2);
				        }
				    /* Free shared memory */
				    if((shmctl(sm_id, IPC_RMID, NULL)) == -1){
				        perror("PARENT:shmctl failed \n");
				        exit(2);
				        }
			} // End of Parent Code

		}
	
	}	
} // End of main
