/*-------------------------------------------------------
	Instituto Superior Técnico - 1st Semester 17/18

	Integrated Avionic Systems


	FILE coord.c
	Contains all of the functions that convert position and velocities into different coordinte systems


	Authors:
		Luís Bernardo, 78267
		Marta Marques, 78289
		Rafael Borges, 78796

	Date: 
    05-02-2018
---------------------------------------------------------*/

#include "general.h"

/*
  Function: xyz2llh
  Converts cartesian (WGS84) to geodetic coordinates
  Method used in Navigation Systems course
*/
POS xyz2llh(POS pos)
{
  // WGS84 constants
  double a = 6378137;
  double f = 1/298.257223563;

  // parameters
  double b = a*(1-f);
  double e2 = 1-pow((b/a),2);
  double e2l = pow((a/b),2)-1;
  double p = sqrt(pow(pos.x,2)+pow(pos.y,2));
  double beta = atan2(a*pos.z, (b*p));

  // latitude
  pos.lat = atan2(pos.z+e2l*b*pow(sin(beta),3), p-e2*a*pow(cos(beta),3));

  // longitude
  if (pos.y >= 0)
    pos.lon = PI/2 - 2*atan2(pos.x, (sqrt(pow(pos.x,2)+pow(pos.y,2))+pos.y));
  else
    pos.lon = -PI/2 + 2*atan2(pos.x, (sqrt(pow(pos.x,2)+pow(pos.y,2))-pos.y));


  if(pos.lat<-90)
    pos.lat=-180-pos.lat;
  if(pos.lat>90)
    pos.lat=180-pos.lat;

  if(pos.lon<-180)
    pos.lon=pos.lon+360;
  if(pos.lon>180)
    pos.lon=pos.lon-360;

  // altitude
  double RN = a/(sqrt(1-f*(2-f)*pow(sin(pos.lat),2)));
  pos.alt = p*cos(pos.lat)+pos.z*sin(pos.lat)-pow(a,2)/RN;

  return pos;
}

/*
  Function: llh2xyz
  Converts geodetic to cartesian coordinates (WGS84)
  Method used in Navigation Systems course
*/
POS llh2xyz(POS pos)
{
  // WGS84 constants
  double a = 6378137;
  double f = 1/298.257223563;

  double RN = a/(sqrt(1-f*(2-f)*pow(sin(pos.lat),2)));

  // x, y, z in meters
  pos.x = (RN+pos.alt)*cos(pos.lat)*cos(pos.lon);
  pos.y = (RN+pos.alt)*cos(pos.lat)*sin(pos.lon);
  pos.z = (pow((1-f),2)*RN+pos.alt)*sin(pos.lat);

  return pos;
}

/*
  Function: vel_enu2xyz
  Converts velocities in ENU into WGS84 XYZ
*/
VEL vel_enu2xyz(POS pos, VEL vel) {

  vel.x = -sin(pos.lon)*vel.east-cos(pos.lon)*sin(pos.lat)*vel.north+cos(pos.lon)*cos(pos.lat)*vel.up;
  vel.y = cos(pos.lon)*vel.east-sin(pos.lon)*sin(pos.lat)*vel.north+sin(pos.lon)*cos(pos.lat)*vel.up;
  vel.z = cos(pos.lat)*vel.north+sin(pos.lat)*vel.up;

  return vel;
}

/*
  Function: vel_xyz2enu
  Converts velocities in WGS84 XYZ into ENU
*/
VEL vel_xyz2enu(POS pos, VEL vel) {

  vel.east = -sin(pos.lon)*vel.x+cos(pos.lon)*vel.y;
  vel.north =-cos(pos.lon)*sin(pos.lat)*vel.x-sin(pos.lon)*sin(pos.lat)*vel.y+cos(pos.lat)*vel.z;
  vel.up = cos(pos.lon)*cos(pos.lat)*vel.x+sin(pos.lon)*cos(pos.lat)*vel.y+sin(pos.lat)*vel.z;

  return vel;
}

/*
  Function: xyz2enu
  Converts positions in WGS84 XYZ into ENU
*/
ENU xyz2enu(POS pos_ac, POS pos_int)
{
    // WGS84 constants

    struct ENU enu;

    double x = pos_int.x-pos_ac.x;
    double y = pos_int.y-pos_ac.y;
    double z = pos_int.z-pos_ac.z;
    double x1, y1, z1;

    //multiplication by z-rot matrix
    x1=x*cos(pos_ac.lon+PI/2)+y*sin(pos_ac.lon+PI/2);
    y1=-x*sin(pos_ac.lon+PI/2)+y*cos(pos_ac.lon+PI/2);
    z1=z;

    enu.e=x1;
    enu.n=y1*cos(PI/2-pos_ac.lat)+z1*sin(PI/2-pos_ac.lat);
    enu.u=-y1*sin(PI/2-pos_ac.lat)+z1*cos(PI/2-pos_ac.lat);

    return enu;
}
