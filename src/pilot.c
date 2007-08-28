

#include "pilot.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "xml.h"

#include "main.h"
#include "log.h"
#include "weapon.h"
#include "pack.h"


#define XML_ID				"Fleets"  /* XML section identifier */
#define XML_FLEET			"fleet"

#define FLEET_DATA		"dat/fleet.xml"


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = PLAYER_ID;


/* stack of pilots */
Pilot** pilot_stack = NULL; /* not static, used in player.c, weapon.c, pause.c and ai.c */
int pilots = 0; /* same */
static int mpilots = 0;
extern Pilot* player;

/* stack of fleets */
static Fleet* fleet_stack = NULL;
static int nfleets = 0;


/*
 * prototyes
 */
/* external */
extern void ai_destroy( Pilot* p ); /* ai.c */
extern void ai_think( Pilot* pilot ); /* ai.c */
extern void player_think( Pilot* pilot ); /* player.c */
extern int gui_load( const char *name ); /* player.c */
/* internal */
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w, const unsigned int t );
static void pilot_update( Pilot* pilot, const double dt );
void pilot_render( Pilot* pilot ); /* externed in player.c */
static void pilot_free( Pilot* p );
static Fleet* fleet_parse( const xmlNodePtr parent );


/*
 * gets the next pilot based on id
 */
unsigned int pilot_getNext( const unsigned int id )
{
	/* binary search */
	int l,m,h;
	l = 0;
	h = pilots-1;
	while (l <= h) {
		m = (l+h)/2;
		if (pilot_stack[m]->id > id) h = m-1;
		else if (pilot_stack[m]->id < id) l = m+1;
		else break;
	}

	if (m == (pilots-1)) return PLAYER_ID;
	else return pilot_stack[m+1]->id;
}


/*
 * gets the nearest enemy to the pilot
 */
unsigned int pilot_getNearest( const Pilot* p )
{
	unsigned int tp;
	int i;
	double d, td;
	for (tp=0,d=0.,i=0; i<pilots; i++)
		if (areEnemies(p->faction, pilot_stack[i]->faction)) {
			td = vect_dist(&pilot_stack[i]->solid->pos, &p->solid->pos);
			if (!pilot_isDisabled(pilot_stack[i]) &&  ((!tp) || (td < d))) {
				d = td;
				tp = pilot_stack[i]->id;
			}
		}
	return tp;
}


/*
 * gets the nearest hostile enemy to the player
 */
unsigned pilot_getHostile (void)
{
	unsigned int tp;
	int i;                                                                 
	double d, td;
	for (tp=PLAYER_ID,d=0.,i=0; i<pilots; i++)
		if (pilot_isFlag(pilot_stack[i],PILOT_HOSTILE)) {
			td = vect_dist(&pilot_stack[i]->solid->pos, &player->solid->pos);
			if ((tp==PLAYER_ID) || (td < d)) {
				d = td;
				tp = pilot_stack[i]->id;
			}
		}
	return tp;
}


/*
 * pulls a pilot out of the pilot_stack based on id
 */
Pilot* pilot_get( const unsigned int id )
{
	if (id==PLAYER_ID) return player; /* special case player */
	
	/* binary */
	int l,m,h;
	l = 0;
	h = pilots-1;
	while (l <= h) {
		m = (l+h)/2;
		if (pilot_stack[m]->id > id) h = m-1;
		else if (pilot_stack[m]->id < id) l = m+1;
		else return pilot_stack[m];
	}
	return NULL;
}


/*
 * makes the pilot shoot
 *
 * @param p the pilot which is shooting
 * @param secondary whether they are shooting secondary weapons or primary weapons
 */
