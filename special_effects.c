#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "highlight.h"
/* to do
 more effects
 adjust effects based on actor size and if sitting
*/
#ifdef SFX
//much of this is based on the highlight.c code
#define SPECIAL_EFFECT_LIFESPAN	(500)
#define SPECIAL_EFFECT_SHIELD_LIFESPAN (1500)
#define SPECIAL_EFFECT_HEAL_LIFESPAN (5000)
#define SPECIAL_EFFECT_RESTORATION_LIFESPAN (1500)
#define NUMBER_OF_SPECIAL_EFFECTS	(100)	// 100 active in one area should be enough, right?

#define STATIC_SFX -2

typedef struct {
	short x;		// used to store x_tile_pos and y_tile_pos
	short y;		// will probably need a z too eventually
	actor *owner;	// will be NULL for stationary effects
	int timeleft;
	int lifespan;	// total lifespan of effect
	int type;		// type of effect / spell that was cast
	int active;	
	int caster;		//is this caster or target, static effects will be set to STATIC_SFX
	Uint32 last_time;	//for timing length of effect especially while in console
} special_effect;

special_effect sfx_markers[NUMBER_OF_SPECIAL_EFFECTS];

int sfx_enabled = 1;

const static float dx = (TILESIZE_X / 6);
const static float dy = (TILESIZE_Y / 6);

//Allocate a spot for a new special effect
special_effect *get_free_special_effect() {
	int i;
	//find the first free slot
	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (!sfx_markers[i].active) {
			return &sfx_markers[i];
		}
	}
	return NULL;	// all memory for special effects has been taken
}

// Initialize a new special effect
void add_sfx(int effect, Uint16 playerid, int caster)
{
	Uint8 str[70];
	actor *this_actor = get_actor_ptr_from_id(playerid);
	special_effect *m = get_free_special_effect();
	if (m == NULL) 
	{
		snprintf (str, sizeof (str), "Could not add special effect.  Increase NUMBER_OF_SPECIAL_EFFECTS.");	
		LOG_TO_CONSOLE (c_purple2, str);
		return;
	}
	if (this_actor == NULL ) return;
		
	// this switch is for differentiating static vs mobile effects
	switch (effect)
	{
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:
		case SPECIAL_EFFECT_HEAL_SUMMONED:
		case SPECIAL_EFFECT_INVASION_BEAMING:
		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			m->x = this_actor->x_tile_pos;		//static effects will not store a actor by convention
			m->y = this_actor->y_tile_pos;		// but we need to know where they were cast
			break;						
		default:								// all others are movable effects
			m->owner = this_actor;				//let sfx_marker know who is target of effect
			m->x = m->owner->x_tile_pos;		// NOTE: x_tile_pos is 2x x_pos (and same for y)
			m->y = m->owner->y_tile_pos;
			break;
	}

	m->type = effect;
	m->last_time = cur_time;					//global cur_time

	// this switch is for setting different effect lengths
	switch (effect)
	{
		case SPECIAL_EFFECT_HEAL:
		case SPECIAL_EFFECT_RESTORATION:
			m->timeleft = SPECIAL_EFFECT_RESTORATION_LIFESPAN;
			m->lifespan = SPECIAL_EFFECT_RESTORATION_LIFESPAN;
			break;
		case SPECIAL_EFFECT_SHIELD:
			m->timeleft = SPECIAL_EFFECT_SHIELD_LIFESPAN;
			m->lifespan = SPECIAL_EFFECT_SHIELD_LIFESPAN;
			break;
		default:
			m->timeleft = SPECIAL_EFFECT_LIFESPAN;
			m->lifespan = SPECIAL_EFFECT_LIFESPAN;
			break;
	}
	
	m->active = 1;
	m->caster = caster;							// should = 1 if caster of spell, 0 otherwise
}

//basic shape template that allows for rotation and duplication
void do_shape_spikes(float x, float y, float z, float center_offset_x, float center_offset_y, float base_offset_z, float a)
{
	int i;
	
	//save the world
	glPushMatrix();
		glTranslatef(x,y,z);

		glRotatef(270.0f*a, 0.0f, 0.0f, 1.0f);

		//now create eight copies of the object, each separated by 45 degrees
		for (i = 0; i < 8; i++)
		{
			glRotatef(45.f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, base_offset_z);
			glEnd();
		}
	//return to the world
	glPopMatrix();
}

