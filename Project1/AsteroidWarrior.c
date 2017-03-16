#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define SCORESIZE 7


int ScreenHeight = 800;
int ScreenWidth = 800;
const int FPS = 60.0;




// draws phantom ship
void drawship2(int x, int y, ALLEGRO_COLOR* mycolor, float thickness)
{
    al_draw_elliptical_arc(x, y, 8, 10, -ALLEGRO_PI * 1.40, ALLEGRO_PI * 1.80, *mycolor, thickness);

    al_draw_line(x + -6, y, x + -1, y + 4, *mycolor, thickness);

    al_draw_line(x + 6, y, x + 1, y + 4, *mycolor, thickness);
}












// draws lightning ship
void drawship(int x, int y, ALLEGRO_COLOR* mycolor, float thickness)
{
    al_draw_line(x + -8, y + 9, x + 0, y + -11, *mycolor, thickness);

    al_draw_line(x + 0, y + -11, x + 8, y + 9, *mycolor, thickness);

    al_draw_line(x + -6, y + 4, x + -1, y + 4, *mycolor, thickness);

    al_draw_line(x + 6, y + 4, x + 1, y + 4, *mycolor, thickness);
}

void drawblast(int x, int y, ALLEGRO_COLOR* mycolor)
{
    al_draw_filled_rectangle(x, y, x + 2, y + 6, *mycolor);
}

void drawasteroid(int x, int y, ALLEGRO_COLOR* mycolor, float thickness)
{
    al_draw_line(x + -20, y + 20, x + -25, y + -10, *mycolor, thickness);

    al_draw_line(x + -25, y + 5, x + -25, y + -10, *mycolor, thickness);

    al_draw_line(x + -25, y + -10, x + -5, y + -10, *mycolor, thickness);

    al_draw_line(x + -5, y + -10, x + -10, y + -20, *mycolor, thickness);

    al_draw_line(x + -10, y + -20, x + 5, y + -20, *mycolor, thickness);

    al_draw_line(x + 5, y + -20, x + 20, y + -10, *mycolor, thickness);

    al_draw_line(x + 20, y + -10, x + 20, y + -5, *mycolor, thickness);

    al_draw_line(x + 20, y + -5, x + 0, y + 0, *mycolor, thickness);

    al_draw_line(x + 0, y + 0, x + 20, y + 10, *mycolor, thickness);

    al_draw_line(x + 20, y + 10, x + 10, y + 20, *mycolor, thickness);

    al_draw_line(x + 10, y + 20, x + 0, y + 15, *mycolor, thickness);

    al_draw_line(x + 0, y + 15, x + -20, y + 20, *mycolor, thickness);
}


void draweffect(float radius, float startx, float starty, float xscale, float yscale, ALLEGRO_BITMAP *effect, ALLEGRO_DISPLAY *display)
{
	
	float rotate = ALLEGRO_PI / 3;
	float angle = 0;


	al_set_target_bitmap(al_get_backbuffer(display));

	for (int i = 0; i < 6; i++)
	{
		float rotation = rotate * i;

		startx += radius * sin(rotation);
		starty -= radius * cos(rotation);

		switch (i)
		{
			case 0:
				angle = ALLEGRO_PI / 4;
				break;
			case 1:
				angle = 2 * ALLEGRO_PI / 4;
				break;
			case 2:
				angle = ALLEGRO_PI - ALLEGRO_PI / 4;
				break;
			case 3:
				angle = ALLEGRO_PI / 4;
				break;
			case 4:
				angle = 2 * ALLEGRO_PI / 4;
				break;
			case 5:
				angle = ALLEGRO_PI - ALLEGRO_PI / 4;
				break;
			default:
				angle = 0;
		}

		al_draw_scaled_rotated_bitmap(effect, 0.5, 0.5, startx, starty, xscale, yscale, angle, 0);
	}
}



bool borders(int x, int y)
{
    bool collide = false;
    // if ship is near screen borders set flag to true
    if(x - 4 < 0 || y - 10 < 0 || x + 20 > ScreenWidth || y + 25 > ScreenHeight)
        collide = true;


    return collide;
}


enum direction {UP, DOWN, LEFT, RIGHT, CTRL, SPACE};
typedef enum direction direction;





bool validmove(int x, int y)
{
    // returns false if ship is near screen borders
    bool valid = true;
    if(borders(x, y))
    {
        if(y - 10 < 0 || y + 25 > ScreenHeight)
            valid = false;
        if(x - 4 < 0 || x + 20 > ScreenWidth)
            valid = false;
    }

    return valid;
}



// data structure for ship lazers
struct link
{
    float x;
    float y;
    float rotate;
    float dist;
    struct link *prev;
    struct link *next;
};
typedef struct link node;



enum collidetype {NONE, BLAST, SHIP};
typedef enum collidetype collidetype;




// data structure for asteroids
struct link2
{
    float x;
    float y;
    float rotate;
    float scale;
    float rotspeed;
    collidetype collide;
    struct link2 *prev;
    struct link2 *next;
};
typedef struct link2 node2;


/*
these functions are for ship lazer doubly list manipulation


*/

