/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE main.c


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
	05-02-2018
---------------------------------------------------------*/ 

#include "general.h"

/*
	Function: ALARMhandler
	Alarm interruption
	Guarantees that the broadcast occurs at 1Hz
*/
void ALARMhandler(int sig){
	struct sigaction  sa;
	sa.sa_handler=ALARMhandler;
	sigaction(SIGALRM, &sa, NULL);

	// Control variables
	allow_dynamics=1;
	alarm(DT);
}

/*
	Function: intHandler
	To exit after user presses CTRL-C
*/
void intHandler(int sig){
	exiting=1;
}


/*
	Function: main
*/
int main(int argc, char const *argv[])
{

	// Selects initial conditions
	startup_menu();

	inputs.vert_speed = 0;		
	range_mode=0;
	range=20000;
	alert_TA = 0;
	alert_RA = 0;
	silent = 1;

	//Create broadcast udp socket
	struct Broad_info broad=initialize_Broadcast();

	//Initialize list of aircrafts
	head = malloc(sizeof(AC_list_t));
	if (head == NULL) {
	    exit(1);
	}

	//Assign its own aircraft to the 1st element
	head->AC = initialize_AC_own();
	head->next = NULL;

	print_list();

	//To detect CTRL+C
	signal(SIGINT, intHandler);
	exiting=0;

	//Create alarm interruption
	struct sigaction  sa;
	sa.sa_handler=ALARMhandler;
	sigaction(SIGALRM, &sa, NULL);

	allow_dynamics = 1;
	allow_tcas = 1;
	alarm(DT);

	//Create threads to run different functions in parallel
	pthread_t thr_listener, thr_tcas, thr_graph, thr_sound;
	pthread_create(&thr_listener, NULL, listener, NULL);
	pthread_create(&thr_tcas, NULL, tcas, NULL);
	pthread_create(&thr_graph, NULL, graph, NULL);
	pthread_create(&thr_sound, NULL, sound, NULL);

	//Updates the position of our aircraft and broadcasts the respective message
	dynamics(broad);

	pthread_join(thr_tcas, NULL);	
	pthread_join(thr_sound, NULL);

	printf("\nThank you for using our TCAS.\n");
	printf("Authors:\n");
	printf("Luís Bernardo, 78267\n");
	printf("Marta Marques, 78289\n");
	printf("Rafael Borges, 78796\n");

	exit(0);
}
