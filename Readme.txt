/* Name: Samkit Jain
*/

Instruction for compiling and running source file:
Execute following 2 commands in sequence from terminal by keeping the currect directory : the submission directory
1) make all
2) ./main.o

Upon successful run 3 text files are generated in current folder:
1. Producer_RED.txt
2. Producer_BLUE.txt
3. Consumer.txt


General Info :
There 4 files in the submission folder:
1. main.c		: Main code. Forks all processes ( consumer , Red-producer and Blue-producer )
2. prodcon.c	: Contains code for launching one thread per process and the task consumer , Red-producer and Blue-producer will perform
3. prodcon.h	: Contains the structures of all buffres , variable , locks , condition variables and flags . Also contains constant declaration. ITEMNo is the no. of items that will be produced by red and blue producers individually. For submission its value is set to 10000.
4. Makefile		: For compiling the code


Code Flow :
The main initializes a shared memory structure which contains 2 buffer (buffer1 and buffer 2 of length 10 and depth 40) , flags( redproddone , blueproddone) , condition variables (SpaceAvailable1 , SpaceAvailable2 and ItemAvailable) , header variables ( [redconhead , blueconhead] for referncing in buffer1 and buffer 2 by consumer ; [redprodhead , blueprodhead] for referncing in buffer1 and buffer 2 by redproducer and blue producer resp. ) and counters (Ccount1 and count2 keep track of empty spaces in buffer1 and buffer2 resp. ). 

It then forks 3 processes : Consumer process , Red-producer process and Blue-producer process and each process executes prodcon.c code . During call the processes specify whether they are consumer or producer and color of ittem they will produce i case if they are producer.

Each process then creates a thread and call respective function ( consumer process thread call consumerfn , producer process call producerfn). The producer process thread also pass pass a variable itemtype as function parameter. itemtype=0 for Red-producer thraed and item-type=1 for blu-producer thread. Depending on this value producer thread generate either RED item(in case of Red-producer thread) or BLUE item(in case of Blue-producer thread).

The producer function loop till they don't generate ITEMNO items whereas Consumer process loops till it hasn't consumed all items in buffer1 and buffer2 and both producers have exited.
The functionailty of producers and consumers is exactly as asked in assigment definition.





 