void deleteNode(node **head_ref, node *del)
{
  /* base case */
    if(*head_ref == NULL || del == NULL)
        return;

   /* If node to be deleted is head node */
    if(*head_ref == del)
        *head_ref = del->next;

    /* Change next only if node to be deleted is NOT the last node */
    if(del->next != NULL)
        del->next->prev = del->prev;

     /* Change prev only if node to be deleted is NOT the first node */
    if(del->prev != NULL)
        del->prev->next = del->next;

  /* Finally, free the memory occupied by del*/
    free(del);
    return;
}


void update_list(float movement, node **nodehead)
{
    node *trav = *nodehead;
    // traverse entire list of doubly linked nodes
    while(trav != NULL)
    {
        // if ship nears screen borders
        if(borders(trav->x, trav->y))
        {

            // teleport to opposite side of screen
            if(trav->y - 10 < 0)
                trav->y = ScreenHeight - 30;
            else if(trav->y + 25 > ScreenHeight)
                trav->y = 15;


            if(trav->x - 10 < 0)
                trav->x = ScreenWidth - 30;
            else if(trav->x + 25 > ScreenWidth)
                trav->x = 15;


        }



        // if node list empty
        if(trav == NULL)
            break;


        // update distance node (*ship lazer*) has traveled
        trav->dist += sqrt(2 * pow(movement, 2));

        // if traveled greater than 2k delete node (*lazer*)
        if(trav->dist > 2000)
        {
            node *freenode = trav;
            trav = trav->next;
            deleteNode(nodehead, freenode);
            continue;
        }


        // rotate and advance lazer
        trav->x += movement * sin(trav->rotate);
        trav->y -= movement * cos(trav->rotate);

        // update trav pointer to next node
        trav = trav->next;
    }
}

void draw_nodes(node *nodehead, ALLEGRO_BITMAP *image, ALLEGRO_DISPLAY *display)
{
    al_set_target_bitmap(al_get_backbuffer(display));
    node *trav = nodehead;
    // iterate through lazers
    while(trav != NULL)
    {
        // draw lazer
        al_draw_rotated_bitmap(image, 1, 3, trav->x, trav->y, trav->rotate, 0);
        // next in lazer list
        trav = trav->next;
    }
}

void destroy_list(node **nodehead)
{
    node *freenode;
    node *trav = *nodehead;
    // iterate list
    while(trav != NULL)
    {
        // delete element
        freenode = trav;
        trav = trav->next;
        deleteNode(nodehead, freenode);
    }
}

node *insert_node(float x, float y, float rotate, node *nodehead)
{
    node *trav;
    node *newnode;
    // if list empty make new node and point nodehead to it
    if(nodehead == NULL)
    {
        nodehead = malloc(sizeof(node));
        nodehead->prev = NULL;
        nodehead->next = NULL;
        nodehead->x = x;
        nodehead->y = y;
        nodehead->rotate = rotate;
        nodehead->dist = 0;
    }

    // traverse list until null and make new node
    else
    {
        trav = nodehead;
        while(trav->next != NULL)
            trav = trav->next;
        trav->next = malloc(sizeof(node));
        newnode = trav->next;
        newnode->prev = trav;
        newnode->next = NULL;
        newnode->x = x;
        newnode->y = y;
        newnode->rotate = rotate;
        newnode->dist = 0;
    }
    return nodehead;
}









int score = 0;
int lives = 3;
int level = 1;






int update_score(char *scorestring)
{
    int error = 0;
    int len = strlen(scorestring);
    char buffer[SCORESIZE + 1];
    // copy integer value into a buffer including null char
    snprintf(buffer, SCORESIZE + 1, "%d", score);
    int lenbuf = strlen(buffer);

    // if score is greater than set scoresize set error to 1
    if(score >= (int)pow(10, SCORESIZE))
    {
        error = 1;
    }
    // otherwise, copy in scorestring to be displayed
    else
    {
		int scorelen = SCORESIZE;
        for(int i = len; scorelen > 0; scorelen--, i--, lenbuf--)
        {
           if(lenbuf > 0)
               scorestring[i - 1] = buffer[lenbuf - 1];
           else
               scorestring[i - 1] = '0';
        }
    }

    return error;
}








/*
these are for asteroid doubly
list manipulation
*/


// comments are the same as above besides minor details 
node2 *insert_node2(float x, float y, float rotate, float scale, node2 *nodehead)
{
    node2 *trav;
    node2 *newnode;
    if(nodehead == NULL)
    {
        nodehead = malloc(sizeof(node2));
        nodehead->prev = NULL;
        nodehead->next = NULL;
        nodehead->x = x;
        nodehead->y = y;
        nodehead->rotate = rotate;
        nodehead->rotspeed = 0;
        nodehead->scale = scale;
        nodehead->collide = NONE;
    }
    else
    {
        trav = nodehead;
        while(trav->next != NULL)
            trav = trav->next;
        trav->next = malloc(sizeof(node2));
        newnode = trav->next;
        newnode->prev = trav;
        newnode->next = NULL;
        newnode->x = x;
        newnode->y = y;
        newnode->rotate = rotate;
        newnode->rotspeed = 0;
        newnode->scale = scale;
        newnode->collide = NONE;
    }
    return nodehead;
}









void deleteNode2(node2 **head_ref, node2 *del)
{
  /* base case */
    if(*head_ref == NULL || del == NULL)
        return;

   /* If node to be deleted is head node */
    if(*head_ref == del)
        *head_ref = del->next;

    /* Change next only if node to be deleted is NOT the last node */
    if(del->next != NULL)
        del->next->prev = del->prev;

     /* Change prev only if node to be deleted is NOT the first node */
    if(del->prev != NULL)
        del->prev->next = del->next;

  /* Finally, free the memory occupied by del*/
    free(del);
    return;
}


