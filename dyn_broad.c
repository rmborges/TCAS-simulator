/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE dyn_broad.c
	Contains functions related to the dynamics and broadcast thread


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
    05-02-2018
---------------------------------------------------------*/

#include "general.h"

/*
	Function: startup_menu
	Asks the user to choose the initial conditions of the aircraft and the network parameters (broadcast address and port) 
*/
void startup_menu (void){
	int valid=0,c;
	char ch, enter;

	printf("\nWelcome to out TCAS simulator!\n");

	// ask for initial aircraft conditions to be stored in global struct (inputs)
	printf("\nGive the initial conditions for your aircraft:\n");

	while(valid==0){
		printf("Would you like to use the default values? (Y/N)\n");
		valid=scanf("%c%c", &ch,&enter);
		if(valid!=2 || enter!='\n'){
			printf("Invalid character\n");

			// Clear input buffer
			while ( (c = getchar()) != '\n' && c != EOF );

			valid=0;
			continue;
		}
	}

	if ((ch=='N') ||(ch=='n')){
		valid=0;
		while(valid==0) {
			printf("\nID (uint64):\n");
			valid=scanf("%"PRIu64"%c", &inputs.ID,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid ID\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}
			printf("Lat (deg) [-90º,90º]:\n");
			valid=scanf("%lf%c", &inputs.lat,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid Lat\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}			
			printf("Lon (deg) [-180º,180º]:\n");
			valid=scanf("%lf%c", &inputs.lon,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid Lon\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}			
			printf("Alt (ft):\n");
			valid=scanf("%lf%c", &inputs.alt,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid altitude\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}				
			printf("Speed (kn):\n");
			valid=scanf("%lf%c", &inputs.hor_speed,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid speed\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}				
			printf("Heading (deg):\n");
			valid=scanf("%lf%c", &inputs.heading,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid heading\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}				
			printf("Mode (M-manual, A-automatic):\n");
			valid=scanf("%c%c", &inputs.mode,&enter);
			if(valid!=2 || enter!='\n' || (inputs.mode!='A'&& inputs.mode!='M')) {
				printf("Invalid mode\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}

		}
	}
	else {
		inputs.ID = 270;
		inputs.mode='A';
		inputs.lat = 38; 			//deg
		inputs.lon = -8.5; 			//deg
		inputs.alt = 35000;			//ft
		inputs.heading = 270;		//deg
		inputs.hor_speed = 400;		//knots		
						
	}

	// Limit heading range
	if(inputs.heading<0)
		inputs.heading=inputs.heading+360;
	if(inputs.heading<0)
		inputs.heading=0;
	if(inputs.heading>360)
		inputs.heading=360;

	// Limit latitude and longitude ranges		
	if(inputs.lat<-90)
		inputs.lat=-90;
	if(inputs.lat>90)
		inputs.lat=90;
	if(inputs.lon<-180)
		inputs.lon=-180;
	if(inputs.lon>180)
		inputs.lon=180;


	printf("\nInitial conditions:\n");
	printf("ID: %"PRIu64"\n",inputs.ID);
	printf("Lat: %0.1lfº\n", inputs.lat);
	printf("Lon: %0.1lfº\n",inputs.lon);
	printf("Alt: %0.1lf ft\n",inputs.alt);
	printf("Speed: %0.1lf kn\n",inputs.hor_speed);
	printf("Heading : %0.1lfº\n",inputs.heading);
	printf("Mode: %c\n", inputs.mode);

	printf("\nPlease, confirm the broadcast network parameters:\n");
	printf("Port:%d Address:%s\n", B_PORT, B_ADDRESS);
	valid=0;
	while(valid!=2){
		printf("Would you like to change them?(Y/N)\n");
		valid=scanf("%c%c", &ch,&enter);

		if(valid!=2 || enter!='\n'){
			printf("Invalid character\n");
			while ( (c = getchar()) != '\n' && c != EOF );
			valid=0;
			continue;
		}
	}	
	if ((ch=='Y') ||(ch=='y')){	
		valid=0;
		while(valid==0){		
			printf("Port:\n");
			valid=scanf("%d%c", &port,&enter);
			if(valid!=2 || enter!='\n') {
				printf("Invalid port\n");
				while ( (c = getchar()) != '\n' && c != EOF );
				valid=0;
				continue;
			}			
			printf("Address:\n");
			valid=scanf("%s", address);	
			if(valid!=1 || verify_address(address)==-1) {
				printf("Invalid address\n");
				valid=0;
			}
		}
	}
	else {
		port=B_PORT;
		strcpy(address, B_ADDRESS);
	}

}

/*
	Function: verify_address
	Verifies if the address is a valid IP address
	Rules: 4 parts of 1-3 numbers separated by a '.'
*/
int verify_address(char*host){
 
    int i, num=0, part=1;
 
    for(i=0; host[i]!='\0';i++){
         
        num++;
         
        if(host[i]==46){ //if it's a dot "."
             
            if(num>=2 && num<=4){ //End of part
                num=0;
                part++;
            }
 
            else
                return -1;
        }               
     
        if((host[i]<48 || host[i]>57) && host[i]!=46) //if it's not a number nor '.'
            return -1;
         
    }
 
    if(part!=4 || num==0)
        return -1;
     
    return 0;
}

/*
  Function: initialize_Broadcast
  Creates a UDP broadcast socket
  Returns the socket ID and the address
*/
struct Broad_info initialize_Broadcast(void){
	int sd;
	struct sockaddr_in broad_addr;
	struct hostent *h;
	int broadcast = 1;

	// Get the IP broadcast address
	h = gethostbyname(address);

	printf("\nBroadcast setup: to '%s'\n", inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

	// Setup the socket parameters
	broad_addr.sin_family = h->h_addrtype;
	memcpy((char *) &broad_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	broad_addr.sin_port = htons(port);	//Broadcast port

	// Creates the socket
	sd = socket(AF_INET,SOCK_DGRAM,0);
	if(sd<0) {		// In case of error
		printf("BROADCAST: Cannot open socket \n");
		exit(1);
	}

	// Creates the broadcast
	if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof(broadcast)) == -1) {
	      perror("setsockopt (SO_BROADCAST)");
	      exit(1);
	}

	// Stores socket ID and address in structure that will be returned by function 
	struct Broad_info broad;
	broad.sd=sd;
	broad.addr=broad_addr;

	return broad;

}


/*
  Function: get_msg
  Creates a message to be broadcast from the aircraft structure parameters
*/
message get_msg(AC_t AC_own)
{
	message msg;

	msg.ID = AC_own.ID;
	msg.AP.X = AC_own.pos.x;
	msg.AP.Y = AC_own.pos.y;
	msg.AP.Z = AC_own.pos.z;
	msg.AV.X = AC_own.vel.x;
	msg.AV.Y = AC_own.vel.y;
	msg.AV.Z = AC_own.vel.z;
	strcpy(msg.TS,AC_own.status);
	msg.IH = AC_own.intruder;
	strcpy(msg.R,AC_own.resolution);
	msg.RV = AC_own.resolution_value;

	// Calculate the checksum of the message
	msg.CS = checksum(msg);

	return msg;

}


/*
  Function: dynamics
  Handles the dynamics of our aircraft and the broadcast to the network
  Receives as input the socket parameters (ID and address)
  Returns -1 in case of error
*/
int dynamics(struct Broad_info broad)
{
	int rc;
    struct timeval t;

	//Initilize message to broadcast
	message msg;
	char* stream=malloc(sizeof(message));

	// Initialize global variable to guarantee synchronization
	sending=0;

	// Program stays in a cycle until the user presses CTRL+C
	while(exiting == 0) {

		//Every 1 second
		if(allow_dynamics==1) {
			// Gets the current time
    		gettimeofday(&t, NULL);
    		if((t.tv_sec - head->AC.time_msg.tv_sec) > TIMEOUT)	// If it does not receive messages after 10s
				remove_distant_ac(head);						// Removes aircrafts from list

			// Computes the necessary velocities to follow the desired route
			follow_route(inputs.heading, inputs.hor_speed, inputs.alt);		//It is only possible the change the route in manual mode
			head->AC=update_aircraft_own(head->AC);		// Updates the position of the aircraft

			// Creates the message to be broadcast
			msg=get_msg(head->AC);

			// To guarantee that only one message is sent at a time
			if(sending==0){

				sending=1;
				
				// Converts message into a string
				sprintf(stream, "%"PRIu64" %lf %lf %lf %lf %lf %lf %s %"PRIu64" %s %lf %"PRIu32,
		          msg.ID, msg.AP.X, msg.AP.Y, msg.AP.Z,
		          msg.AV.X, msg.AV.Y, msg.AV.Z, msg.TS,
		          msg.IH, msg.R, msg.RV, msg.CS);

				// Send message string via broadcast
				rc = sendto(broad.sd, stream, sizeof(message), 0, (struct sockaddr *) &broad.addr, sizeof(broad.addr));
				printf("\nSending %d bytes\n", rc);
				if(rc<0) {
					printf("BROADCAST: cannot send \n");
					close(broad.sd);
					exit(-1);
				}

				// Reset control variables
				allow_dynamics=0;
				sending=0;
			}
		}
	}

	// Close socket
	close(broad.sd);
	free(stream);
	return 1;
}
