/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE general.h
	Contains the includes, defines, global variables and functions used in the program


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
    05-02-2018
---------------------------------------------------------*/

#ifndef SAI_GENERAL_H
#define SAI_GENERAL_H

// Includes --------------------------------------------------------------------

#include "math.h"
#include "stdint.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <inttypes.h>
#include <g2.h>
#include <g2_X11.h>
#include <vlc/vlc.h>


// Defines ---------------------------------------------------------------------
#define PI 3.14159265358979323846

//TCAS resolution values
#define ALT_ERROR 20                    //ft
#define ROC_CLIMB2ROUTE 2000*0.3048/60    //fpm->m/s
#define ROC_DESCEND2ROUTE -2000*0.3048/60  //fpm->m/s
#define ROC_CLIMB 1500*0.3048/60          //fpm->m/s
#define ROC_DESCEND -1500*0.3048/60       //fpm->m/s
#define ROC_CLIMB_NOW 2500*0.3048/60       //fpm->m/s
#define ROC_DESCEND_NOW -2500*0.3048/60    //fpm->m/s

#define CHECK_SUM 32

//Communications constants
#define B_PORT 8000
#define B_ADDRESS "192.168.1.255"

#define STATUS_SIZE 30

#define DT 1
#define TIMEOUT 10

//G2 Window
#define WINDOWX 910
#define WINDOWY 520

// Sound alerts
#define NUM_ALERTS_RA 5
#define NUM_ALERTS_TA 3

// Structs Definitions----------------------------------------------------------
struct Inputs {
    uint64_t ID;
    char mode; //Automatic='A', Manual='M'
    double lat;         // [deg]
    double lon;         // [deg]
    double alt;         // [ft]
    double heading;     // [deg]
    double hor_speed;   // [kn]
    double vert_speed;  // [ft/min]
 };

struct Broad_info {
    int sd;
    struct sockaddr_in addr;
};

struct CoordXYZ {
    double X;
    double Y;
    double Z;
};

typedef struct Message {
    uint64_t ID;
    struct CoordXYZ AP;
    struct CoordXYZ AV;
    char TS[STATUS_SIZE];
    uint64_t IH;
    char R[STATUS_SIZE];
    double RV;
    uint32_t CS;
} message;

// aircraft velocity
typedef struct VEL {
    double x;
    double y;
    double z;
    double north;
    double east;
    double up;
} VEL;

// aircraft position
typedef struct POS {
    // cartesian coordinates
    double x;
    double y;
    double z;
    // geodetic coordinates
    double lat;
    double lon;
    double alt;
} POS;

typedef struct ENU {
    double e;
    double n;
    double u;
} ENU;

// aircraft structure
typedef struct AC {
    uint64_t ID;
    char status[STATUS_SIZE];
    char resolution[STATUS_SIZE];
    uint64_t intruder;
    double resolution_value;
    POS pos;
    VEL vel;
    double heading;
    double hor_speed;
    double tau;
    struct timeval time_msg;
} AC_t;

typedef struct AC_list {
    AC_t AC;
    struct AC_list * next;
} AC_list_t;


// Global variables-------------------------------------------------------------

struct Inputs inputs;
int port;
char address[STATUS_SIZE];

AC_list_t * head;   //List of other aircrafts

int allow_dynamics, allow_tcas,  range_mode;
int exiting, receiving, sending;
int desired_ROC;
double range;

// used to play alert sounds
int alert_TA, alert_RA, silent;


// Functions--------------------------------------------------------------------

// In dyn_broad.c
void startup_menu (void);
int verify_address(char*host);
struct Broad_info initialize_Broadcast(void);
message get_msg(AC_t AC_own);
int dynamics(struct Broad_info broad);

// In coord.c
POS xyz2llh(POS pos);
POS llh2xyz(POS pos);
VEL vel_xyz2enu(POS pos, VEL vel);
VEL vel_enu2xyz(POS pos, VEL vel);
ENU xyz2enu(POS pos_ac, POS pos_int);

// In aircraft.c
AC_t initialize_AC_own(void);
void follow_route(double heading, double speed, double alt);
AC_t update_aircraft_own(AC_t ac);
AC_t update_aircraft_others(AC_t ac);
void print_AC(AC_t ac);
void print_list(void);
int n_algarisms(int n);
uint32_t convert_string(char * string);
uint32_t convert_number(double num);
uint32_t xor(uint32_t a, uint32_t b);
uint32_t checksum(message msg);

// In listener.c
int initialize_listener(void);
message initialize_msg(message msg);
void print_msg(message msg);
AC_t msg_to_ac(message msg);
void insert_AC_in_list(AC_t ac_intruder);
void *listener(void *vargp);

// In tcas.c
void *tcas(void *vargp);
void get_table_line(double alt, float * table_line);
int check_RA_TA_zone(AC_t ac, float * table_line, double * tau_H);
int dot_product(double * x, double * y, int n);
void climb();
void descend();
void climb_now();
void descend_now();
void remove_distant_ac(AC_list_t * head);

// In graphics.c
void *graph(void *vargp);
float deg2rad(int a);
void g2_display(int *d);
void g2_init(int *vd, int *d);
void g2_intruder (int *d, ENU enu, int type, int pos, int mov, double ac_heading,uint64_t int_id);
void switch_1(int * d, float posx, float posy, char* title);
void switch_2(int * d, float posx, float posy, char* title);
void RA_disp(int * d, double desired_h);
float button(int * d, int* state, float posx,float posy, char* sign);
void button2(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title);
void button3(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title);
void button4(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title);
void delay(unsigned int milliseconds);

// In sound.c
void *sound(void *vargp);

#endif //SAI_GENERAL_H

