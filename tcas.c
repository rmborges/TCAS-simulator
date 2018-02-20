/*-------------------------------------------------------
  Instituto Superior Técnico - 1st Semester 17/18

  Integrated Avionic Systems


  FILE tcas.c
  Contains the functions related to the TCAS algorithm
  
  IMPORTANT: The sounds used do not belong to us! 
  They are freeware and can be found in:
  https://library.avsim.net/search.php?SearchTerm=tcas&CatID=fsxgau&Go=Search
  The files are in tcasii_v1.0.zip and the creator 
  of that file is Dietmar Loleit


  Authors:
    Luís Bernardo, 78267
    Marta Marques, 78289
    Rafael Borges, 78796

  Date: 
    05-02-2018
---------------------------------------------------------*/

#include "general.h"

/*
    Function: tcas
    Runs the TCAS algorithm
*/
void *tcas(void *vargp){

    double tau, prev_tau;
    AC_list_t * pt;
    int zone;

    // table line with altitude, Tau, DMOD and ZTHR
    float table_line[8];

    // Program stays in a cycle until the user presses CTRL+C
    while(exiting == 0){

      //When a message is received
      if(allow_tcas==1){

        //Check altitude index
        get_table_line(head->AC.pos.alt,table_line);

        // assuming no intruder(s)
        head->AC.intruder = 0;
        // assuming no possible collisions
        tau = 0;
        prev_tau = 0;

        //No aircrafts detected
        if(head->next==NULL)
          continue;

        // Go through all aircrafts detected
        for(pt = head->next; pt!= NULL; pt = pt->next) {
            // updates the data of the intruder aircraft
            pt->AC=update_aircraft_others(pt->AC);
            // Checks if its in RA (zone=1) ou TA zone (zone=2)
            zone=check_RA_TA_zone(pt->AC, table_line, &tau);
            pt->AC.tau=tau;

            // resolution advisory
            if (zone==1)
            {
                // In case there is more than 1 intruder
                // Do reslution with the one with the smallest positive tau
                if(((fabs(tau) < fabs(prev_tau)) && tau>0)|| (prev_tau == 0)|| ((fabs(tau) < fabs(prev_tau)) && tau<0 && prev_tau<0)) {
                    head->AC.intruder = pt->AC.ID;

                    prev_tau = tau;
                }
                else {
                    printf("Confliting resolutions!");
                    printf("Previous res has lower tau\n");
                    continue;
                }

                // our A/C with lower altitude than the other
                if (pt->AC.pos.alt > head->AC.pos.alt) {
                    // our A/C climbing ou leveled
                    // our A/C will descend
                    if (pt->AC.vel.up >= 0) {
                        // other A/C not yet resolving
                        if (strcmp(pt->AC.status,"RESOLVING") != 0)  {
                            descend();
                        }
                        // other A/C resolving
                        else {
                            // other A/C climbing
                            if (strcmp(pt->AC.resolution, "CLIMB") == 0) {
                                descend();
                            }
                            // other A/C descending (conflict)
                            else {
                                // our A/C with priority
                                if (head->AC.ID < pt->AC.ID) {
                                    descend();
                                }
                                // other A/C with priority
                                else {
                                    climb();
                                }
                            }
                        }
                    }

                    // other A/C descending and our A/C descending or mantaining altitude
                    if (pt->AC.vel.up<0 && head->AC.vel.up<=0) {
                        // other A/C not yet resolving
                        if (strcmp(pt->AC.status, "RESOLVING") != 0) {
                            // our A/C will descend at high rate
                            descend_now();
                        }
                        // other A/C resolving
                        else {
                            // other A/C climbing
                            if (strcmp(pt->AC.status, "CLIMB") == 0) {
                                descend();
                            }
                            // other A/C descending (conflict)
                            else {
                                // our A/C with priority
                                if (head->AC.ID < pt->AC.ID) {
                                    descend();
                                }
                                // other A/C with priority
                                else {
                                    climb();
                                }
                            }
                        }
                    }
                }
                // our A/C with higher altitude than the other
                if (pt->AC.pos.alt < head->AC.pos.alt) {
                    // other A/C climbing
                    if (pt->AC.vel.up > 0) {
                        // other A/C not yet resolving
                        if (strcmp(pt->AC.status, "RESOLVING") != 0) {
                            // our A/C will climb at high rate
                            climb_now();
                        }
                        // other A/C is resolving
                        else {
                            // other A/C descending
                            if (strcmp(pt->AC.resolution,"DESCEND") == 0) {
                                climb();
                            }
                            // other A/C climbing (conflict)
                            else {
                                // our A/C with priority
                                if (head->AC.ID < pt->AC.ID) {
                                    climb();
                                }
                                // other A/C with priority
                                else {
                                    descend();
                                }
                            }
                        }
                    }
                    // our and other A/C descending or mantaining altitude
                    if (pt->AC.vel.up<=0 && head->AC.vel.up<=0) {
                        // other A/C not yet resolving
                        if (strcmp(pt->AC.status,"RESOLVING") != 0) {
                            climb();
                        }
                        // other A/C resolving
                        else {
                            // other A/C descending
                            if (strcmp(pt->AC.resolution,"DESCEND") == 0) {
                                climb();
                            }
                            // other A/C climbing (conflict)
                            else {
                                // our A/C with priority
                                if (head->AC.ID < pt->AC.ID) {
                                    climb();
                                }
                                // other A/C with priority
                                else {
                                    descend();
                                }
                            }
                        }
                    }
                }
                // our and other A/C with very close altitudes
                if (fabs(pt->AC.pos.alt - head->AC.pos.alt)<=ALT_ERROR*0.3048) {
                    // other A/C not yet resolving
                    if (strcmp(pt->AC.status, "RESOLVING") != 0) {
                        // our A/C with priority
                        if (head->AC.ID < pt->AC.ID) {
                            climb();
                        }
                        // other A/C with priority
                        else {
                            descend();
                        }
                    }
                    // other A/C resolving
                    else {
                        // other A/C descending
                        if (strcmp(pt->AC.resolution, "DESCEND") == 0) {
                            climb();
                        }
                        // other A/C climbing
                        else {
                            descend();
                        }
                    }
                }


            }
            // traffic advisory
            else if (zone==2)
            {
                // Update the structure parameters
                head->AC.intruder = pt->AC.ID;

                strcpy(head->AC.status, "ADVISORY");
                strcpy(head->AC.resolution,"NONE");
                head->AC.resolution_value=0;
                desired_ROC=0;
            }
        }

        // no intruders found
        if (head->AC.intruder == 0) {

            // our A/C returning (not yet on desired altitude)
            if (fabs(head->AC.pos.alt/0.3048 - inputs.alt) > ALT_ERROR) {

              strcpy(head->AC.status,"RETURNING");
              strcpy(head->AC.resolution,"NONE");
              head->AC.resolution_value=0;

              // our A/C above desired altitude
              if (head->AC.pos.alt/0.3048 > inputs.alt) {
                  // If mode A -> descend to go back to the desired height
                  if(inputs.mode=='A')
                      head->AC.vel.up = ROC_DESCEND2ROUTE;

                  // If mode M -> Advise to descend
                  desired_ROC=ROC_DESCEND2ROUTE/0.3048*60;
              }
              // our A/C below desired altitude
              else{
                  if(inputs.mode=='A')
                      head->AC.vel.up = ROC_CLIMB2ROUTE;

                  desired_ROC=ROC_CLIMB2ROUTE/0.3048*60;
              }
          }
          // our A/C already with desired altitude
          else {
              strcpy(head->AC.status, "CLEAR");
              strcpy(head->AC.resolution,"NONE");
              head->AC.resolution_value=0;
              desired_ROC=0;
              head->AC.vel.up = 0;
          }
      }

      // remove distant A/C from list
      remove_distant_ac(head);

      print_list();

      // to wait until new message received
      allow_tcas=0;

    }
  }

  return NULL;
}


