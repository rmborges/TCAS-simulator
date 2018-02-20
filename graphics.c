/*-------------------------------------------------------
  Instituto Superior Técnico - 1st Semester 17/18

  Integrated Avionic Systems


  	FILE graphics.c
  	"Contains the functions related to the graphic displays"


  Authors:
    Luís Bernardo, 78267
    Marta Marques, 78289
    Rafael Borges, 78796

  Date: 
    05-02-2018
---------------------------------------------------------*/


#include "general.h"


/*
  Function: g2_init
  Opens and initializes a G2 window
*/
void g2_init (int *vd, int *d)
{

	*vd = g2_open_vd();
	*d = g2_open_X11(WINDOWX, WINDOWY);
	g2_clear(*vd);
	g2_attach(*vd,*d);
	g2_set_auto_flush(*vd,0);

	return;
}

/*
  Function: g2_intruder
  Handles intruder aircrafts
*/
void g2_intruder (int *d, ENU enu, int type, int pos, int mov, double ac_heading, uint64_t int_id)
{
	//Variables
	char aux[10];
	double points[8];
	double reason= 225/range;
	double dist=sqrt(pow(enu.n,2)+pow(enu.e,2));
	double ang=PI/2-atan2(enu.n,enu.e);


	if (dist>range){
		dist=220;
	}
	else{
		dist=dist*reason;
	}

	int x_center = dist*cos(ac_heading-ang)+550;
	int y_center = dist*sin(ac_heading-ang)+260;


	//Choosing Symbol for Intruder Aircraft

	type=2; //Unfilled Polygon	
	
	if( (dist<11111.11) && (fabs(pos)<1200) ){

		type=3; //Filled Polygon
	}

	if((strcmp(head->AC.status,"ADVISORY")==0) && (int_id== head->AC.intruder))
	{
		type=1; //Ýellow Circle

	}

	if ((strcmp(head->AC.status,"RESOLVING")==0)&& (int_id== head->AC.intruder))
	{
		type=0; //Red Square
	}

	switch(type) {

		case 0: //Red Square
			g2_pen(*d,19);
			g2_filled_rectangle(*d,x_center+5,y_center+5,x_center-5,y_center-5);
	    	break;

		case 1: //Ýellow Circle
			g2_pen(*d,22);
			g2_filled_circle(*d,x_center,y_center,6);
	    	break;

		case 2: //Unfilled Polygon
			g2_pen(*d,6);
			points[0] = x_center-5;
			points[1] = y_center+2;
			points[2] = x_center;
			points[3] = y_center+9;
			points[4] = x_center+5;
			points[5] = y_center+2;
			points[6] = x_center;
			points[7] = y_center-5;
			g2_polygon(*d, 4, points);
	    	break;

		case 3: //Filled Polygon
			g2_pen(*d,6);
			points[0] = x_center-5;
			points[1] = y_center+2;
			points[2] = x_center;
			points[3] = y_center+9;
			points[4] = x_center+5;
			points[5] = y_center+2;
			points[6] = x_center;
			points[7] = y_center-5;
			g2_filled_polygon(*d, 4, points);
	    	break;

		default :
			printf("Invalid intruder\n" );
	}


	//Arrow concerning movement of intruder (climbing or descending)
	if (mov< 0)
	{
		g2_line(*d,x_center+10,y_center-5,x_center+10,y_center+5);
		g2_filled_triangle(*d,x_center+10,y_center-5,x_center+7,y_center,x_center+13,y_center);
	}
	else if (mov>0)
	{
		g2_line(*d,x_center+10,y_center-5,x_center+10,y_center+5);
		g2_filled_triangle(*d,x_center+10,y_center+5,x_center+7,y_center,x_center+13,y_center);
	}

	//Intruder data above or below 
	if (pos>= 100)
	{
		sprintf(aux,"-%d",pos/100);
		g2_string(*d, x_center-7, y_center-18, aux);
	}
	else if (pos<=-100 && type<2)
	{
		sprintf(aux,"+%d",-pos/100);
		g2_string(*d, x_center-7, y_center+7, aux);
	}
	else if (pos<=-100 && type>=2)
	{
		sprintf(aux,"+%d",-pos/100);
		g2_string(*d, x_center-7, y_center+9, aux);
	}
	else if (pos<=100 && pos>=-100 && type>=2)
	{
		sprintf(aux,"00");
		g2_string(*d, x_center-7, y_center+9, aux);
	}
	else if (pos<=100 && pos>=-100 && type<2)
	{
		sprintf(aux,"00");
		g2_string(*d, x_center-7, y_center+7, aux);
	}

	//Intruder ID (introduced during demonstration)
	sprintf(aux,"ID: %lu",int_id);
	g2_string(*d, x_center-14, y_center-30, aux);	

	return;
}


