/*-------------------------------------------------------
  Instituto Superior Técnico - 1st Semester 17/18

  Integrated Avionic Systems


  	FILE sound.c
  	"Contains the functions related to sound"
	
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
    Function: sound
    Handles the sound alerts thread
*/
void *sound(void *vargp)
{
    libvlc_instance_t *inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    char prev_status[STATUS_SIZE] = "CLEAR";
    char prev_resolution[STATUS_SIZE] = "NONE";
    char alert_sound[STATUS_SIZE] = "";

    // run until program closed
    while (exiting == 0) {

        // load the engine
        inst = libvlc_new(0, NULL);

        // no new status and no new resolution -> no alert
        if ((strcmp(head->AC.status, prev_status) == 0) && (strcmp(head->AC.resolution, prev_resolution) == 0))
            continue;
        // play alerts
        else {
            // entrance on TA
            if (strcmp(head->AC.status, "ADVISORY") == 0)
                strcpy(alert_sound, "TrafficTraffic.wav");
            // returning
            else if (strcmp(head->AC.status, "RETURNING") == 0)
                strcpy(alert_sound, "ClearOfConflict.wav");
            // starting resolution (climb)
            else if (strcmp(head->AC.resolution, "CLIMB") == 0)
                strcpy(alert_sound, "CLIMBCLIMB.wav");
            // starting resolution (descend)
            else if (strcmp(head->AC.resolution, "DESCEND") == 0)
                strcpy(alert_sound, "DecentDecent.wav");
            else
                continue;
        }

        // keeping the previous status and resolution
        strcpy(prev_status, head->AC.status);
        strcpy(prev_resolution, head->AC.resolution);

        // create a file to play
        m = libvlc_media_new_path(inst, alert_sound);

        // create a media play playing environment
        mp = libvlc_media_player_new_from_media(m);

        // release the media now.
        libvlc_media_release(m);

        // play the media_player
        libvlc_media_player_play(mp);

        // play for 3 seconds
        sleep(3);

        // stop playing
        libvlc_media_player_stop(mp);

        libvlc_media_player_release(mp);
        libvlc_release(inst);
    }

    return NULL;
}
