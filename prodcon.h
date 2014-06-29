/* Name: Samkit Jain
*/
#include <pthread.h>
#define BUFSIZE 10				// Total no. of items that can be present in a buffer at a time
#define BUFDEPTH 40				// Each item will be of 40 bytes ( assumed )
#define ITEMNO 10000			// No. of items to be produced (ITEMNO red and ITEMNO blue )
#define RED "0"
#define BLUE "1"

struct smspace{
	char buffer1[BUFSIZE][BUFDEPTH];
	char buffer2[BUFSIZE][BUFDEPTH];
	int count1 , count2;												// Tells no of Empty spaces in Buffer 1 and 2
	int redprodhead , blueprodhead;										// Points to location in buffer 1 and 2 resp. where producers will produce data
	int redconhead , blueconhead ;										// Points to location in buffer 1 and 2 resp. from where consumer will consume data
	int redproddone , blueproddone ;									// Flag to signal that Red and blue producers have finished producing total no. of items
	int Totalredcount , Totalbluecount ; 								// Total no. of RED and BLUE items produced
	pthread_mutex_t  mutex1 , mutex2 ;									// Mutex to acquire resources and shared variables
	pthread_cond_t  SpaceAvailable1, ItemAvailable,SpaceAvailable2;		// Condition variable for space available in buffer 1 , buffer 2 and items available in both the buffers 
};