/*
  Function: get_table_line
  Checks the altitude index and gets the corresponding table parameters
*/
void get_table_line(double alt, float * table_line) {

    float  table_alt[] = {1000,2350,5000,10000,20000,42000, 100000};
    int table_tau_TA[] = {20,25,30,40,45,48,48};
    int table_tau_RA[] = {10,15,20,25,30,35,35};
    float table_dmod_TA[] = {0.3,0.33,0.48,0.75,1,1.30, 1.30};
    float table_dmod_RA[] = {0.1,0.2,0.35,0.55,0.8,1.1, 1.1};
    int table_zthr_TA[] = {850,850,850,850,850,850,1200};
    int table_zthr_RA[] = {600,600,600,600,600,700,800};
    int table_alim[] = {300,300,300,350,400,600,700};

    int index;

    // Obtain index
    alt = alt/0.3048;   //Convert to ft

    if (alt <= table_alt[0])
        index=0;

    int i;

    for (i=0; i<5; i++) {
        if ((alt > table_alt[i]) && (alt < table_alt[i+1]))
            index = i+1;
    }

    // Assign the right parameters for the current altitude
    index = i;
    table_line[0] = table_alt[index];
    table_line[1] = table_tau_TA[index];
    table_line[2] = table_tau_RA[index];
    table_line[3] = table_dmod_TA[index];
    table_line[4] = table_dmod_RA[index];
    table_line[5] = table_zthr_TA[index];
    table_line[6] = table_zthr_RA[index];
    table_line[7] = table_alim[index];
}