//example halos moving in opposite directions, not yet optimized, and still just an example
void do_double_spikes(float x, float y, float z, float center_offset_x, float center_offset_y, float base_offset_z, float a)
{
	int i;
	
	//save the world
	glPushMatrix();
		glTranslatef(x,y,z);

		glRotatef(270.0f*a, 0.0f, 0.0f, 1.0f);

		//now create eight copies of the object, each separated by 45 degrees
		for (i = 0; i < 8; i++)
		{
			glRotatef(45.f, 0.0f, 0.0f, 1.0f);
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, 0.5+base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5+base_offset_z);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 1.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 0.0f*dx - center_offset_x,  - 0.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 1.0f*dy - center_offset_y, 0.5-base_offset_z);
			glVertex3f( - 2.0f*dx - center_offset_x,  - 2.0f*dy - center_offset_y, 0.5-base_offset_z);
			glEnd();
		}
	//return to the world
	glPopMatrix();
}

void draw_heal_effect(float x, float y, float z, float age)
{
	float center_offset_y = TILESIZE_Y * 0.7f;
	float top_z = 0;
	int i = 0;
	int elementsDrawn = 0;
	float final_z = 0;
	float d_y = 0;
	float ageAsc = 1.0f - age;

//save the world
	glPushMatrix();
		glTranslatef(x,y,z);

		if (age > 0.5f)
		{
			// Move up
			glRotatef(980.0f * ageAsc * 2.0f, 0.0f, 0.0f, 1.0f);
		
			elementsDrawn = 200 * ageAsc * 2.0f;
		
			top_z = 2.0f * ageAsc * 2.0f;
			
			d_y = 0.30f - (0.30f * ageAsc * 2.0f);
		}
		else
		{
			// Roll back
			glRotatef(980.0f, 0.0f, 0.0f, 1.0f);
		
			elementsDrawn = 200 * (1.0f - ageAsc) * 2.0f;
		
			top_z = 2.0f;
			
			d_y = 0.0f;
		}

		for (i = 0; i < elementsDrawn; i++)
		{
				top_z -= 0.01;
				d_y += 0.0015f;

				final_z = top_z;
				final_z -= (0.5f - ((float)rand() / (float)RAND_MAX)) / 6.0f;

				glRotatef(-4.9, 0.0f, 0.0f, 1.0f);

				// Draw crystal
				glBegin(GL_TRIANGLE_FAN);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glVertex3f(0, center_offset_y + d_y, final_z);
				glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
				glVertex3f(0.0f, center_offset_y + d_y, final_z + 0.06f);
				glVertex3f(0.02f, center_offset_y + d_y, final_z);
				glVertex3f(0.0f, center_offset_y + d_y, final_z - 0.06f);
				glVertex3f(-0.02f, center_offset_y + d_y, final_z);
				glVertex3f(0.0f, center_offset_y + d_y, final_z + 0.06f);
				glEnd();
		}

	//return to the world
	glPopMatrix();
}

void draw_restoration_effect(float x, float y, float z, float age)
{
	float top_z = 0;
	int i = 0;
	float final_z = 0;
	float d_y = 0;
	float ageAsc = 1.0f - age;
	float alpha = 1.0f;

	//save the world
		glPushMatrix();
		glTranslatef(x,y,z);

		if (age > 0.5f)
		{
			// Move up
			top_z = 2.0f * ageAsc * 2.0f;
			alpha = 1.0f;
		}
		else
		{
			// Hover				
			top_z = 2.0f;
			alpha = 2.0f - (ageAsc * 2.0f);
		}

		for (i = 0; i < 200; i++)
		{
				d_y = 0.4f + (0.2f * (float)rand() / (float)RAND_MAX);
				final_z = top_z;
				final_z -= (0.5f - ((float)rand() / (float)RAND_MAX)) / 15.0f;

				glRotatef(-1.8 /*360 / elementsDrawn = 100*/, 0.0f, 0.0f, 1.0f);
				
				// Draw crystal ring
				glBegin(GL_TRIANGLE_FAN);
				glColor4f(1.0f, 1.0f, 1.0f, alpha);
				glVertex3f(0, d_y, final_z);
				glColor4f(0.0f, 0.0f, 1.0f, alpha / 2.0f);
				glVertex3f(0.0f, d_y, final_z + 0.06f);
				glVertex3f(0.02f, d_y, final_z);
				glVertex3f(0.0f, d_y, final_z - 0.06f);
				glVertex3f(-0.02f, d_y, final_z);
				glVertex3f(0.0f, d_y, final_z + 0.06f);
				glEnd();

				if (i % 4 == 0) // Every n-th
				{
					final_z = top_z * ((float)rand() / (float)RAND_MAX);
					// Draw crystall fallout
					glBegin(GL_TRIANGLE_FAN);
					glColor4f(1.0f, 1.0f, 1.0f, alpha);
					glVertex3f(0, d_y, final_z);
					glColor4f(0.0f, 0.0f, 1.0f, alpha / 2.0f);
					glVertex3f(0.0f, d_y, final_z + 0.06f);
					glVertex3f(0.02f, d_y, final_z);
					glVertex3f(0.0f, d_y, final_z - 0.06f);
					glVertex3f(-0.02f, d_y, final_z);
					glVertex3f(0.0f, d_y, final_z + 0.06f);
					glEnd();
				}
		}
	//return to the world
	glPopMatrix();
}