void pilot_shoot( Pilot* p, const unsigned int target, const int secondary )
{
	int i;

	if (!p->outfits) return; /* no outfits */

	if (!secondary) { /* primary weapons */

		for (i=0; i<p->noutfits; i++) /* cycles through outfits to find primary weapons */
			if (!outfit_isProp(p->outfits[i].outfit,OUTFIT_PROP_WEAP_SECONDARY))
				pilot_shootWeapon( p, &p->outfits[i], target );
	}
	else { /* secondary weapon */

		if (!p->secondary) return; /* no secondary weapon */
		pilot_shootWeapon( p, p->secondary, target );

	}
}
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w, const unsigned int t )
{
	/* will segfault when trying to launch with 0 ammo otherwise */
	int quantity = (outfit_isAmmo(w->outfit) && p->secondary) ?
			p->secondary->quantity : w->quantity ;
	
	/* check to see if weapon is ready */
	if ((SDL_GetTicks() - w->timer) < (w->outfit->delay / quantity)) return;

	/*
	 * regular weapons
	 */
	if (outfit_isWeapon(w->outfit)) {
		
		/* different weapons, different behaviours */
		switch (w->outfit->type) {
			case OUTFIT_TYPE_BOLT:
				weapon_add( w->outfit, p->solid->dir,
						&p->solid->pos, &p->solid->vel, p->id, t );

				/* can't shoot it for a bit */      
				w->timer = SDL_GetTicks();
				break;

			default:
				break;
		}

	}

	/*
	 * missile launchers
	 *
	 * @must be a secondary weapon
	 * @shooter can't be the target - sanity check for the player
	 */
	else if (outfit_isLauncher(w->outfit) && (w==p->secondary) && (p->id!=t)) {
		if (p->ammo && (p->ammo->quantity > 0)) {

			weapon_add( p->ammo->outfit, p->solid->dir,
					&p->solid->pos, &p->solid->vel, p->id, t );

			w->timer = SDL_GetTicks(); /* can't shoot it for a bit */
			p->ammo->quantity -= 1; /* we just shot it */
		}
	}
}


/*
 * damages the pilot
 */
void pilot_hit( Pilot* p, const double damage_shield, const double damage_armour )
{
	if (p->shield-damage_shield > 0.)
		p->shield -= damage_shield;
	else if (p->shield > 0.) { /* shields can take part of the blow */
		p->armour -= p->shield/damage_shield*damage_armour;
		p->shield = 0.;
	}
	else if (p->armour-damage_armour > 0.)
		p->armour -= damage_armour;
	else
		p->armour = 0.;
}


/*
 * sets the pilot's ammo based on their secondary weapon
 */
void pilot_setAmmo( Pilot* p )
{
	int i;
	char *name;

	if ((p->secondary == NULL) || !outfit_isLauncher(p->secondary->outfit)) {
		p->ammo = NULL;
		return;
	}

	name = p->secondary->outfit->ammo;

	for (i=0; i<p->noutfits; i++)
		if (strcmp(p->outfits[i].outfit->name,name)==0) {
			p->ammo = p->outfits + i;
			return;
		}

	p->ammo = NULL;
}


/*
 * renders the pilot
 */
void pilot_render( Pilot* p )
{
	int sx,sy;

	/* get the sprite corresponding to the direction facing */
	gl_getSpriteFromDir( &sx, &sy, p->ship->gfx_space, p->solid->dir );

	gl_blitSprite( p->ship->gfx_space,
			p->solid->pos.x, p->solid->pos.y, sx, sy, NULL );
}


/*
 * updates the Pilot
 */
static void pilot_update( Pilot* pilot, const double dt )
{
	if ((pilot != player) && 
			(pilot->armour < PILOT_DISABLED_ARMOR*pilot->armour_max)) { /* disabled */
		pilot_setFlag(pilot,PILOT_DISABLED);
		vect_pset( &pilot->solid->vel, /* slowly brake */
			VMOD(pilot->solid->vel) * (1. - dt*0.10),
			VANGLE(pilot->solid->vel) );
		vectnull( &pilot->solid->force );
		pilot->solid->dir_vel = 0.; /* no more turning */

		/* update the solid */
		pilot->solid->update( pilot->solid, dt );
		return;
	}

	/* still alive */
	else if (pilot->armour < pilot->armour_max)
		pilot->armour += pilot->ship->armour_regen * dt;
	else
		pilot->shield += pilot->ship->shield_regen * dt;
	
	if (pilot->armour > pilot->armour_max) pilot->armour = pilot->armour_max;
	if (pilot->shield > pilot->shield_max) pilot->shield = pilot->shield_max;

	/* update the solid */
	(*pilot->solid->update)( pilot->solid, dt );

	if (VMOD(pilot->solid->vel) > pilot->ship->speed) /* shouldn't go faster */
		vect_pset( &pilot->solid->vel, pilot->ship->speed, VANGLE(pilot->solid->vel) );
}


/*
 * Initialize pilot
 *
 * @ ship : ship pilot will be flying
 * @ name : pilot's name, if NULL ship's name will be used
 * @ dir : initial direction to face (radians)
 * @ vel : initial velocity
 * @ pos : initial position
 * @ flags : used for tweaking the pilot
 */
