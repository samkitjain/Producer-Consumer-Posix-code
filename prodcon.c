/* Name: Samkit Jain
   Student ID: 4634193
   CSE-Lab Login: jainx224
*/
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "prodcon.h"
#include <stdlib.h>

/* Structure for consumer & producer function parameters */
struct conparam{
	int fd ;
	struct smspace *conptr;
};
struct prodparam{
	int fd ;
	int itemtype ;		// 0 for RED item and 1 for BLUE item
	struct smspace *prodptr;
};

																	/* Consumer Function */
void *consumerfn(void *arg){
	struct conparam *myparam = (struct conparam *)malloc(sizeof(struct conparam));
	myparam = (struct conparam *)arg ;
	while(1){
			pthread_mutex_lock(&(myparam->conptr-> mutex1));  				// Acquire mutex1
			pthread_mutex_lock(&(myparam->conptr-> mutex2));  				// Acquire mutex2
				if(myparam->conptr->count1 == BUFSIZE ){					// Check if Buffer 1 is full
					if(myparam->conptr->count2 == BUFSIZE ){				// Check if Buffer 2 is full

							/* Both buffers are full. Consumer will wait but before it checks if both producers have finished producing all items*/
							if(myparam->conptr->redproddone == 1 && myparam->conptr->blueproddone == 1){
									printf("CONSUMER: Both producer have finished producing !\n");
									pthread_exit(NULL); // Both producers have exited so consumer thread exits
							}
							pthread_mutex_unlock(&(myparam->conptr-> mutex2));  // Release mutex1
							/* Since both buffers are full consumer waits on ItemAvailable condition variable till producer produces item */ 
							while(pthread_cond_wait (&(myparam->conptr->ItemAvailable),&(myparam->conptr-> mutex1))!=0);
							pthread_mutex_unlock(&(myparam->conptr-> mutex2));  // Release mutex1
						}
							else{
								/*Consume product from buffer2 since buffer1 is full */
								int bluehead;
								bluehead = 	myparam->conptr->blueconhead ;				   // Point to element to be consumed from buffer2
								/* Consume item at head of buffer2 and write it to Consumer.txt */
								if(write(myparam->fd , myparam->conptr->buffer2[bluehead] , strlen(myparam->conptr->buffer2[bluehead])) != strlen(myparam->conptr->buffer2[bluehead]))
										perror("CONSUMER: Error in writting :\n");
								/* Force curson to write to next line in Consumer.txt */
								if(write(myparam->fd ,"\n", 1) != 1)
										perror("CONSUMER: Error in writting :\n");			
								myparam->conptr->blueconhead = (bluehead + 1) % BUFSIZE ; // Increment header to point to next element to be consumed
								myparam->conptr->count2 = myparam->conptr->count2 + 1 ;	  // Increment count2 to show availabilty of one space in buffer2
								pthread_cond_signal(&(myparam->conptr-> SpaceAvailable2));   // Signal SpaceAvailable2
								pthread_mutex_unlock(&(myparam->conptr-> mutex1));  		 // Release mutex2
								pthread_mutex_unlock(&(myparam->conptr-> mutex2));  	     // Release mutex1
							}
				}

				else{
						/*Consume product from buffer 1 */		
						int redhead;
						redhead = myparam->conptr->redconhead ;									// Point to element to be consumed from buffer1
						/* Consume the item at head of buffer1 and write it to Consumer.txt */
						if(write(myparam->fd , myparam->conptr->buffer1[redhead] , strlen(myparam->conptr->buffer1[redhead])) != strlen(myparam->conptr->buffer1[redhead]))
								perror("CONSUMER: Error in writting :\n");
						/* Force curson to write to next line in Consumer.txt */
						if(write(myparam->fd ,"\n", 1) != 1)
								perror("CONSUMER: Error in writting :\n");			
						myparam->conptr->redconhead =  (redhead + 1) % BUFSIZE ;	// Increment header to point to next element to be consumed
						myparam->conptr->count1 = myparam->conptr->count1 + 1 ;		// Increment count1 to show availabilty of one space in buffer1
						pthread_cond_signal(&(myparam->conptr-> SpaceAvailable1));  // Signal SpaceAvailable1
						pthread_mutex_unlock(&(myparam->conptr-> mutex1));  		// Release mutex2
						pthread_mutex_unlock(&(myparam->conptr-> mutex2));  		// Release mutex1
				}
		} // End of while loop
} // End of consumer function
																	/* Consumer Function ends */


																	/* Producer Function */
