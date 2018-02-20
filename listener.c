/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE listener.c
	Contains the functions related to the listener thread


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
	05-02-2018
---------------------------------------------------------*/
#include "general.h"


/*
	Function: initialize_listener
	Creates a UDP broadcast socket, in which it listens for messages being sent to the network through the port
*/
int initialize_listener(void)
{
	int sd, rc;
	struct sockaddr_in local_addr;
	int broadcast = 1;

	// Creates UDP sockets
	sd=socket(AF_INET, SOCK_DGRAM, 0);
	if(sd<0) {	// in case of error
		printf("LISTENER: cannot open socket\n");
		exit(1);
	}

	// Sets the broadcast
	if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof(broadcast)) == -1) {
	      perror("setsockopt (SO_BROADCAST)");
	      exit(1);
	}

	// Binds the local server port
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(port);

	rc = bind (sd, (struct sockaddr *) &local_addr,sizeof(local_addr));
	if(rc<0) {
		printf("LISTENER: cannot bind port number \n");
		exit(1);
	}

	printf("LISTENER: waiting for data on port UDP %u\n", port);

	return sd;
}


/*
	Function: initialize_msg
	Initializes a message by setting all parameters to 0 
*/
message initialize_msg(message msg)
{
	msg.ID=0;
	msg.AP.X=0;
	msg.AP.Y=0;
	msg.AP.Z=0;
	msg.AV.X=0;
	msg.AV.Y=0;
	msg.AV.Z=0;
	strcpy(msg.TS,"CLEAR");
	msg.IH=0;
	strcpy(msg.R,"NONE");
	msg.RV=0;
	msg.CS=0;

	return msg;

}

/*
	Function: print_msg
	Prints all of the parameters of a message 
*/
void print_msg(message msg)
{
	printf("\n*-------Message-------*\n");
	printf("ID: %ld\n", msg.ID);
	printf("AP: %f %f %f\n", msg.AP.X, msg.AP.Y, msg.AP.Z);
	printf("AV: %f %f %f\n", msg.AV.X, msg.AV.Y, msg.AV.Z);
	printf("TS: %s\n", msg.TS);
	printf("IH: %ld\n", msg.IH);
	printf("R: %s\n", msg.R);
	printf("RV: %f\n", msg.RV);
	printf("CS: %ud\n\n", msg.CS);

}

/*
	Function: msg_to_ac
	Stores the data from the messages in the aircraft structure
*/
AC_t msg_to_ac(message msg)
{
	AC_t ac;
	ac.ID=msg.ID;
	ac.pos.x=msg.AP.X;
	ac.pos.y=msg.AP.Y;
	ac.pos.z=msg.AP.Z;
	ac.vel.x=msg.AV.X;
	ac.vel.y=msg.AV.Y;
	ac.vel.z=msg.AV.Z;
	strcpy(ac.status, msg.TS);
	strcpy(ac.resolution, msg.R);
	ac.intruder=msg.IH;
	ac.resolution_value=msg.RV;

	// save the current time in the list
	gettimeofday(&ac.time_msg,NULL);		// intruder's aircraft
	gettimeofday(&head->AC.time_msg,NULL);	// our aircraft

	return ac;
}

/*
	Function: insert_AC_in_list
	Inserts the intruder's aircraft in the list
*/
void insert_AC_in_list(AC_t ac_intruder)
{
	AC_list_t * current=head;

	// Go through all of the elements to find the last one
    while (current->next != NULL) {

        current = current->next;

    	//If AC already exists in the list, replace the old data
    	if(current->AC.ID==ac_intruder.ID){
    		current->AC=ac_intruder;
    		return;
    	}

    }

    //If it's new, add it in the end of the list
    current->next = malloc(sizeof(AC_list_t));
    current->next->AC = ac_intruder;
    current->next->next = NULL;

}

/*
	Function: listener
	Listens for intruder's messages in the network 
*/
void *listener(void *vargp){

	printf("\n***************Listener***********************\n");

	// Creates the server UDP broadcast socket
	int sd=initialize_listener();

	int n;
	struct sockaddr_in intruder_addr;
	socklen_t  addr_size;

	// Initialize message variable
	message msg;
	msg=initialize_msg(msg);
	char* stream=malloc(sizeof(message));
	uint32_t CS32;

	AC_t ac_intruder;
	// Control variable to guarantee that only one message is being received at a time
	receiving=0;

	// Program stays in a cycle until the user presses CTRL+C
	while(exiting == 0) {

		if(receiving==0) {
			receiving=1;
			// Receives message
			addr_size = sizeof(intruder_addr);
			n = recvfrom(sd, stream, sizeof(message) , 0, (struct sockaddr *) &intruder_addr, &addr_size);
			printf("\nReceived %d bytes from %s\n", n, inet_ntoa(intruder_addr.sin_addr));
			if(n<0) {
				printf("LISTENER: cannot receive data \n");
				receiving=0;
				continue;
			}

			// Translates string received in the message parameters
			sscanf(stream, "%"PRIu64" %lf %lf %lf %lf %lf %lf %s %"PRIu64" %s %lf %"PRIu32"",
			&msg.ID, &msg.AP.X,&msg.AP.Y, &msg.AP.Z,
			&msg.AV.X,& msg.AV.Y, &msg.AV.Z, msg.TS,
				&msg.IH, msg.R, &msg.RV, &msg.CS);


			CS32=checksum(msg);

			if(CS32!=msg.CS) {
				printf("Invalid msg!!\n");
				receiving=0;
				continue;
			}

			//Ignore its own messages
			if(msg.ID==head->AC.ID) {
				receiving=0;
				continue;
			}

			// Stores the message data in the aircraft structure
			ac_intruder=msg_to_ac(msg);

			// Insert ou replace aircraft in the list
			insert_AC_in_list(ac_intruder);

			// Reset control variables
			allow_tcas=1;
			receiving=0;
		}


	}


	free(stream);
	close(sd);
	return NULL;

}