void pilot_init( Pilot* pilot, Ship* ship, char* name, Faction* faction, AI_Profile* ai,
		const double dir, const Vector2d* pos, const Vector2d* vel, const int flags )
{
	if (flags & PILOT_PLAYER) /* player is ID 0 */
		pilot->id = PLAYER_ID;
	else
		pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

	pilot->ship = ship;
	pilot->name = strdup( (name==NULL) ? ship->name : name );

	/* faction */
	pilot->faction = faction;

	/* AI */
	pilot->ai = ai;
	pilot->tcontrol = 0;
	pilot->flags = 0;

	/* solid */
	pilot->solid = solid_create(ship->mass, dir, pos, vel);

	/* max shields/armour */
	pilot->armour_max = ship->armour;
	pilot->shield_max = ship->shield;
	pilot->energy_max = ship->energy;
	pilot->armour = pilot->armour_max;
	pilot->shield = pilot->shield_max;
	pilot->energy = pilot->energy_max;

	/* initially idle */
	pilot->task = NULL;

	/* outfits */
	pilot->outfits = NULL;
	pilot->secondary = NULL;
	pilot->ammo = NULL;
	ShipOutfit* so;
	if (ship->outfit) {
		pilot->noutfits = 0;
		for (so=ship->outfit; so; so=so->next) {
			pilot->outfits = realloc(pilot->outfits, (pilot->noutfits+1)*sizeof(PilotOutfit));
			pilot->outfits[pilot->noutfits].outfit = so->data;
			pilot->outfits[pilot->noutfits].quantity = so->quantity;
			pilot->outfits[pilot->noutfits].timer = 0;
			(pilot->noutfits)++;
		}
	}

	if (flags & PILOT_PLAYER) {
		pilot->think = player_think; /* players don't need to think! :P */
		pilot->render = NULL;
		pilot_setFlag(pilot,PILOT_PLAYER); /* it is a player! */
		player = pilot;
		gui_load( pilot->ship->gui ); /* load the gui */
	}
	else {
		pilot->think = ai_think;
		pilot->render = pilot_render;
	}

	pilot->update = pilot_update;
}


/*
 * Creates a new pilot
 *
 * see pilot_init for parameters
 *
 * returns pilot's id
 */
unsigned int pilot_create( Ship* ship, char* name, Faction* faction, AI_Profile* ai,
		const double dir, const Vector2d* pos, const Vector2d* vel, const int flags )
{
	Pilot* dyn = MALLOC_ONE(Pilot);
	if (dyn == NULL) {
		WARN("Unable to allocate memory");
		return 0;;
	}
	pilot_init( dyn, ship, name, faction, ai, dir, pos, vel, flags );

	if (flags & PILOT_PLAYER) { /* player */
		if (!pilot_stack) {
			pilot_stack = MALLOC_ONE(Pilot*);
			pilots = 1;
			mpilots = 1;
		}
		pilot_stack[0] = dyn;
	}
	else { /* add to the stack */

		pilots++; /* there's a new pilot */

		if (pilots >= mpilots) /* needs to grow */
			pilot_stack = realloc( pilot_stack, ++mpilots*sizeof(Pilot*) );

		pilot_stack[pilots-1] = dyn;
	}

	return dyn->id;
}


/*
 * frees and cleans up a pilot
 */
static void pilot_free( Pilot* p )
{
	solid_free(p->solid);
	free(p->outfits);
	free(p->name);
	ai_destroy(p);
	free(p);
}


/*
 * destroys pilot from stack
 */
void pilot_destroy(Pilot* p)
{
	int i;

	for (i=0; i < pilots; i++)
		if (pilot_stack[i]==p)
			break;

	while (i < pilots) {
		pilot_stack[i] = pilot_stack[i+1];
		i++;
	}

	pilot_free(p);
}


/*
 * frees the pilots
 */
void pilots_free (void)
{
	int i;
	for (i=0; i < pilots; i++)
		pilot_free(pilot_stack[i]);
	free(pilot_stack);
	pilot_stack = NULL;
	pilots = 0;
}


/*
 * cleans up the pilots - leaves the player
 */
void pilots_clean (void)
{
	int i;
	for (i=1; i < pilots; i++)
		pilot_free(pilot_stack[i]);
	pilots = 1;
}


/*
 * updates all the pilots
 */
void pilots_update( double dt )
{
	int i;
	for ( i=0; i < pilots; i++ ) {
		if (pilot_stack[i]->think && /* think */
				!pilot_isDisabled(pilot_stack[i]))
			pilot_stack[i]->think(pilot_stack[i]);
		if (pilot_stack[i]->update) /* update */
			pilot_stack[i]->update( pilot_stack[i], dt );
	}
}