void update_list2(float movement, node2 **nodehead)
{
    node2 *trav = *nodehead;
    while(trav != NULL)
    {
        if(borders(trav->x, trav->y))
        {
            if(trav->y - 10 < 0)
                trav->y = ScreenHeight - 30;
            else if(trav->y + 25 > ScreenHeight)
                trav->y = 15;


            if(trav->x - 10 < 0)
                trav->x = ScreenWidth - 30;
            else if(trav->x + 25 > ScreenWidth)
                trav->x = 15;

        }


        if(trav->collide == BLAST && trav->scale <= 0.6)
        {
            node2 *freenode = trav;
            trav = trav->next;
            deleteNode2(nodehead, freenode);
            score += 75;
            continue;
        }


        if(trav->collide == BLAST && trav->scale >= 1.4)
        {
            trav->scale = 0.5;
            float rotate1 = trav->rotate <= -360 ? 0 : trav->rotate - 10;
            float rotate2 = trav->rotate >= 360 ? 0 : trav->rotate + 10;

            insert_node2(trav->x, trav->y, trav->rotate, trav->scale, *nodehead);
            insert_node2(trav->x, trav->y, rotate1, trav->scale, *nodehead);
            insert_node2(trav->x, trav->y, rotate2, trav->scale, *nodehead);






            node2 *freenode = trav;
            trav = trav->next;
            deleteNode2(nodehead, freenode);
            continue;
        }






        if(trav->collide != NONE)
        {
            if(trav->collide == SHIP)
            {
                lives--;
                trav->collide = NONE;
            }
            else if(trav->collide == BLAST)
            {
                trav->scale = 0.5;
                float rotate1 = trav->rotate <= -360 ? 0 : trav->rotate - 10;
                float rotate2 = trav->rotate >= 360 ? 0 : trav->rotate + 10;

                insert_node2(trav->x, trav->y, rotate1, trav->scale, *nodehead);
                insert_node2(trav->x, trav->y, rotate2, trav->scale, *nodehead);

                node2 *freenode = trav;
                trav = trav->next;
                deleteNode2(nodehead, freenode);
                continue;
            }
        }

        trav->x += movement * sin(trav->rotate);
        trav->y -= movement * cos(trav->rotate);
        trav = trav->next;
    }
}



void draw_nodes2(node2 *nodehead, ALLEGRO_BITMAP *image, ALLEGRO_BITMAP *image2, ALLEGRO_DISPLAY *display, float asteroidspeed)
{
    al_set_target_bitmap(al_get_backbuffer(display));
    node2 *trav = nodehead;
    while(trav != NULL)
    {
		// if rotation reaches limits reset it to 0
        if(trav->rotspeed + trav->rotate >= 360 || trav->rotspeed + trav->rotate <= -360)
            trav->rotspeed = 0;

        trav->rotspeed += 0.5;


		// draw appropriate asteroid based off scale
        if(trav->scale <= 1.1)
            al_draw_scaled_rotated_bitmap(image, 20, 22.5, trav->x, trav->y, trav->scale, trav->scale, trav->rotate + trav->rotspeed, 0);
        else
            al_draw_scaled_rotated_bitmap(image2, 20, 22.5, trav->x, trav->y, trav->scale, trav->scale, trav->rotate + trav->rotspeed, 0);

        trav = trav->next;
    }
}

void destroy_list2(node2 **nodehead)
{
    node2 *freenode;
    node2 *trav = *nodehead;
    while(trav != NULL)
    {
        freenode = trav;
        trav = trav->next;
        deleteNode2(nodehead, freenode);
    }
}





struct effectdata
{
	float xcor;
	float ycor;
	bool collide;
};
typedef struct effectdata effectdata;


bool collision(node **blastlist, node2 *asteroidlist, float x, float y, bool *asteroidblasted, bool *shipblasted, effectdata *data)
{

    *asteroidblasted = false;
    *shipblasted = false;


    node *trav = *blastlist;
    node2 *trav2 = asteroidlist;

    bool collision = false;

	// iterate through asteroid list
    while(trav2 != NULL)
    {
		// iterate through ship lazer list
        while(trav != NULL)
        {
			// if lazer is near asteroid set collision to BLAST
            if((abs(trav->x - trav2->x) <= 30 * trav2->scale) && (abs(trav->y - trav2->y) <= 30 * trav2->scale))
            {
               *asteroidblasted = true;


			   data->xcor = trav2->x;
			   data->ycor = trav2->y;
			   data->collide = true;


               trav2->collide = BLAST;

			   // update lazer list and delete lazer node
               node *freenode = trav;
               trav = trav->next;
               deleteNode(blastlist, freenode);
               continue;
            }

			// update lazer list
            trav = trav->next;
        }
		// reset lazer list to head and advance asteroid list
        trav = *blastlist;
        trav2 = trav2->next;
    }

	// set asteroid list to head
    trav2 = asteroidlist;

	// iterate asteroid list
    while(trav2 != NULL)
    {
		// if ship close to asteroid set collision to SHIP and raise flag
        if(abs(x - trav2->x) <= 30 * trav2->scale && abs(y - trav2->y) <= 30 * trav2->scale)
        {
            trav2->collide = SHIP;
            collision = true;

			data->xcor = x;
			data->ycor = y;
			data->collide = true;


            *shipblasted = true;
        }

        trav2 = trav2->next;
    }

    return collision;
}
















