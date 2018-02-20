/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE aircraft.c
	Contains all of the functions related to the aircraft that were used in different threads


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
    05-02-2018
---------------------------------------------------------*/

#include "general.h"

/*
  Function: initialize_AC_own
  Initializes all of the components of the AC_t structure
*/
AC_t initialize_AC_own(void)
{
  AC_t AC_own;

  AC_own.ID = inputs.ID;
  strcpy(AC_own.status,"CLEAR");
  strcpy(AC_own.resolution,"NONE");
  AC_own.intruder = 0;
  AC_own.resolution_value = 0;
  AC_own.pos.lat = inputs.lat*PI/180;     // [rad]
  AC_own.pos.lon = inputs.lon*PI/180;     // [rad]
  AC_own.pos.alt = inputs.alt*0.3048;     // [m]
  AC_own.pos = llh2xyz(AC_own.pos);
  AC_own.vel.x = 0;                       //[m/s]
  AC_own.vel.y = 0;                       //[m/s]
  AC_own.vel.z = 0;                       //[m/s]
  AC_own.vel.north = 0;                   //[rad/s]
  AC_own.vel.east = 0;                    //[rad/s]
  AC_own.vel.up = 0;                      //[m/s]
  AC_own.heading = inputs.heading;        //[deg]
  AC_own.hor_speed = inputs.hor_speed*0.5144;    //[m/s]
  AC_own.tau = 0;                         //[s]

  return AC_own;
}

/*
  Function: follow_route
  Computes the necessary velocities to follow desired route
  input units: heading in rad, speed in knots, alt in ft
*/
void follow_route(double heading, double speed, double alt)
{
  // Change the first element of he AC list, which corresponds to the data of our aircraft
  AC_t *own_ac = &head->AC;
  double vel_up, vel_east, vel_north;

  // heading and horizontal speed can only change in the manual mode
  if(inputs.mode=='M') {
    own_ac->heading=heading;
    own_ac->hor_speed=speed*0.5144;
  }

  //Automatic mode
  if(inputs.mode=='A'){
    // if it's not in the process of resolution
    if (strcmp(own_ac->status,"RESOLVING")!=0 && strcmp(own_ac->status,"RETURNING")!=0) {
      // To guarantee that the aircraft follows the desired route
      if(fabs(own_ac->pos.alt/0.3048 - alt) > ALT_ERROR) {
        if (own_ac->pos.alt/0.3048 > alt) {
          vel_up = ROC_DESCEND2ROUTE;
        } else {
          vel_up = ROC_CLIMB2ROUTE;
        }
      }
      else {
          vel_up = 0;
        }
    }
    else {
        vel_up = own_ac->vel.up;
    }
  }
  else    //Manual mode
      vel_up=inputs.vert_speed*0.3048/60;

  // Compute the velocities in ENU coordinate system
  if (own_ac->heading == 90) {
    vel_east = own_ac->hor_speed;
    vel_north = 0;
  }
  else if (own_ac->heading == 270) {
    vel_east = -own_ac->hor_speed;
    vel_north = 0;
  }
  else {
    if (own_ac->heading > 90 && own_ac->heading < 270) {
      vel_north = -sqrt(pow(own_ac->hor_speed,2))/(1+pow((tan(own_ac->heading*PI/180)),2));
    }
    else {
      vel_north = sqrt(pow(own_ac->hor_speed,2))/(1+pow((tan(own_ac->heading*PI/180)),2));
    }
    if (own_ac->heading >= 0 && own_ac->heading <= 180) {
      vel_east = fabs(tan(own_ac->heading*PI/180)*vel_north);
    }
    else {
      vel_east = -fabs(tan(own_ac->heading*PI/180)*vel_north);
    }
  }
  double R = sqrt(pow(own_ac->pos.x,2)+pow(own_ac->pos.y,2)+pow(own_ac->pos.z,2));; // radius
  vel_north = atan2(vel_north, R);
  vel_east = atan2(vel_east, R);

  // Convert velocities to WGS84 XYZ coordinate system
  POS new_pos;
  new_pos.lat = own_ac->pos.lat + vel_north;
  new_pos.lon = own_ac->pos.lon + vel_east;
  new_pos.alt = own_ac->pos.alt + vel_up;
  new_pos = llh2xyz(new_pos);

  own_ac->vel.x = new_pos.x - own_ac->pos.x;
  own_ac->vel.y = new_pos.y - own_ac->pos.y;
  own_ac->vel.z = new_pos.z - own_ac->pos.z;

  own_ac->vel.north = vel_north;
  own_ac->vel.east = vel_east;
  own_ac->vel.up = vel_up;
}