void draw_teleport_effect(float x, float y, float z, float age)
{
	//adapted from RedBook
    float theta, phi, theta1, cosTheta, sinTheta, cosTheta1, sinTheta1;
    float ringDelta, sideDelta, cosPhi, sinPhi, dist, alpha;
	float TubeRadius, Radius; 
	
	//could use LOD here
	int sides = 6;
	int rings = 12;
	int h,i,j;
	float z_trans;
	GLuint TorusDL;
	
	sideDelta = 2.0 * 3.14159 / sides;
	ringDelta = 2.0 * 3.14159 / rings;
	theta = 0.0f;
	cosTheta = 1.0f;
	sinTheta = 0.0f;
	
	z_trans = z - 4*age + 4; //empirically, about height of actor
	
	TorusDL = glGenLists(1);
	glNewList(TorusDL, GL_COMPILE);
    
	glPushMatrix();
		glTranslatef(x,y,z_trans);
		// Make 3 tubes, would be nice if we could generalize alpha equations to have
		//  any number of tubes
		for (h = -2; h < 1; h++)
		{
			if (h == 0)								// set to fade each in and out
				if (age >= 0.5f)
					alpha = -8*(age*age) + 12*age - 4.0f;
				else
					alpha = 0.0f;
			else if (h == -1)
				if ((age <= 0.75f) && (age > 0.25f))
					alpha = -8*(age*age) + 8*age - 1.5f;
				else
					alpha = 0.0f;
			else if (h == -2)
				if (age <= 0.5f)
					alpha = -8*(age*age) + 4*age;
				else
					alpha = 0.0f;
			else
				alpha = 0.0f;	//should not get here, but make sure nothing would display anyhow
			
			TubeRadius = alpha * TILESIZE_X/16 * 2;		// adjust radii based on quadratic function too
			Radius = alpha * TILESIZE_X/1.3f * 2;		// and according to age
			
			//only display tubes if aboveground
			if (z_trans + h >= z)
				for (i = 0; i < rings; i++)
				{
					theta1 = theta + ringDelta;
					cosTheta1 = cos(theta1);
					sinTheta1 = sin(theta1);
					glBegin(GL_QUAD_STRIP);
						// set our fade in and out with color shift
						glColor4f(age, alpha * 2, 1.0f, alpha);
						phi = 0.0;
						for (j = 0; j < sides; j++)
						{
							phi = phi + sideDelta;
							cosPhi = cos(phi);
							sinPhi = sin(phi);
							dist = Radius + (TubeRadius * cosPhi);
		
							glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi + 1*h);
							glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, TubeRadius * sinPhi + 1*h);
		
							glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi + 1*h);
							glVertex3f(cosTheta * dist, -sinTheta * dist, TubeRadius * sinPhi + 1*h);
						}
					glEnd();
					theta = theta1;
					cosTheta = cosTheta1;
					sinTheta = sinTheta1;
				}
		}
		glEndList();
		glCallList(TorusDL);
	glPopMatrix();
}