/*
 * renders all the pilots
 */
void pilots_render (void)
{
	int i;
	for (i=1; i<pilots; i++) /* skip player */
		if (pilot_stack[i]->render) /* render */
			pilot_stack[i]->render(pilot_stack[i]);
}


/* returns the fleet based on name */
Fleet* fleet_get( const char* name )
{
	int i;
	for (i=0; i<nfleets; i++)
		if (strcmp(name, fleet_stack[i].name)==0)
			return fleet_stack+i;
	return NULL;
}


/* parses the fleet node */
static Fleet* fleet_parse( const xmlNodePtr parent )
{
	xmlNodePtr cur, node;
	FleetPilot* pilot;
	char* c;
	node  = parent->xmlChildrenNode;

	Fleet* temp = CALLOC_ONE(Fleet);

	temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name"); /* already mallocs */
	if (temp->name == NULL) WARN("Fleet in "FLEET_DATA" has invalid or no name");

	do { /* load all the data */
		if (strcmp((char*)node->name,"faction")==0)
			temp->faction = faction_get((char*)node->children->content);
		else if (strcmp((char*)node->name,"ai")==0)
			temp->ai = ai_getProfile((char*)node->children->content);
		else if (strcmp((char*)node->name,"pilots")==0) {
			cur = node->children;     
			do {
				if (strcmp((char*)cur->name,"pilot")==0) {
					temp->npilots++; /* pilot count */
					pilot = MALLOC_ONE(FleetPilot);

					/* name is not obligatory, will only override ship name */
					c = (char*)xmlGetProp(cur,(xmlChar*)"name"); /* mallocs */
					pilot->name = c; /* no need to free here though */

					pilot->ship = ship_get((char*)cur->children->content);
					if (pilot->ship == NULL)
						WARN("Pilot %s in Fleet %s has null ship", pilot->name, temp->name);

					c = (char*)xmlGetProp(cur,(xmlChar*)"chance"); /* mallocs */
					pilot->chance = atoi(c);
					if (pilot->chance == 0)
						WARN("Pilot %s in Fleet %s has 0%% chance of appearing",
							pilot->name, temp->name );
					if (c) free(c); /* free the external malloc */

					/* memory silliness */
					temp->pilots = realloc(temp->pilots, sizeof(FleetPilot)*temp->npilots);
					memcpy(temp->pilots+(temp->npilots-1), pilot, sizeof(FleetPilot));
					free(pilot);
				}
			} while ((cur = cur->next));
		}
	} while ((node = node->next));

#define MELEMENT(o,s)      if ((o) == NULL) WARN("Fleet '%s' missing '"s"' element", temp->name)
	MELEMENT(temp->ai,"ai");
	MELEMENT(temp->faction,"faction");
	MELEMENT(temp->pilots,"pilots");
#undef MELEMENT

	return temp;
}


/* loads the fleets */
int fleet_load (void)
{
	uint32_t bufsize;
	char *buf = pack_readfile(DATA, FLEET_DATA, &bufsize);

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	Fleet* temp = NULL;

	node = doc->xmlChildrenNode; /* Ships node */
	if (strcmp((char*)node->name,XML_ID)) {
		ERR("Malformed "FLEET_DATA" file: missing root element '"XML_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first ship node */
	if (node == NULL) {
		ERR("Malformed "FLEET_DATA" file: does not contain elements");
		return -1;
	}

	do {  
		if (node->type ==XML_NODE_START &&         
				strcmp((char*)node->name,XML_FLEET)==0) {
			temp = fleet_parse(node);
			fleet_stack = realloc(fleet_stack, sizeof(Fleet)*(++nfleets));
			memcpy(fleet_stack+nfleets-1, temp, sizeof(Fleet));
			free(temp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	DEBUG("Loaded %d fleet%c", nfleets, (nfleets==1)?' ':'s');

	return 0;
}


/* frees the fleets */
void fleet_free (void)
{
	int i,j;
	if (fleet_stack != NULL) {
		for (i=0; i<nfleets; i++) {
			for (j=0; j<fleet_stack[i].npilots; j++)
				if (fleet_stack[i].pilots[j].name)
					free(fleet_stack[i].pilots[j].name);
			free(fleet_stack[i].name);
			free(fleet_stack[i].pilots);
		}
		free(fleet_stack);
		fleet_stack = NULL;
	}
	nfleets = 0;
}