/*
  Function: g2_display
  Builds the TCAS display
*/
void g2_display (int *d)
{

	char aux[100];
	double TAS=sqrt(pow(head->AC.hor_speed/0.5144,2) +pow(head->AC.vel.up/0.5144,2));
	double Hd=90+head->AC.heading;
	double ff=deg2rad(Hd);

	//Cleaning display square
	g2_pen(*d,0);
	g2_filled_rectangle(*d,0,0,910,520);


	//TCAS display ------------------------------------------

	//window division in displays
	g2_pen(*d,1);
	g2_filled_rectangle(*d,300,10,800,510);
	g2_filled_rectangle(*d,810,10,900,510);
	g2_pen(*d,0);
	g2_set_line_width(*d,1);
	g2_rectangle(*d,730,20,790,50);

	//Buttons and switches
	switch_1(d, 220, 441, "Range:");
	switch_2(d, 140, 441, "Fl Mode:");
	button2(d, 250,4000,-4000,20 ,470, "Cl/Desc Rate:");
	button3(d,15,360,0,20 ,430, "Heading:");
	button4(d, 25,600,100,20 ,390, "Hor Speed:");

	//Airplane figure
	g2_pen(*d,0);
	g2_set_line_width(*d,1);

	g2_filled_rectangle(*d,549,250,551,275);
	g2_filled_rectangle(*d,535,264,565,267);
	g2_filled_rectangle(*d,544,252,556,254);

	//Display circles
	g2_circle(*d,550,260,220); //100%
	g2_line(*d,550,260,550,480);
	double dashes[2] = {5,6};
	g2_set_dash(*d, 2, dashes);
	g2_circle(*d,550,260,165); //75%
	g2_circle(*d,550,260,110); //50%
	g2_circle(*d,550,260,55); //25%
	g2_set_dash(*d, 0, dashes);

	//RA DISPLAY---------------------------
	RA_disp(d, desired_ROC);

	//INTRUDERS HANDLING--------------------
	if(head->next){

		AC_list_t * current = head->next;
		ENU enu;
		int delta_alt;
		int g=0;

		//Iterating over list of aircrafts
		do{

			enu=xyz2enu(head->AC.pos, current->AC.pos);
			delta_alt= (head->AC.pos.alt - current->AC.pos.alt)/0.3048;
			g2_intruder(d,enu,1,delta_alt,current->AC.vel.up,ff,current->AC.ID);
			double dist=sqrt(pow(enu.n,2)+pow(enu.e,2));

			if (g<11){
				sprintf(aux,"- ID:%lu, H Dist(nm): %0.3f, V Dist(ft): %0.0f",current->AC.ID, dist*0.000539956803,fabs(delta_alt));
				g2_string(*d, 20, 255-20*g, aux);
				g=g+1;
			}
			else
			{
				sprintf(aux,"...");
				g2_string(*d, 20, 20, aux);
			}

			current=current->next;

		}while(current);


	}
	else
	{
			g2_pen(*d,1);
			sprintf(aux,"No aircrafts in sight!");
			g2_string(*d, 20, 250, aux);

	}


	//Danger Symbol!
	if(strcmp(head->AC.status,"RESOLVING")==0)
	{
		g2_pen(*d,19);
		g2_set_line_width(*d,3);
		g2_triangle(*d,310,20,350,20,330,55);
		g2_filled_circle(*d,330,26,1);
		g2_line(*d,330,31,330,45);
	}


	//HEADING DISPLAY----------------

	g2_pen(*d,0);
	g2_set_line_width(*d,1);
	g2_rectangle(*d,530,490,570,510);
	g2_triangle(*d,545,488,555,488,550,480);
	sprintf(aux,"%0.0f",Hd-90);//=%0f",*T);
	g2_string(*d, 545, 495, aux);

	for(int i = 0; i<360;i=i+5)
	{
		if(i%2!=0)
			g2_line(*d, 215*cos(deg2rad(Hd-i))+550, 215*sin(deg2rad(Hd-i))+260, 220*cos(deg2rad(Hd-i))+550, 220*sin(deg2rad(Hd-i))+260);
		else
			g2_line(*d, 210*cos(deg2rad(Hd-i))+550, 210*sin(deg2rad(Hd-i))+260, 220*cos(deg2rad(Hd-i))+550, 220*sin(deg2rad(Hd-i))+260);

		if(i%30 == 0)
		{
			sprintf(aux,"%d",i/10);
			g2_string(*d, 200*cos(deg2rad(Hd-i))+550, 200*sin(deg2rad(Hd-i))+260, aux);
		}

	}
	//North Pointer
	g2_pen(*d,19);
	g2_set_line_width(*d,3);
	g2_line(*d, 210*cos(ff)+550, 210*sin(ff)+260, 220*cos(ff)+550, 220*sin(ff)+260);





	//TEXT in DISPLAY----------------


	//White text
	g2_pen(*d,0);
	g2_set_line_width(*d,1);
	sprintf(aux,"TAS:");
	g2_string(*d, 310, 490, aux);
	sprintf(aux,"ALT:");
	g2_string(*d, 310, 475, aux);
	sprintf(aux,"nm");
	g2_string(*d, 765, 25, aux);


	//Green text
	g2_pen(*d,4);
	sprintf(aux,"%0.0f kn",TAS);
	g2_string(*d, 340, 490, aux);
	sprintf(aux,"%0.0f ft",head->AC.pos.alt/0.3048);
	g2_string(*d, 340, 475, aux);
	sprintf(aux,"TCAS SYSTEM");
	g2_string(*d, 700, 490, aux);
	sprintf(aux,"HDG");
	g2_string(*d, 500, 495, aux);
	sprintf(aux,"RA Display");
	g2_string(*d, 825, 493, aux);
	sprintf(aux,"(fpm)");
	g2_string(*d, 840, 478, aux);


	//Black Text
	g2_pen(*d,1);
	sprintf(aux,"POSITION:");
	g2_string(*d, 120, 420, aux);
	if(inputs.lat >=0)
		sprintf(aux," %0.3f N",head->AC.pos.lat/PI*180);
	else
		sprintf(aux," %0.3f S",-head->AC.pos.lat/PI*180);
	g2_string(*d, 180, 420, aux);
	if(inputs.lon >=0)
		sprintf(aux," %0.3f E",head->AC.pos.lon/PI*180);
	else
		sprintf(aux," %0.3f W",-head->AC.pos.lon/PI*180);
	g2_string(*d, 230, 420, aux);
	sprintf(aux,"ID:%lu", inputs.ID);
	g2_string(*d, 120, 400, aux);

	sprintf(aux,"TCAS INFORMATION");
	g2_string(*d, 20, 355, aux);
	g2_line(*d, 20, 351,140, 351);

	sprintf(aux,"AIRCRAFT LIST");
	g2_string(*d, 20, 275, aux);
	g2_line(*d, 20, 271,110, 271);

	sprintf(aux,"Resolution:");
	g2_string(*d, 20, 305, aux);
	g2_string(*d, 80, 305, head->AC.resolution);

	sprintf(aux,"Resolution Value: %0.0f", head->AC.resolution_value);
	g2_string(*d, 160, 305, aux);

	sprintf(aux,"Intruder ID: %lu", head->AC.intruder);
	g2_string(*d, 160, 325, aux);

	//Status value has same color as intruder symbol
	sprintf(aux,"Status:");
	g2_string(*d, 20, 325, aux);
	if(strcmp(head->AC.status,"ADVISORY")==0)
		g2_pen(*d,22);
	else if(strcmp(head->AC.status,"RESOLVING")==0)
		g2_pen(*d,19);
	g2_string(*d, 60, 325, head->AC.status);


	//Display borders
	g2_pen(*d,14);
	g2_set_line_width(*d,4);
	g2_rectangle(*d,300,10,800,510);
	g2_rectangle(*d,810,10,900,510);
	g2_rectangle(*d,10,380,290,510);
	g2_rectangle(*d,10,10,290,370);


	//Flushing window
	g2_flush(*d);
}