/*
  Function: check_RA_TA_zone
  Computes tau and checks TA (return 2) or RA (return 1) zones
*/
int check_RA_TA_zone(AC_t ac, float * table_line, double * tau_H) {

  // Get the intermediate altitude
  double new_h = (head->AC.pos.alt+ac.pos.alt)/2;

  POS pos1, pos2;

  pos1.lat = head->AC.pos.lat;
  pos1.lon = head->AC.pos.lon;
  pos1.alt = new_h;
  pos1 = llh2xyz(pos1);

  pos2.lat = ac.pos.lat;
  pos2.lon = ac.pos.lon;
  pos2.alt = new_h;
  pos2 = llh2xyz(pos2);

  //Relative positions in the intermediate plan
  double pos_H_diff[]={pos1.x-pos2.x, pos1.y-pos2.y, pos1.z-pos2.z};

  // Compute the horizontal speed for both aircrafts 
  
  // our aircraft
  double R1 = sqrt(pow(pos1.x,2)+pow(pos1.y,2)+pow(pos1.z,2));
  VEL vel1, vel2;
  vel1.east = head->AC.vel.east*R1;
  vel1.north = head->AC.vel.north*R1;
  vel1.up = 0;
  vel1 = vel_enu2xyz(pos1,vel1);  // Convert to XYZ

  // intruder's aircraft
  double R2 = sqrt(pow(pos2.x,2)+pow(pos2.y,2)+pow(pos2.z,2));
  vel2.east = ac.vel.east*R2;
  vel2.north = ac.vel.north*R2;
  vel2.up = 0;
  vel2 = vel_enu2xyz(pos2,vel2);  // Convert to XYZ

  //Relative velocities in the intermediate plan
  double vel_H_diff[]={vel1.x-vel2.x, vel1.y-vel2.y, vel1.z-vel2.z};

  //Compute range tau
  if (dot_product(pos_H_diff, vel_H_diff,3) != 0)
    *tau_H = (pow(table_line[4],2)-dot_product(pos_H_diff, pos_H_diff, 3))/dot_product(pos_H_diff, vel_H_diff, 3);
  else
    *tau_H = 100000;   //Value to be discarded


  // Check for RA zone---------------------------------------------------------------------
  // Horizontal RA condition
  int RA_H;
  if((sqrt(dot_product(pos_H_diff, pos_H_diff,3)) <= table_line[4]) ||
        ((dot_product(pos_H_diff, vel_H_diff,3)<0) && (*tau_H>0) && (fabs(*tau_H)<=table_line[2])))
    RA_H = 1;
  else
    RA_H = 0;

  // Relative vertical position
  double pos_V_diff = head->AC.pos.alt - ac.pos.alt;
  // Relative vertical velocity 
  double vel_V_diff = head->AC.vel.up - ac.vel.up;

  // Compute vertical tau
  double tau_V;
  if(vel_V_diff != 0)
    tau_V = pos_V_diff/vel_V_diff;
  else
    tau_V = 100000;   //Value to be discarded


  // Vertical RA condition
  int RA_V;
  if((fabs(pos_V_diff) <= table_line[6]) ||
        ((pos_V_diff*vel_V_diff<0) && (fabs(tau_V)<=table_line[2]) && (tau_V>0)))
    RA_V = 1;
  else
    RA_V = 0;

  // RA if vertical and horizontal RA hold
  if((RA_V==1) && (RA_H==1))
    return 1;


  // Check for TA zone---------------------------------------------------------------------
  // Horizontal TA
  int TA_H;
  if((sqrt(dot_product(pos_H_diff, pos_H_diff,3)) <= table_line[3]) ||
        ((dot_product(pos_H_diff, vel_H_diff,3)<0) && (*tau_H>0) && (fabs(*tau_H)<=table_line[1])))
        TA_H = 1;
  else
    TA_H = 0;

  // Vertical TA
  int TA_V;
  if((fabs(pos_V_diff) <= table_line[5]) ||
        ((pos_V_diff*vel_V_diff<0) && (tau_V>0) && (fabs(tau_V)<=table_line[5])))
    TA_V = 1;
  else
    TA_V = 0;

  if((TA_V==1) && (TA_H==1) && (strcmp(head->AC.status, "RESOLVING")!=0) && (strcmp(head->AC.status, "RETURNING")!=0))
    return 2;

  return 0;

}