void display_special_effect(special_effect *marker) {
	
	// (a) varies from 1..0 depending on the age of this marker
	const float a = ((float)marker->timeleft) / ((float)marker->lifespan);

	// x and y are the location for the effect
	//	center_offset_x&y are for radial distance from actor in ground plane
	//	base_offset_z is for height off the ground (z)
	float x,y,center_offset_x, center_offset_y, base_offset_z;
	
	// height of terrain at the effect's location
	float z = get_tile_display_height(marker->x, marker->y);

	// place x,y in the center of the actor's tile
	switch (marker->type)
	{
		case SPECIAL_EFFECT_SMITE_SUMMONINGS:			// group "static" tile-based effects
		case SPECIAL_EFFECT_HEAL_SUMMONED:
		case SPECIAL_EFFECT_INVASION_BEAMING:
		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			x= (float)marker->x/2 + (TILESIZE_X / 2);	// "static" tile based effects
			y= (float)marker->y/2 + (TILESIZE_Y / 2);	// "static" tile based effects
			break;						
		default:										// all others are movable effects
			x = marker->owner->x_pos + (TILESIZE_X / 2);	// movable effects need current position
			y = marker->owner->y_pos + (TILESIZE_X / 2);
			break;
	}
		
	switch (marker->type) {
/*		case SPECIAL_EFFECT_SMITE_SUMMONINGS:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));	//fast expanding
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			base_offset_z = z + a*0.3f;						//drop toward ground
			glColor4f(1.0f, 0.0f, 0.0f, a);
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_HEAL_SUMMONED:
			center_offset_x = ((TILESIZE_X / 2) / (a*a));
			center_offset_y = ((TILESIZE_X / 2) / (a*a));
			base_offset_z = z + a*0.3f;						//drop toward ground
			glColor4f(0.0f, 0.0f, 1.0f, a);
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_INVASION_BEAMING:

		case SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			draw_teleport_effect(x,y,z,a);
			break;
*/
		case SPECIAL_EFFECT_HEAL:
			draw_heal_effect(x,y,z,a);						//Kindar Naar's effect
			break;
		case SPECIAL_EFFECT_RESTORATION:
			draw_restoration_effect(x,y,z,a);
			break;
/*		case SPECIAL_EFFECT_REMOTE_HEAL:
			center_offset_x = ((TILESIZE_X / 2) * (a*a));
			center_offset_y = ((TILESIZE_X / 2) * (a*a));
			if (a > 0) base_offset_z = z + 1.5/(a+.5) - 1;	//beam up effect
			if (marker->caster)
				glColor4f(0.0f, 0.0f, 1.0f, a);				//caster
			else
				glColor4f(0.0f, 1.0f, 0.0f, a);				//recipient
			do_shape_spikes(x, y, z, center_offset_x, center_offset_y, base_offset_z, a);
			break;
		case SPECIAL_EFFECT_SHIELD:
			draw_shield_effect(x,y,z,a);					
			break;
*/
		default: // for all the spells we have not gotten to yet
			break;
	}

}

void display_special_effects() {
	int i; 

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_ALPHA_TEST);

	for(i = 0; i < NUMBER_OF_SPECIAL_EFFECTS; i++) {
		if (sfx_markers[i].active) {
			sfx_markers[i].timeleft -= (cur_time - sfx_markers[i].last_time); //use global cur_time
			if (sfx_markers[i].timeleft > 0) {
				sfx_markers[i].last_time = cur_time;
				display_special_effect(&sfx_markers[i]);
			} else {
				// This marker has lived long enough now.
				sfx_markers[i].active = 0;
			}
		}
	}

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

//send server data packet to appropriate method depending on desired effect
void parse_special_effect(int sfx, const Uint16 *data)
{
	Uint8 str[100];
	int offset = 0;
	Uint16 var_a, var_b =0;
	
	switch(sfx){
		//player only
		case	SPECIAL_EFFECT_SHIELD:
		case	SPECIAL_EFFECT_RESTORATION:
		case	SPECIAL_EFFECT_SMITE_SUMMONINGS:
		case	SPECIAL_EFFECT_CLOAK:
		case	SPECIAL_EFFECT_DECLOAK:
		case	SPECIAL_EFFECT_HEAL_SUMMONED:
		case	SPECIAL_EFFECT_HEAL:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				add_sfx(sfx,var_a,1);
			}
			break;
		//player to player, var_a is caster, var_b is recipient/target
		case	SPECIAL_EFFECT_POISON:
		case	SPECIAL_EFFECT_REMOTE_HEAL:
		case	SPECIAL_EFFECT_HARM:
		case	SPECIAL_EFFECT_MANA_DRAIN:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				var_b = SDL_SwapLE16 (*((Uint16 *)(&data[offset+1])));
				add_sfx(sfx,var_a,1); //caster
				add_sfx(sfx,var_b,0); //target
			}
			break;
		//location (a&b variable are not known until implemented by server)
		case	SPECIAL_EFFECT_INVASION_BEAMING:
		case	SPECIAL_EFFECT_TELEPORT_TO_RANGE:
			{
				var_a = SDL_SwapLE16 (*((Uint16 *)(&data[offset])));
				var_b = SDL_SwapLE16 (*((Uint16 *)(&data[offset+1])));
#ifdef DEBUG
				snprintf (str, sizeof (str), "effect %d,  x pos=%d, y pos=%d",sfx,var_a,var_b);	
				LOG_TO_CONSOLE (c_purple2, str);
#endif
			}
			break;
		default:
#ifdef DEBUG
			snprintf (str, sizeof (str), " SPECIAL_EFFECT_unknown:%d",sfx);
			LOG_TO_CONSOLE (c_purple2, str);
#endif
			break;
	}
}

#endif //SFX