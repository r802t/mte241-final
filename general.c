#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <stdio.h>
#include <stdbool.h>
// add any #defines here

// add global variables here
int report = 0; //global reporter
char queue[7];
int n_generals = 0; //global # of generals
int send; //global sender
int traitor ; //global traitor
int count = 0;
osSemaphoreId_t sem;
osStatus_t stat;

/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
	
void checkStatus (osStatus_t stat){ //check for semaphore function return status
		if (stat == osErrorResource)
			printf("Error Resource");
		else if (stat == osErrorTimeout)
			printf("Error Timeout");
		else if (stat == osErrorParameter)
			printf("Error Parameter");
}

bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	report = reporter; //copy value to global reporter
	int n_traitor = 0; 
	for (int i=0; i<sizeof(&loyal)/sizeof(bool); i++)
		if (loyal[i] == false) {
			n_traitor++;
			traitor = i; //record traitor
		}
	if (!c_assert(nGeneral > 3*n_traitor)){ //check if it satisfies n>3m
		traitor = 0;
		return false;
	}
	n_generals = nGeneral; //record number of generals
	return true;
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
		stat = osSemaphoreDelete(sem);
		checkStatus(stat);
}


/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t sender) {
	send = sender;
	sem = osSemaphoreNew(n_generals,0,NULL);
		for (int i=0; i<n_generals;i++){
			if (i != sender){
					if (traitor == sender && i%2 ==0) //if traitor & it is even number
						queue[i] = 'R';
					else if (traitor == sender && i%2 ==1) //if traitor & it is odd number
						queue[i] = 'A';
					else
						queue[i] = command; //if sender is not traitor
			}
		stat = osSemaphoreAcquire(sem,osWaitForever); //acquire n semeaphores
		checkStatus(stat);
		}
}


/** Generals are created before each test and deleted after each
  * test.  The function should wait for a value from broadcast()
  * and then use the OM algorithm to solve the Byzantine General's
  * Problem.  The general designated as reporter in setup()
  * should output the messages received in OM(0).
  * idPtr: pointer to general's id number which is in [0,n-1]
  */
void general(void *idPtr) {
	int id = *(uint8_t *)idPtr;
		if (report == id && n_generals > 3){ //OM(2)--OM(1)
			for (int i =0; i<n_generals; i++)
					if(i!=send && i!= report){ //only consider generals'command except sender and reporter
						if(traitor == i && i%2 == 0) //even number
							printf("%d:%d:%c ",i,send,'R');
						else if (traitor == i && i %2 ==1)//odd number
							printf("%d:%d:%c ",i,send,'A');
						else
							printf("%d:%d:%c ",i,send,queue[i]); //loyal member
					}
		}
		else if (report ==id) //OM(1)
			printf("%d:%c ",send,queue[report]);
	stat = osSemaphoreRelease(sem); //release 1 semaphore for every general
	checkStatus(stat);
	osThreadYield();
}