/*
  Function: dot_product
  Does the dot product of 2 vectors of size n
*/
int dot_product(double * x, double * y, int n) {
    double res=0;
    for (int i=0; i<n; i++) {
      res += x[i]*y[i];
    }
    return res;
}

/*
  Function: climb
  Implements the climb in a resolution
*/
void climb()
{
  strcpy(head->AC.status,"RESOLVING");

  if(inputs.mode=='A') {
    strcpy(head->AC.resolution, "CLIMB");
    head->AC.resolution_value = fabs(ROC_CLIMB);
    head->AC.vel.up = ROC_CLIMB;
  }
  else
    strcpy(head->AC.resolution, "NONE");    
  
   desired_ROC=ROC_CLIMB/0.3048*60;
}

/*
  Function: descend
  Implements the descend in a resolution
*/
void descend()
{
  strcpy(head->AC.status,"RESOLVING");

  if(inputs.mode=='A') {
    strcpy(head->AC.resolution, "DESCEND");
    head->AC.resolution_value = fabs(ROC_DESCEND);
    head->AC.vel.up = ROC_DESCEND;
  }
  else
    strcpy(head->AC.resolution, "NONE");    

  desired_ROC=ROC_DESCEND/0.3048*60;
}

/*
  Function: climb_now
  Implements a faster climb in a resolution
*/
void climb_now()
{
  strcpy(head->AC.status,"RESOLVING");

  if(inputs.mode=='A') {
    strcpy(head->AC.resolution, "CLIMB");
    head->AC.resolution_value = fabs(ROC_CLIMB_NOW);
    head->AC.vel.up = ROC_CLIMB_NOW;
  }
  else
    strcpy(head->AC.resolution, "NONE");    

  desired_ROC=ROC_CLIMB_NOW/0.3048*60;
}


/*
  Function: descend_now
  Implements a faster descend in a resolution
*/
void descend_now()
{
  strcpy(head->AC.status,"RESOLVING");  

  if(inputs.mode=='A') {
    strcpy(head->AC.resolution, "DESCEND");
    head->AC.resolution_value = fabs(ROC_DESCEND_NOW);
    head->AC.vel.up = ROC_DESCEND_NOW;
  }
  else
    strcpy(head->AC.resolution, "NONE");    

  desired_ROC=ROC_DESCEND_NOW/0.3048*60;
}


/*
  Function: remove_distant_ac
  Removes distant A/Cs ou A/Cs that haven't transmited for more than 10 seconds
*/
void remove_distant_ac(AC_list_t * head)
{
  AC_list_t * previous = NULL;
  struct timeval t;

  // Get current time
  gettimeofday(&t, NULL);

  // Go hrough all elemnts of the list
  for(AC_list_t * pt = head; pt!= NULL; pt = pt->next) {

    // Ignore our own A/C
    if(pt->AC.ID == head->AC.ID) {
      previous = pt;
      continue;
    }

    // A/C not in 5min range or not transmitted for more than 10sec
    if ((pt->AC.tau > 600 && pt->AC.ID!=inputs.ID) || (t.tv_sec - pt->AC.time_msg.tv_sec) > 10){

        // remove A/C (pt->AC)
        previous->next = pt->next;
        free(pt);
        pt=previous->next;
        
        if(!pt)
          break;
    }
    else {
        previous = pt;
    }
  }
}