void *producerfn(void *arg){
	int errno; // To check total no. of bytes written by write() command . Used for error checking
	/* Pointer to argument structure element */
	struct prodparam *myparam = (struct prodparam *)malloc(sizeof(struct prodparam));
	myparam = (struct prodparam *)arg ;
	char timestampstring [35] ; // Array to hold time stamp
	struct timeval time ;

	
	/* itemtype = 0 means it is Red Producer  &  itemtype = 1 means it is Blue Producer*/


	if (myparam->itemtype == 0){									/* RED producer */
			/* Loop untill no. of red items produced =  ITEMNO */
			while(myparam->prodptr->Totalredcount<ITEMNO){
					char item [40] = "RED  "; // Array to hold RED item temporarily
					pthread_mutex_lock(&(myparam->prodptr-> mutex1));  // Acquire mutex1
					pthread_mutex_lock(&(myparam->prodptr-> mutex2));  // Acquire mutex2
					if(myparam->prodptr->count1 ==0 ){							// Check if empty space is buffer1
						if(myparam->prodptr->count2 ==0 ){						// Check if empty space is buffer2
							/* Both Buffers are full. Producer will wait till space is not free in either buffers */
							pthread_mutex_unlock(&(myparam->prodptr-> mutex2));  // Release mutex2
							/* Producer will wait on SpaceAvailable1 condition variable till consumer doesn't create an empty space */
							while(pthread_cond_wait (&(myparam->prodptr->SpaceAvailable1),&(myparam->prodptr-> mutex1))!=0); // Wait for signal from consumer
							pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  // Release mutex1
						}

						/*  Buffer 1 full but Buffer2 is empty. Generating data in buffer2 at its head */
						else{
							int bluehead;
							bluehead = myparam->prodptr->blueprodhead ;  // Head of buffer2 where item will be produced
							/* Generate TimeStamp and RED item */
							gettimeofday(&time , NULL);
							sprintf(timestampstring , "%llu" , (((unsigned long long)time.tv_sec *1000000ULL) + (unsigned long long)time.tv_usec));
							strcat(item, timestampstring);
							/* Write item in buffer2 */
							strncpy(myparam->prodptr->buffer2[bluehead] , item , strlen(item));
							/* Write item in RED_producer.txt */
							if(errno = write(myparam->fd , myparam->prodptr->buffer2[bluehead] , strlen(myparam->prodptr->buffer2[bluehead]))!=strlen(myparam->prodptr->buffer2[bluehead]))
									perror("REDPRODUCER: Error in writting data  in buffer2\n");
							if(write(myparam->fd ,"\n", 1) != 1)
									perror("REDPRODUCER: Error in writting nextline operator\n");

							myparam->prodptr->count2 = myparam->prodptr->count2 - 1;						// Decrement count2 to indiacte one less space available in buffer2
							myparam->prodptr->blueprodhead = (bluehead + 1) % BUFSIZE ; 					// Increment header to point to next buffer2 space where item will be produced
							myparam->prodptr->Totalredcount = myparam->prodptr->Totalredcount + 1 ;			// Increment total number of RED items produced
							pthread_cond_signal(&(myparam->prodptr-> ItemAvailable)); 						// Signal ItemAvailable
							/* Check if RED producer has finished producing RED items it is required to */ 
							if(myparam->prodptr->Totalredcount == ITEMNO)									
									myparam->prodptr->redproddone = 1 ;									    // Set redproddone flag to indicate that all RED items have been produced
							pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  							// Release mutex1
							pthread_mutex_unlock(&(myparam->prodptr-> mutex2));  							// Release mutex2
						}
					}
					/*  Buffer 1 is empty. Generating data in buffer1 at its head */
					else{
						int redhead ;
						redhead = myparam->prodptr->redprodhead ; 	// Head of buffer1 where item will be produced
						/* Generate TimeStamp and RED item */
						gettimeofday(&time , NULL);
						sprintf(timestampstring , "%llu" , (((unsigned long long)time.tv_sec *1000000ULL) + (unsigned long long)time.tv_usec));
						strcat(item, timestampstring);
						/* Write item in buffer1 */
						strncpy(myparam->prodptr->buffer1[redhead] , item , strlen(item));
						/* Write item in RED_producer.txt */
						if(errno = write(myparam->fd , myparam->prodptr->buffer1[redhead] , strlen(myparam->prodptr->buffer1[redhead]))!=strlen(myparam->prodptr->buffer1[redhead]))
								perror("REDPRODUCER: Error in writting data in buffer1\n");
						if(write(myparam->fd ,"\n", 1) != 1)
								perror("REDPRODUCER: Error in writting nextline operator\n");

						myparam->prodptr->count1 = myparam->prodptr->count1 - 1;							// Decrement count1 to indiacte one less space available in buffer1
						myparam->prodptr->redprodhead = (redhead + 1) % BUFSIZE;							// Increment header to point to next buffer1 space where item will be produced
						myparam->prodptr->Totalredcount = myparam->prodptr->Totalredcount + 1 ;				// Increment total number of RED items produced
						pthread_cond_signal(&(myparam->prodptr-> ItemAvailable)); 							// Signal ItemAvailable
						/* Check if RED producer has finished producing RED items it is required to */ 
						if(myparam->prodptr->Totalredcount == ITEMNO)
								myparam->prodptr->redproddone = 1 ;								    		// Set redproddone flag to indicate that all RED items have been produced
						pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  								// Release mutex1
						pthread_mutex_unlock(&(myparam->prodptr-> mutex2)); 								 // Release mutex2
						}
				}
			pthread_exit(NULL);
	}
																	/* Red Producer Ends */

																	/* Blue Producer */
	else{

		/* Loop untill no. of red items produced =  ITEMNO */
		while(myparam->prodptr->Totalbluecount <ITEMNO){
				char item [40] = "BLUE ";											// Array to hold BLUE item temporarily
				pthread_mutex_lock(&(myparam->prodptr-> mutex1));  					// Acquire mutex1
				pthread_mutex_lock(&(myparam->prodptr-> mutex2));  					// Acquire mutex2
				if(myparam->prodptr->count2 ==0 ){									// Check if empty space is buffer2
					if(myparam->prodptr->count1 ==0 ){								// Check if empty space is buffer1
							/* Both Buffers are full. Producer will wait till space is not free in either buffers */
							pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  	// Release mutex2
							/* Producer will wait on SpaceAvailable2 condition variable till consumer doesn't create an empty space */
							while(pthread_cond_wait (&(myparam->prodptr->SpaceAvailable2),&(myparam->prodptr-> mutex2))!=0); // Wait for signal from consumer
							pthread_mutex_unlock(&(myparam->prodptr-> mutex2));  // Release mutex2				
					}
					/*  Buffer2 full but Buffer1 is empty. Generating data in buffer1 at its head */
					else{
						/* Generating data in buffer1 at its head */
						int redhead;
						redhead = myparam->prodptr->redprodhead ;		// Head of buffer1 where item will be produced
						/* Generate TimeStamp and BLUE item */
						gettimeofday(&time , NULL);
						sprintf(timestampstring , "%llu" , (((unsigned long long)time.tv_sec *1000000ULL) + (unsigned long long)time.tv_usec));
						strcat(item, timestampstring);
						/* Write item in buffer1 */
						strncpy(myparam->prodptr->buffer1[redhead] , item , strlen(item));
						/* Write item in BLUE_producer.txt */
						if(  errno = write(myparam->fd , myparam->prodptr->buffer1[redhead] , strlen(myparam->prodptr->buffer1[redhead]))!=strlen(myparam->prodptr->buffer1[redhead]))
							perror("BLUEPRODUCER: Error in writting data in buffer1\n");
						if(write(myparam->fd ,"\n", 1) != 1)
							perror("BLUEPRODUCER: Error in writting nextline operator\n");

						myparam->prodptr->count1 = myparam->prodptr->count1 - 1;						// Decrement count1 to indiacte one less space available in buffer1
						myparam->prodptr->redprodhead = (redhead + 1) % BUFSIZE ;						// Increment header to point to next buffer1 space where item will be produced
						myparam->prodptr->Totalbluecount = myparam->prodptr->Totalbluecount +1 ;		// Increment total number of BLUE items produced
						pthread_cond_signal(&(myparam->prodptr-> ItemAvailable)); 						// Signal ItemAvailable
						/* Check if BLUE producer has finished producing BLUE items it is required to */ 
						if(myparam->prodptr->Totalbluecount == ITEMNO)
							myparam->prodptr->blueproddone = 1 ;										// Set blueproddone flag to indicate that all BLUE items have been produced
						pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  							// Release mutex1
						pthread_mutex_unlock(&(myparam->prodptr-> mutex2)); 							// Release mutex2
					}
				}
				/*  Buffer1 full but Buffer2 is empty. Generating data in buffer2 at its head */
				else{
					int bluehead ;
					bluehead = myparam->prodptr->blueprodhead ;				// Head of buffer2 where item will be produced
					/* Generating data in buffer2 at its head */
					/* Generate TimeStamp and BLUE item */
					gettimeofday(&time , NULL);
					sprintf(timestampstring , "%llu" , (((unsigned long long)time.tv_sec *1000000ULL) + (unsigned long long)time.tv_usec));
					strcat(item, timestampstring);
					/* Write item in buffer2 */
					strncpy(myparam->prodptr->buffer2[bluehead] , item , strlen(item));
					/* Write item in BLUE_producer.txt */
					if(errno = write(myparam->fd , myparam->prodptr->buffer2[bluehead] , strlen(myparam->prodptr->buffer2[bluehead]))!=strlen(myparam->prodptr->buffer2[bluehead]))
						perror("BLUEPRODUCER: Error in writting data  in buffer2\n");
					if(write(myparam->fd ,"\n", 1) != 1)
						perror("BLUEPRODUCER: Error in writting nextline operator\n");
					
					myparam->prodptr->count2 = myparam->prodptr->count2 - 1;							// Decrement count2 to indiacte one less space available in buffer2
					myparam->prodptr->blueprodhead = (bluehead + 1) % BUFSIZE ;							// Increment header to point to next buffer1 space where item will be produced
					myparam->prodptr->Totalbluecount = myparam->prodptr->Totalbluecount +1 ;			// Increment total number of BLUE items produced
					pthread_cond_signal(&(myparam->prodptr-> ItemAvailable)); 							// Signal ItemAvailable
					/* Check if BLUE producer has finished producing BLUE items it is required to */ 
					if(myparam->prodptr->Totalbluecount == ITEMNO)
						myparam->prodptr->blueproddone = 1 ;											// Set blueproddone flag to indicate that all BLUE items have been produced
					pthread_mutex_unlock(&(myparam->prodptr-> mutex1));  								// Release mutex1
					pthread_mutex_unlock(&(myparam->prodptr-> mutex2));  								// Release mutex2
					}
		} // End of while loop
	}	// End of BLUE producer
	pthread_exit(NULL);
	/* Blue Producer Ends */
}
																		/* Producer Ends */