/*
  Function: deg2rad
  Converts from degrees to radians 
*/
float deg2rad(int a)
{
	float b;
	float c;

	c=a;
	b=c/180*PI;

	return b;
}

/*
  Function: RA_disp
  Handles the RA display
*/
void RA_disp(int * d, double desired_cr)
{

	char aux[10];

	double current_cr=head->AC.vel.up/0.00508;//inputs.vert_speed;

	double c_cr=20+(current_cr+4000)*55/1000;
	double d_cr=20+(desired_cr+4000)*55/1000;


	g2_pen(*d,19);
	g2_filled_rectangle(*d,820,20,845,d_cr);
	g2_pen(*d,7);
	g2_filled_rectangle(*d,820,d_cr-3,845,d_cr+3);
	g2_pen(*d,25);
	g2_filled_rectangle(*d,820,c_cr-2,845,c_cr+2);


	g2_pen(*d,0);


	for(int i=0;i<9;i++)
	{
		g2_set_line_width(*d,2);
		g2_line(*d,860,20+i*55,850,20+i*55);
		sprintf(aux,"%d",(-4000+i*1000));
		g2_string(*d, 863,17+i*55 , aux);

	}
	for(int i=0;i<8;i++)
	{

		g2_set_line_width(*d,1);
		g2_line(*d,850,47.5+i*55,860,47.5+i*55);


	}
	for(int i=0;i<16;i++)
	{
		g2_line(*d,850,32.5+i*27.5,855,32.5+i*27.5);

	}

}