int main(void)
{
    node *blastlist = NULL;
    node2 *asteroidlist = NULL;


	// seed for pseudo random generator
    srand((unsigned int)time(NULL));



    ALLEGRO_DISPLAY *display;

    if(!al_init_native_dialog_addon() || !al_init_font_addon() || !al_init_ttf_addon())
    {
        fprintf(stderr, "Error could not initialize addons\n");
        exit(1);
    }

    if(!al_init())
    {
        al_show_native_message_box(NULL, NULL, NULL, "Could not initialize allegro 5", NULL, ALLEGRO_MESSAGEBOX_QUESTION);
        exit(1);
    }


    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    display = al_create_display(ScreenWidth, ScreenHeight);
    ScreenWidth = al_get_display_width(display);
    ScreenHeight = al_get_display_height(display);

    if(!display)
    {
        al_show_native_message_box(NULL, NULL, NULL, "Could not initialize allegro 5", NULL, ALLEGRO_MESSAGEBOX_QUESTION);
    }

    al_set_window_position(display, 0, 0);
    al_set_window_title(display, "ASTEROID WARRIOR");


    ALLEGRO_FONT *font = al_load_font("assets/fonts/pirulen.ttf", 10, 0);

    ALLEGRO_FONT *font2 = al_load_font("assets/fonts/bodoni.ttf", 20, 0);


    al_init_primitives_addon();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();



    al_reserve_samples(4);

    ALLEGRO_SAMPLE *explosionsound = al_load_sample("assets/sounds/explosion.wav");
    ALLEGRO_SAMPLE *lazersound = al_load_sample("assets/sounds/lazer.wav");
    ALLEGRO_SAMPLE *blastsound = al_load_sample("assets/sounds/blast.wav");


    ALLEGRO_SAMPLE *musicsound = al_load_sample("assets/music/ken.wav");
    ALLEGRO_SAMPLE *musicsound1 = al_load_sample("assets/music/guile.wav");
    ALLEGRO_SAMPLE *musicsound2 = al_load_sample("assets/music/battles.wav");

    ALLEGRO_SAMPLE_INSTANCE *musicinstance = al_create_sample_instance(musicsound);
    al_set_sample_instance_playmode(musicinstance, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(musicinstance, al_get_default_mixer());


    ALLEGRO_SAMPLE_INSTANCE *musicinstance1 = al_create_sample_instance(musicsound1);
    al_set_sample_instance_playmode(musicinstance1, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(musicinstance1, al_get_default_mixer());


    ALLEGRO_SAMPLE_INSTANCE *musicinstance2 = al_create_sample_instance(musicsound2);
    al_set_sample_instance_playmode(musicinstance2, ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(musicinstance2, al_get_default_mixer());










    ALLEGRO_COLOR mycolor = al_map_rgb(20, 150, 255);
    ALLEGRO_COLOR mycolor2 = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR blastcolor = al_map_rgb(200, 50, 0);
    ALLEGRO_COLOR asteroidcolor = al_map_rgb(255, 25, 255);




    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);



    bool done = false;
    bool draw = true;
    int shipx = ScreenWidth / 2 - 8;
    int shipy = ScreenHeight / 2 - 10;
    float asteroidspeed = 5;
    float lazerspeed = 15;
    float shipspeed = 0;
    float rotate = 0;
    direction dir = CTRL;




    ALLEGRO_BITMAP *bitmap = al_create_bitmap(16, 20);


    al_set_target_bitmap(bitmap);
    drawship(8, 11, &mycolor, 1.0);



    ALLEGRO_BITMAP *bitmap2 = al_create_bitmap(16, 20);


    al_set_target_bitmap(bitmap2);
    drawship2(8, 11, &mycolor2, 1.0);



    ALLEGRO_BITMAP *blast = al_create_bitmap(2, 6);

    al_set_target_bitmap(blast);
    drawblast(0, 0, &blastcolor);


    ALLEGRO_BITMAP *asteroid = al_create_bitmap(45, 40);

    al_set_target_bitmap(asteroid);
    drawasteroid(25, 20, &asteroidcolor, 1.0);


    ALLEGRO_COLOR asteroidcolor2 = al_map_rgb(0, 255, 50);
    ALLEGRO_BITMAP *asteroid2 = al_create_bitmap(45, 40);

    al_set_target_bitmap(asteroid2);
    drawasteroid(25, 20, &asteroidcolor2, 1.0);







    al_set_target_bitmap(al_get_backbuffer(display));

    bool death = false;
    int waitframes = 0;


    char scorestring[20] = "Score 0000000";




	ALLEGRO_BITMAP *line = al_create_bitmap(1, 1);
	al_set_target_bitmap(line);
	al_draw_filled_rectangle(0, 0, 1, 1, al_map_rgb(250, 255, 0));

	al_set_target_bitmap(al_get_backbuffer(display));

	// title screen

    al_draw_elliptical_arc(ScreenWidth / 2, ScreenHeight / 2, ScreenWidth / 2, ScreenHeight / 2, 0, ALLEGRO_PI, al_map_rgb(100, 200, 255), 1.0);
    al_draw_elliptical_arc(ScreenWidth / 2, ScreenHeight / 2, ScreenWidth / 2, ScreenHeight / 2, 0, -ALLEGRO_PI, al_map_rgb(100, 200, 255), 1.0);


    ALLEGRO_COLOR textcolor = al_map_rgba(20, 100, 255, 255);

    drawship(ScreenWidth / 2 - 30, ScreenHeight / 2 - 30, &mycolor, 1.0);
    drawship2(ScreenWidth / 2 + 30, ScreenHeight / 2 - 30, &mycolor2, 1.0);

    al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2, ALLEGRO_ALIGN_CENTRE, "ASTEROID WARRIOR");
    al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 30, ALLEGRO_ALIGN_CENTRE, "Press any key to continue");
	draweffect(175, ScreenWidth / 2 - 150, ScreenHeight / 2 + 75, 15, 5, line, display);
    al_flip_display();


    ALLEGRO_EVENT key;
    ALLEGRO_EVENT_QUEUE *tempqueue = al_create_event_queue();
    al_register_event_source(tempqueue, al_get_keyboard_event_source());
    al_register_event_source(tempqueue, al_get_display_event_source(display));
    do
    {
        al_wait_for_event(tempqueue, &key);
    }while(key.type != ALLEGRO_EVENT_KEY_DOWN && key.type != ALLEGRO_EVENT_DISPLAY_CLOSE);


	// sources for assets

    al_clear_to_color(al_map_rgb(0, 0, 0));


    al_draw_text(font2, textcolor, ScreenWidth / 2, 50, ALLEGRO_ALIGN_CENTRE, "MUSIC BY BULBY");


    al_draw_text(font2, textcolor, ScreenWidth / 2, 75, ALLEGRO_ALIGN_CENTRE, "channel: \
youtube.com/channel/UCz6zvgkf6eKpgqlUZQstOtQ");

    al_draw_text(font2, textcolor, ScreenWidth / 2, 100, ALLEGRO_ALIGN_CENTRE, "1st level: \
youtube.com/watch?v=4eg1uYQhcoM");

    al_draw_text(font2, textcolor, ScreenWidth / 2, 125, ALLEGRO_ALIGN_CENTRE, "2nd level: \
youtube.com/watch?v=kucS8dNTHuM");

    al_draw_text(font2, textcolor, ScreenWidth / 2, 150, ALLEGRO_ALIGN_CENTRE, "3rd level: \
youtube.com/watch?v=04_jviOqc3Y");



    al_draw_text(font2, textcolor, ScreenWidth / 2, 175, ALLEGRO_ALIGN_CENTRE, "SOUNDS");
    al_draw_text(font2, textcolor, ScreenWidth / 2, 200, ALLEGRO_ALIGN_CENTRE, "freesound.org");



    al_draw_text(font2, textcolor, ScreenWidth / 2, 225, ALLEGRO_ALIGN_CENTRE, "FONTS");
    al_draw_text(font2, textcolor, ScreenWidth / 2, 250, ALLEGRO_ALIGN_CENTRE, "1001freefonts.com");

    al_draw_text(font2, textcolor, ScreenWidth / 2, 275, ALLEGRO_ALIGN_CENTRE, "GAME LIBRARY");
    al_draw_text(font2, textcolor, ScreenWidth / 2, 300, ALLEGRO_ALIGN_CENTRE, "Allegro 5");
    al_draw_text(font2, textcolor, ScreenWidth / 2, 325, ALLEGRO_ALIGN_CENTRE, "liballeg.org");
    al_draw_text(font2, textcolor, ScreenWidth / 2, 350, ALLEGRO_ALIGN_CENTRE, "allegro.cc");
	al_draw_text(font, textcolor, ScreenWidth / 2, 375, ALLEGRO_ALIGN_CENTRE, "programming by mike mo");

    al_draw_text(font, textcolor, ScreenWidth / 2, 400, ALLEGRO_ALIGN_CENTRE, "press any key to continue");


    al_flip_display();



//www.youtube.com/watch?v=4eg1uYQhcoM
//www.youtube.com/watch?v=kucS8dNTHuM
//www.youtube.com/watch?v=04_jviOqc3Y








    do
    {
        al_wait_for_event(tempqueue, &key);
    }while(key.type != ALLEGRO_EVENT_KEY_DOWN && key.type != ALLEGRO_EVENT_DISPLAY_CLOSE);












    al_clear_to_color(al_map_rgb(0, 0, 0));



	// instructions screen
	textcolor = al_map_rgba(75, 255, 255, 255);

	drawship(30, 30, &mycolor, 1.0);
	drawship2(30, ScreenHeight / 2 - 30, &mycolor2, 1.0);

	al_draw_text(font, textcolor, ScreenWidth / 2, 45, ALLEGRO_ALIGN_CENTRE, "lightning");
	al_draw_text(font, textcolor, ScreenWidth / 2, 60, ALLEGRO_ALIGN_LEFT, "space to shoot");
	al_draw_text(font, textcolor, ScreenWidth / 2, 75, ALLEGRO_ALIGN_LEFT, "up/down to a/de/ccelerate");
	al_draw_text(font, textcolor, ScreenWidth / 2, 90, ALLEGRO_ALIGN_LEFT, "right/left to turn");
	al_draw_text(font, textcolor, ScreenWidth / 2, 105, ALLEGRO_ALIGN_LEFT, "right control to do a fullstop");

	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 - 15, ALLEGRO_ALIGN_CENTRE, "phantom");
	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2, ALLEGRO_ALIGN_RIGHT, "space to shoot");
	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 15, ALLEGRO_ALIGN_RIGHT, "up to start moving or to move in rotated position, down to deccelerate");
	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 30, ALLEGRO_ALIGN_RIGHT, "right/left to turn");
	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 45, ALLEGRO_ALIGN_RIGHT, "right control to do a fullstop");
	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 60, ALLEGRO_ALIGN_RIGHT, "right shift to accelerate");

	al_draw_text(font, al_map_rgb(255, 25, 25), ScreenWidth / 2, ScreenHeight / 2 + 90, ALLEGRO_ALIGN_CENTRE, "r to reset lives");

	al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 120, ALLEGRO_ALIGN_CENTRE, "press any key to continue");

	al_flip_display();


	do
	{
		al_wait_for_event(tempqueue, &key);
	} while (key.type != ALLEGRO_EVENT_KEY_DOWN && key.type != ALLEGRO_EVENT_DISPLAY_CLOSE);


	al_clear_to_color(al_map_rgb(0, 0, 0));



    direction dir2 = CTRL;
    bool finished = false;


    if(key.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        finished = true;
    else if(key.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        if(key.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            finished = true;
    }


    bool asteroidblasted = false;


    bool shipblasted = false;


    bool defmove = true;
    float temprotate = 0;


    bool framewait = true;
    int frames = 0;


	effectdata data;

	int effectframes = 0;

	bool cheat = false;

	while(!finished)
	{



		al_clear_to_color(al_map_rgb(0, 0, 0));

		mycolor = al_map_rgb(20, 150, 255);
		drawship(ScreenWidth / 2 - 30, ScreenHeight / 2 - 30, &mycolor, 1.0);

		mycolor = al_map_rgb(255, 255, 255);
		drawship2(ScreenWidth / 2 + 30, ScreenHeight / 2 - 30, &mycolor, 1.0);


		al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2, ALLEGRO_ALIGN_CENTRE, "LIGHTNING or PHANTOM");
		al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 30, ALLEGRO_ALIGN_CENTRE, "Press L or P");
		al_flip_display();



		// check for keyboard input until valid response
		ALLEGRO_EVENT_QUEUE *temp = al_create_event_queue();
		al_register_event_source(temp, al_get_keyboard_event_source());
		ALLEGRO_EVENT key0;
		do
		{
			al_wait_for_event(temp, &key0);
		}while(key0.type != ALLEGRO_EVENT_KEY_DOWN || (key0.keyboard.keycode != ALLEGRO_KEY_L && key0.keyboard.keycode !=\
		ALLEGRO_KEY_P && key0.keyboard.keycode != ALLEGRO_KEY_ESCAPE));


		al_destroy_event_queue(temp);


		// set according to input
		if(key0.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if(key0.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			{
				done = true;
				finished = true;
				goto finished;
			}
			else if(key0.keyboard.keycode == ALLEGRO_KEY_P)
			{
				defmove = false;
				al_set_target_bitmap(bitmap2);
				mycolor2 = al_map_rgb(255, 255, 255);
				drawship2(8, 11, &mycolor2, 1.0);
				al_set_target_bitmap(al_get_backbuffer(display));
			}
			else
			{
				defmove = true;
				al_set_target_bitmap(bitmap);
				mycolor = al_map_rgb(20, 150, 255);
				drawship(8, 11, &mycolor, 1.0);
				al_set_target_bitmap(al_get_backbuffer(display));

			}

		}


		al_clear_to_color(al_map_rgb(0, 0, 0));



		// set and start timer for FPS
		ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
		al_register_event_source(event_queue, al_get_keyboard_event_source());
		al_register_event_source(event_queue, al_get_display_event_source(display));
		al_register_event_source(event_queue, al_get_timer_event_source(timer));


		ALLEGRO_EVENT events;



		al_start_timer(timer);

		// game loop
		while(!done)
		{









			al_wait_for_event(event_queue, &events);


			// if keyboard key is pressed
			if(events.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				switch(events.keyboard.keycode)
				{
					// resets your lives (*cheat?*)
					case ALLEGRO_KEY_R:
						lives = 3;
						cheat = true;
						break;
					// set enum accordingly
					case ALLEGRO_KEY_SPACE:
						dir2 = SPACE;
						break;
					case ALLEGRO_KEY_DOWN:
						dir = DOWN;
						break;
					case ALLEGRO_KEY_UP:
						if(!defmove && shipspeed <= 0.1)
							shipspeed = 5.0;
						dir = UP;
						break;
					case ALLEGRO_KEY_RIGHT:
						dir = RIGHT;
						break;
					case ALLEGRO_KEY_LEFT:
						dir = LEFT;
						break;
					case ALLEGRO_KEY_RCTRL:
						dir = CTRL;
						break;
					case ALLEGRO_KEY_ESCAPE:
						done = true;
						finished = true;
						goto finished;
						break;
				}
			}
			else if(events.type == ALLEGRO_EVENT_KEY_UP)
			{
				switch(events.keyboard.keycode)
				{

					// def move is if you chose lightning
					case ALLEGRO_KEY_RSHIFT:
						if(!defmove)
						{
							if(shipspeed < 15)
								shipspeed += 2.5;
						}
					case ALLEGRO_KEY_UP:
						if(defmove)
						{
							if(shipspeed < 15)
								shipspeed += 2.5;
						}
						dir = UP;
						break;
					case ALLEGRO_KEY_DOWN:
						if(shipspeed > 0)
							shipspeed -= 2.5;
						dir = DOWN;
						break;
					case ALLEGRO_KEY_RIGHT:
						dir = CTRL;
						break;
					case ALLEGRO_KEY_LEFT:
						dir = CTRL;
						break;
					case ALLEGRO_KEY_RCTRL:
						dir = CTRL;
						shipspeed = 0;
						break;
					case ALLEGRO_KEY_SPACE:
						dir2 = CTRL;
						break;
				}
			}
			else if(events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				done = true;
				finished = true;
				goto finished;
			}

			if(events.type == ALLEGRO_EVENT_TIMER)
			{

				// shoots lazers even when holding space
				// limited by framewait
				if(!framewait && dir2 == SPACE)
				{
					blastlist = insert_node(shipx, shipy, rotate, blastlist);
					al_play_sample(lazersound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
					framewait = true;
				}
				else
				{
					if(frames <= 10)
						frames++;
					else
					{
						frames = 0;
						framewait = false;
					}
				}


				// checks for death
				// waits 2 seconds before checking for collisions again
				if(!death && collision(&blastlist, asteroidlist, shipx, shipy, &asteroidblasted, &shipblasted, &data))
				{
					shipx = ScreenWidth / 2 - 8;
					shipy = ScreenHeight / 2 - 10;
					shipspeed = 0;
					rotate = 0;
					death = true;
				}
				else if(death)
					waitframes++;

				if(waitframes >= 120)
				{
					death = false;
					waitframes = 0;
				}

				switch(dir)
				{
					case RIGHT:
						rotate += ALLEGRO_PI / 18;
						break;
					case LEFT:
						rotate -= ALLEGRO_PI / 18;
						break;
					case CTRL:
						break;
				}

			
				// handles different movement for different ships
				if(defmove)
				{
					shipx += shipspeed * sin(rotate);
					shipy -= shipspeed * cos(rotate);
				}
				else
				{
					if(dir == UP)
					{
						shipx += shipspeed * sin(rotate);
						shipy -= shipspeed * cos(rotate);
						temprotate = rotate;
					}
					else
					{
						shipx += shipspeed * sin(temprotate);
						shipy -= shipspeed * cos(temprotate);
					}
				}

				// teleports ship if near screen borders to opposite side
				if(borders(shipx, shipy))
				{
					if(shipy - 10 < 0)
						shipy = ScreenHeight - 30;
					else if(shipy + 25 > ScreenHeight)
						shipy = 15;


					if(shipx - 10 < 0)
						shipx = ScreenWidth - 30;
					else if(shipx + 25 > ScreenWidth)
						shipx = 15;
				}




				// plays sounds if asteroid or ship destroyed
				if(asteroidblasted)
				{
					al_play_sample(blastsound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
					asteroidblasted = false;
				}
				if(shipblasted)
				{
					al_play_sample(explosionsound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
					shipblasted = false;
				}


				// if level cleared advances level waits 2 seconds
				if(asteroidlist == NULL)
				{
					death = true;
					if(level == 1)
					{

						al_play_sample_instance(musicinstance);
						for(int i = 0; i < 10; i++)
						{
							float x2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenWidth - 60) + 30;
							float y2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenHeight - 60) + 30;
							float rotaterand = ((float)rand()/(float)(RAND_MAX)) * (2 * ALLEGRO_PI - ALLEGRO_PI / 36) +\
							ALLEGRO_PI / 36;

							asteroidlist = insert_node2(x2, y2, rotaterand, 1.0, asteroidlist);
						}
						level++;
					}
					else if(level == 2)
					{
						al_stop_sample_instance(musicinstance);
						al_play_sample_instance(musicinstance1);
						for(int i = 0; i < 10; i++)
						{
							float x2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenWidth - 60) + 30;
							float y2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenHeight - 60) + 30;
							float rotaterand = ((float)rand()/(float)(RAND_MAX)) * (2 * ALLEGRO_PI - ALLEGRO_PI / 36) +\
							ALLEGRO_PI / 36;

							asteroidlist = insert_node2(x2, y2, rotaterand, 1.5, asteroidlist);
						}
						level++;

					}
					else if(level == 3)
					{
						al_stop_sample_instance(musicinstance1);
						al_play_sample_instance(musicinstance2);
						for(int i = 0; i < 10; i++)
						{
							float x2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenWidth - 60) + 30;
							float y2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenHeight - 60) + 30;
							float rotaterand = ((float)rand()/(float)(RAND_MAX)) * (2 * ALLEGRO_PI - ALLEGRO_PI / 36) +\
							ALLEGRO_PI / 36;

							asteroidlist = insert_node2(x2, y2, rotaterand, 1.5, asteroidlist);
						}
						for(int i = 0; i < 10; i++)
						{
							float x2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenWidth - 60) + 30;
							float y2 = ((float)rand()/(float)(RAND_MAX)) * (ScreenHeight - 60) + 30;
							float rotaterand = ((float)rand()/(float)(RAND_MAX)) * (2 * ALLEGRO_PI - ALLEGRO_PI / 36) +\
							ALLEGRO_PI / 36;

							asteroidlist = insert_node2(x2, y2, rotaterand, 1.0, asteroidlist);
						}

					}
				}

				draw = true;
			}


			bool first = true;
			// if timer finished and no events draw and update accordingly
			if(draw && al_is_event_queue_empty(event_queue))
			{

				if (data.collide && effectframes <= 30)
				{
					draweffect(5, data.xcor, data.ycor, 8, 3, line, display);
					effectframes++;
				}
				else
				{
					data.collide = false;
					effectframes = 0;
				}




				// draws and checks for lives
				if(lives > -1)
				{
					int startx = 20;
					int starty = ScreenHeight - 20;
					ALLEGRO_COLOR livescolor = al_map_rgba(255, 20, 20, 255);
					for(int i = 0; i < lives; i++)
						drawship(startx + 16 * i, starty, &livescolor, 1.0);
				}
				else
				{
					done = true;
				}

				update_score(scorestring);

				ALLEGRO_COLOR textcolor = al_map_rgba(20, 100, 255, 255);
				al_draw_text(font, textcolor, 30, 30, ALLEGRO_ALIGN_LEFT, scorestring);

				if(rotate > 2 * ALLEGRO_PI || rotate < -2 * ALLEGRO_PI)
				{
					rotate = 0;
				}


				if(defmove)
					al_draw_rotated_bitmap(bitmap, 8, 10, shipx, shipy, rotate, 0);
				else
					al_draw_rotated_bitmap(bitmap2, 8, 10, shipx, shipy, rotate, 0);


				draw_nodes(blastlist, blast, display);

				draw_nodes2(asteroidlist, asteroid, asteroid2, display, asteroidspeed);

				al_flip_display();
				update_list(lazerspeed, &blastlist);

				update_list2(asteroidspeed, &asteroidlist);

				al_clear_to_color(al_map_rgb(0, 0, 0));
			}
		}
		al_stop_timer(timer);
		if(al_get_sample_instance_playing(musicinstance))
			al_stop_sample_instance(musicinstance);


		else if(al_get_sample_instance_playing(musicinstance1))
			al_stop_sample_instance(musicinstance1);


		else if(al_get_sample_instance_playing(musicinstance2))
			al_stop_sample_instance(musicinstance2);





		// game over screen
		al_clear_to_color(al_map_rgb(0, 0, 0));

		ALLEGRO_COLOR textcolor = al_map_rgba(255, 0, 0, 255);
		al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2, ALLEGRO_ALIGN_CENTRE, scorestring);
		al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 - 30, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
		al_draw_text(font, textcolor, ScreenWidth / 2, ScreenHeight / 2 + 30, ALLEGRO_ALIGN_CENTRE,\
		"Press enter to play again, escape to quit");

		if(cheat)
			al_draw_text(font, al_map_rgb(250, 250, 0) , ScreenWidth / 2, ScreenHeight / 2 - 60, ALLEGRO_ALIGN_CENTRE, "you cheated!");

		al_flip_display();


		ALLEGRO_EVENT key1;
		ALLEGRO_EVENT_QUEUE *tempqueue1 = al_create_event_queue();
		al_register_event_source(tempqueue1, al_get_keyboard_event_source());
		al_register_event_source(tempqueue1, al_get_display_event_source(display));
		do
		{
			al_wait_for_event(tempqueue1, &key1);
		}while(key1.keyboard.keycode != ALLEGRO_KEY_ESCAPE && key1.keyboard.keycode != ALLEGRO_KEY_ENTER && key1.type != ALLEGRO_EVENT_DISPLAY_CLOSE);


		al_destroy_event_queue(tempqueue1);



		al_destroy_event_queue(event_queue);

		// terminates program if user decides to quit
		if(key1.type == ALLEGRO_EVENT_DISPLAY_CLOSE || key1.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			finished = true;
		// otherwise resets data/or state for another playthrough
		else
		{
			shipx = ScreenWidth / 2 - 8;
			shipy = ScreenHeight / 2 - 10;
			shipspeed = 0;
			rotate = 0;
			dir = CTRL;
			dir2 = CTRL;

			score = 0;
			lives = 3;
			level = 1;
			update_score(scorestring);

			data.collide = false;
			effectframes = 0;
			cheat = false;

			done = false;
			death = true;
			destroy_list(&blastlist);
			destroy_list2(&asteroidlist);
		}

	}


	finished:
		al_destroy_font(font);
		al_destroy_font(font2);
		al_destroy_display(display);
		al_destroy_timer(timer);

		al_destroy_event_queue(tempqueue);

		al_destroy_bitmap(bitmap);
		al_destroy_bitmap(bitmap2);
		al_destroy_bitmap(blast);
		al_destroy_bitmap(asteroid);
		al_destroy_bitmap(asteroid2);
		al_destroy_bitmap(line);

		al_destroy_sample(blastsound);
		al_destroy_sample(lazersound);
		al_destroy_sample(musicsound);
		al_destroy_sample(musicsound1);
		al_destroy_sample(musicsound2);
		al_destroy_sample(explosionsound);

		al_destroy_sample_instance(musicinstance);
		al_destroy_sample_instance(musicinstance1);
		al_destroy_sample_instance(musicinstance2);


		destroy_list(&blastlist);
		destroy_list2(&asteroidlist);

    return 0;
}