/*
  Function: update_aircraft_own
  Updates the position of our aircraft
*/
AC_t update_aircraft_own(AC_t ac)
{
  ac.pos.x += ac.vel.x;
  ac.pos.y += ac.vel.y;
  ac.pos.z += ac.vel.z;

  // convert position into geodetic coordinates
  ac.pos = xyz2llh(ac.pos);

  return ac;
}

/*
  Function: update_aircraft_others
  Updates the position of our aircraft
*/
AC_t update_aircraft_others(AC_t ac)
{
  // convert position into geodetic coordinates
  ac.pos = xyz2llh(ac.pos);

  // computes velocities in ENU coordinate system
  POS pos1;
  pos1.x = ac.pos.x + ac.vel.x;
  pos1.y = ac.pos.y + ac.vel.y;
  pos1.z = ac.pos.z + ac.vel.z;
  pos1 = xyz2llh(pos1);

  ac.vel.north = pos1.lat-ac.pos.lat;
  ac.vel.east = pos1.lon-ac.pos.lon;
  ac.vel.up = pos1.alt-ac.pos.alt;

  // heading
  ac.heading = atan2(ac.vel.east, ac.vel.north)*180/PI;
  if (ac.heading < 0)
      ac.heading += 360; // to have positive heading

  // horizontal speed
  double R = sqrt(pow(ac.pos.x,2)+pow(ac.pos.y,2)+pow(ac.pos.z,2));
  ac.hor_speed = sqrt(pow(tan(ac.vel.east)*R, 2)+pow(tan(ac.vel.north)*R, 2));

  return ac;
}

/*
  Function: print_AC
  Auxiliary function; prints the aircraft data
*/
void print_AC(AC_t ac){

  printf("\n*-------AC-------*\n");
  printf("ID: %ld\n", ac.ID);
  printf("POS xyz: %f %f %f\n", ac.pos.x, ac.pos.y, ac.pos.z);
  printf("POS : %f %f %f\n", ac.pos.lat*180/PI, ac.pos.lon*180/PI, ac.pos.alt/0.3048);
  printf("VEL xyz: %f %f %f\n", ac.vel.x, ac.vel.y, ac.vel.z);
  printf("VEL : %f %f %f\n", ac.vel.north*180/PI, ac.vel.east*180/PI, ac.vel.up);
  printf("TS: %s\n", ac.status);
  printf("IH: %ld\n", ac.intruder);
  printf("R: %s\n", ac.resolution);
  printf("RV: %f\n", ac.resolution_value);
  printf("Heading: %f\n", ac.heading);
  printf("H_speed: %f\n", ac.hor_speed);
  printf("tau: %f\n", ac.tau);
}

/*
  Function: print_list
  Auxiliary function; prints all of the elements of the list
*/
void print_list(void) {
  AC_list_t * current = head;

  printf("\n*******************Print List**********************\n");

  while (current != NULL) {
    print_AC(current->AC);
    current = current->next;
  }
  printf("\n");
}

/*
  Function: n_algarisms
  ????
*/
int n_algarisms(int n)
{
	int count = 0;

	for (; n != 0; count++)
		n /= 10;

	return count;
}

/*
  Function: convert_string
  Converts string for checksum
*/
uint32_t convert_string(char * string)
{
	uint32_t res, aux;

	if (string == NULL)
		return -1;

	res = string[0];
	aux = n_algarisms(string[0]);

	for (int i=1; i<strlen(string); i++){
		res = res + string[i]*pow(10, aux);
		aux += n_algarisms(string[i]);
	}

	return res;
}

/*
  Function: convert_number
  Converts number for checksum (ceil)
*/
uint32_t convert_number(double num)
{
  return ceil(num);
}

/*
  Function: xor
  Performs the XOR operation
*/
uint32_t xor(uint32_t a, uint32_t b)
{
	return a^b;
}

/*
  Function: checksum
  Calculates the checksum of a message
*/
uint32_t checksum(message msg)
{
  uint32_t result, check1, check2, check3, check4, check5,
            check6, check7, check8, check9, check10, check11;

  // converting each parameter of the message
  check1 = convert_number(msg.ID);
  check2 = convert_number(msg.AP.X);
  check3 = convert_number(msg.AP.Y);
  check4 = convert_number(msg.AP.Z);
  check5 = convert_number(msg.AV.X);
  check6 = convert_number(msg.AV.Y);
  check7 = convert_number(msg.AV.Z);
  check8 = convert_string(msg.TS);
  check9 = convert_number(msg.IH);
  check10 = convert_string(msg.R);
  check11 = convert_number(msg.RV);

  // XOR of message parameter by parameter
  result = xor(check1, check2);
  result = xor(result, check3);
  result = xor(result, check4);
  result = xor(result, check5);
  result = xor(result, check6);
  result = xor(result, check7);
  result = xor(result, check8);
  result = xor(result, check9);
  result = xor(result, check10);
  result = xor(result, check11);

  return result;
}