/*
  Function: switch_1
  Handles Range switch
*/
void switch_1(int * d, float posx, float posy, char* title)
{
	double x,y;
	unsigned int stat;
	char str[10]="\0";


	g2_query_pointer(*d,&x,&y,&stat);

	//Allowed clicking positions
	if((x>=posx+5)&&(x<=posx+50)&&(y>=posy+5)&&(y<=posy+25)&&(stat==272)&&(range_mode == 0))
		range_mode = 1;
	if((x>=posx+5)&&(x<=posx+50)&&(y>=posy+25)&&(y<=posy+45)&&(stat==272)&&(range_mode == 1))
		range_mode = 0;


	g2_pen(*d,1);
	g2_filled_rectangle(*d, posx, posy, posx + 55,posy + 50);
	g2_string(*d, posx, posy+54, title);
	g2_pen(*d,0);

	//Far range - 20 nmi
	if(range_mode==1)
	{
		g2_set_font_size(*d,20);
		sprintf(str,"20");
		g2_string(*d, 740, 25, str);
		g2_set_font_size(*d,12);
		range=37038;
		sprintf(str,"FAR");
		g2_filled_rectangle(*d, posx+5, posy+5, posx + 50,posy + 25);
		g2_pen(*d,1);
		g2_string(*d, posx+17, posy+8 , str);

	}
	//Close range - 10 nmi
	if(range_mode==0)
	{
		g2_set_font_size(*d,20);
		sprintf(str,"10");
		g2_string(*d, 740, 25, str);
		g2_set_font_size(*d,12);
		range=18520;
		sprintf(str,"CLOSE");
		g2_filled_rectangle(*d, posx+5, posy+25, posx + 50,posy + 45);
		g2_pen(*d,1);
		g2_string(*d, posx+10, posy+28 , str);

	}


	return;
}

/*
  Function: switch_2
  Handles Mode switch
*/
void switch_2(int * d, float posx, float posy, char* title)
{
	double x,y;
	unsigned int stat;
	char str[5]="\0";

	g2_query_pointer(*d,&x,&y,&stat);

	//Allowed clicking positions
	if((x>=posx+5)&&(x<=posx+50)&&(y>=posy+5)&&(y<=posy+25)&&(stat==272)&&(inputs.mode == 'M'))
		inputs.mode = 'A';
	if((x>=posx+5)&&(x<=posx+50)&&(y>=posy+25)&&(y<=posy+45)&&(stat==272)&&(inputs.mode == 'A'))
		inputs.mode = 'M';

	g2_pen(*d,1);
	g2_filled_rectangle(*d, posx, posy, posx + 55,posy + 50);
	g2_string(*d, posx, posy+54, title);
	g2_pen(*d,0);


	//Auto mode
	if(inputs.mode=='A')
	{
		sprintf(str,"AUTO");
		g2_filled_rectangle(*d, posx+5, posy+5, posx + 50,posy + 25);
		g2_pen(*d,1);
		g2_string(*d, posx+10, posy+8 , str);
	}
	//Manual mode
	if(inputs.mode=='M')
	{
		sprintf(str,"MANU");
		g2_filled_rectangle(*d, posx+5, posy+25, posx + 50,posy + 45);
		g2_pen(*d,1);
		g2_string(*d, posx+10, posy+28 , str);
	}



	return;
}