main( int argc, char* argv[] ) {

		int type ;    					// Type of comodity : RED or BLUE
		int fdout;    					// File descriptor for log file
		mode_t  perms;
		perms = 0740; 					// Permission
		int prodcon = atoi(argv[1] );  // prodcon = 1 for producer and 0 for consumer
		key_t key;   					// key for accessing shared memory

		switch(prodcon){

			/* Consumer Process*/
			case 0:
						/* Consumer process and thread Initialization */
						key = atoi(argv[2]);  // Key for shared memory
						/* Shared memory setup and utilization code has been built with help of shared memory: child.c example given on course website */
						int consumer_sm_id;
						struct smspace *consumer_sm_ptr ;
						if((consumer_sm_id = shmget(key , 0 , 0)) == -1){
								perror("CONSUMER: shmget failed !");
						}
						consumer_sm_ptr = (void *)shmat(consumer_sm_id , (void *)NULL , sizeof(struct smspace));
						if(consumer_sm_ptr == (void *) -1 ){
							 	perror("CONSUMER:shmat failed\n");
								exit(2);
						}

						/* Open consumer log file */
						if((fdout = open("Consumer.txt" , (O_WRONLY | O_CREAT | O_APPEND), perms)) == -1){
								perror("Error in opening the output consumer file\n :");
								exit(2);
						}

						/* Consumer Thread Setup */
						pthread_t consumer;  // thread variables  
					 	pthread_attr_t attr; // attribute object 
					 	int n;
						/* Thread Attribute initialization and setup starts */	
						if ( n = pthread_attr_init( &attr ) ){
						 		fprintf(stderr,"pthread_init_err: %s\n",strerror(n)); 
						 		exit(1); 
						}
					 	/* Set thread attributes */
						pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);
						pthread_attr_setscope(&attr , PTHREAD_SCOPE_SYSTEM);
						/* Comsumer function parameters */
						struct conparam consumerparam;
						consumerparam.fd = fdout;
						consumerparam.conptr = consumer_sm_ptr;
						/* Creating consumer thread */ 
						 if ( n = pthread_create(&consumer, &attr, consumerfn ,(void *) &consumerparam)) {
								printf("CONSUMER: Value of n is %d",n); 
						 		fprintf(stderr,"pthread_create :%s\n",strerror(n)); 
						 		exit(1); 
					 	}
						/* Wait for Consumer thread to finish */
						pthread_join(consumer, NULL);
						/* Detaching shared memory */
						if((shmdt((void *)consumer_sm_ptr)) == -1){
								perror("CHILD: Deattaching failed :\n");
								exit(2);
						}
						/* Close consumer log file */
						close(fdout);
						/* Return 0 on successful execution else return error no */
						return 0;
						break;
				/* Consumer Process Ends */


			/* Producer Process */
			case 1:
					/* Producer process and thread Initialization*/
					type = atoi(argv[2] ); // 0 for red producer and 1 for blue producer
					key = atoi(argv[3]);  // Key for shared memory

					/* Red Producer Process */
					if(type == 0){
								/* Shared memory setup and utilization code has been built with help of shared memory: child.c example given on course website */
								int redproducer_sm_id;
								struct smspace *redproducer_sm_ptr ;

							   	if((redproducer_sm_id = shmget(key , 0 , 0)) == -1){
										perror("REDPRODUCER: shmget failed !");
								}
								redproducer_sm_ptr = (void *)shmat(redproducer_sm_id , (void *)NULL , sizeof(struct smspace));
								if(redproducer_sm_ptr == (void *) -1 ){
									 	perror("REDPRODUCER:shmat failed\n");
										exit(2);
								}
								if((fdout = open("Producer_RED.txt" , (O_WRONLY | O_CREAT | O_APPEND), perms)) == -1){
										perror("Error in opening the output consumer file\n :");
										exit(2);
								}

								/* Red Producer Thread Setup */
								pthread_t redproducer; //thread variables 
							 	pthread_attr_t attr;  //attribute object
							 	int n;
								/* Thread Attribute initialization */	
								if ( n = pthread_attr_init( &attr ) ){ 
								 		fprintf(stderr,"pthread_init_err: %s\n",strerror(n)); 
								 		exit(1); 
								}
							 	/* Setting thread attributes */
								pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE); 
								pthread_attr_setscope(&attr , PTHREAD_SCOPE_SYSTEM); 
								/* Red producer function parameters */
								struct prodparam producerparam;
								producerparam.fd = fdout;
								producerparam.itemtype = type ;
								producerparam.prodptr = redproducer_sm_ptr;
								/* Creating Red producer thread */ 
								 if ( n = pthread_create(&redproducer, &attr, producerfn ,(void *) &producerparam)) {
										printf("CONSUMER: Value of n is %d",n); 
								 		fprintf(stderr,"pthread_create :%s\n",strerror(n)); 
								 		exit(1); 
							 	}
								/* Wait for Red producer thread */
								pthread_join(redproducer, NULL);
								/* Detaching shared memory */
								if((shmdt((void *)redproducer_sm_ptr)) == -1){
										perror("REDPRODUCER: Deattaching failed :\n");
										exit(2);
								}
					}
					/* Red Producer Process ends */

					/* Blue Producer Process */
					else{
								/* Shared memory setup and utilization code has been built with help of shared memory: child.c example given on course website */
								int blueproducer_sm_id;
								struct smspace *blueproducer_sm_ptr ;

								if((blueproducer_sm_id = shmget(key , 0 , 0)) == -1){
										perror("BLUEPRODUCER: shmget failed !");
								}
								blueproducer_sm_ptr = (void *)shmat(blueproducer_sm_id , (void *)NULL , sizeof(struct smspace));
								if(blueproducer_sm_ptr == (void *) -1 ){
										perror("BLUEPRODUCER:shmat failed\n");
										exit(2);
								}
								if((fdout = open("Producer_BLUE.txt" , (O_WRONLY | O_CREAT | O_APPEND), perms)) == -1){
										perror("Error in opening the output consumer file\n :");
										exit(2);
								}

								/* Blue Producer Thread Setup */
								pthread_t blueproducer; // thread variables
							 	pthread_attr_t attr;   //attribute object 
							 	int n;
								/* Thread Attribute initialization */	
								if ( n = pthread_attr_init( &attr ) ){
								 		fprintf(stderr,"pthread_init_err: %s\n",strerror(n)); 
								 		exit(1); 
								}
							 	/* setting blue producer thread attributes */
								pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);
								pthread_attr_setscope(&attr , PTHREAD_SCOPE_SYSTEM);
							 
								/* Blue Producer thread function parameters*/
								struct prodparam producerparam;
								producerparam.fd = fdout;
								producerparam.itemtype = type ;
								producerparam.prodptr = blueproducer_sm_ptr;
								/* Creating Blue Producer thread */ 
								 if ( n = pthread_create(&blueproducer, &attr, producerfn ,(void *) &producerparam)) {
										printf("BLUEPRODUCER: Value of n is %d",n); 
								 		fprintf(stderr,"pthread_create :%s\n",strerror(n)); 
								 		exit(1); 
							 	}
								/* Wait for Blue Producer thread to finish */
								pthread_join(blueproducer, NULL);
								/* Detaching shared memory */
								if((shmdt((void *)blueproducer_sm_ptr)) == -1){
										perror("BLUEPRODUCER: Deattaching failed :\n");
										exit(2);
								}

					}
					/* Closing producer file */
					close(fdout);
					/* Return 0 on successful execution else return error no */
					return 0;
					break;
					/* Producer Process Ends */

			default:
				printf("Unrecognized option. Exiting from prodcon !\n");
				return 0;
				break;

			 } // Switch case ends

} // main ends 