/*
  Function: button
  Handles button function
*/
float button(int * d, int* state, float posx,float posy, char* sign)
{
	double x,y;
	unsigned int stat;
	int width = 20;
	int var=1;

	g2_query_pointer(*d,&x,&y,&stat);


	if((x>=posx)&&(x<=posx+width)&&(y>=posy)&&(y<=posy+width)&&(stat==272)&&(*state==0))
		*state = 1;
	else
		var = 0;

	if((x<posx)||(x>posx+width)||(y<posy)||(y>posy+width)||(stat!=272))
		*state = 0;

	//Graphics
 	g2_pen(*d, 14);
	g2_filled_rectangle(*d, posx, posy, posx + width, posy + width);
	g2_pen(*d, 1);
	g2_set_line_width(*d, 3);
	g2_rectangle(*d, posx, posy, posx + width, posy + width);
	g2_set_line_width(*d, 1);
	g2_string(*d, posx+7, posy+7, sign);


	return var;
}

/*
  Function: button2
  Button that changes vertical speed
*/
void button2(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title)
{
	char aux[20];
	int state=0;
	double var2= inputs.vert_speed;

	var2-=rate*button(d, &state, posx,posy, "-");
	var2+=rate*button(d, &state, posx+60,posy, "+");

	//allowed limits
	if(var2 >= up_lim)
		var2=up_lim;
	if(var2<= down_lim)
		var2=down_lim;
	//Updating aricraft values
	inputs.vert_speed=var2;

	//Graphics
	g2_pen(*d,0);
	g2_filled_rectangle(*d,posx+23,posy+3,posx+57,posy+17);
	g2_pen(*d,1);
	g2_set_line_width(*d,3);
	g2_rectangle(*d,posx+20,posy,posx+60,posy+20);
	sprintf(aux,"%0.0f",var2);//=%0f",*T);
	g2_string(*d, posx+25, posy+5, aux);
	g2_string(*d, posx, posy+25, title);

	return;
}

/*
  Function: button3
  Button that changes heading
*/
void button3(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title)
{
	char aux[20];
	int state=0;
	double var2= inputs.heading;

	var2-=rate*button(d, &state, posx,posy, "-");
	var2+=rate*button(d, &state, posx+60,posy, "+");

	//allowed limits
	if(var2 >= up_lim)
		var2=var2-360;
	if(var2< down_lim)
		var2=var2+360;
	//Updating aricraft values
	inputs.heading=var2;

	//Graphics
	g2_pen(*d,0);
	g2_filled_rectangle(*d,posx+23,posy+3,posx+57,posy+17);
	g2_pen(*d,1);
	g2_set_line_width(*d,3);
	g2_rectangle(*d,posx+20,posy,posx+60,posy+20);
	sprintf(aux,"%0.0f",var2);//=%0f",*T);
	g2_string(*d, posx+25, posy+5, aux);
	g2_string(*d, posx, posy+25, title);

	return;
}

/*
  Function: button4
  Button that changes horizontal speed
*/
void button4(int * d, int rate,int up_lim,int down_lim, float posx,float posy, char* title)
{
	char aux[20];
	int state=0;
	double var2= inputs.hor_speed;

	var2-=rate*button(d, &state, posx,posy, "-");
	var2+=rate*button(d, &state, posx+60,posy, "+");

	//allowed limits
	if(var2 >= up_lim)
		var2=up_lim;
	if(var2< down_lim)
		var2=down_lim;
	//Updating aricraft values
	inputs.hor_speed=var2;

	g2_pen(*d,0);
	g2_filled_rectangle(*d,posx+23,posy+3,posx+57,posy+17);
	g2_pen(*d,1);

	g2_set_line_width(*d,3);
	g2_rectangle(*d,posx+20,posy,posx+60,posy+20);
	sprintf(aux,"%0.0f",var2);//=%0f",*T);
	g2_string(*d, posx+25, posy+5, aux);
	g2_string(*d, posx, posy+25, title);

	return;
}

/*
  Function: delay
  Handles delay
*/
void delay(unsigned int milliseconds){

    clock_t start = clock();

    while((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds);
}

/*
  Function: graph
  Graphics thread function 
*/
void * graph(void *vargp){

	int vd;
	int d;

	g2_init(&vd,&d);

	while(1)
	{

		g2_display(&d);
		delay(1000);	//To allow a better control of the buttons
	}

	g2_close(vd);

}
