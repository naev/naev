

#include "pilot.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "libxml/parser.h"

#include "main.h"
#include "log.h"
#include "weapon.h"
#include "pack.h"


#define XML_NODE_START  1
#define XML_NODE_TEXT   3

#define XML_ID				"Fleets"  /* XML section identifier */
#define XML_FLEET			"fleet"

#define FLEET_DATA		"dat/fleet.xml"


/* stack of pilot ids to assure uniqueness */
static unsigned int pilot_id = 0;


/* stack of pilots */
Pilot** pilot_stack = NULL; /* not static, used in player.c and weapon.c */
int pilots = 0; /* same */
extern Pilot* player;

/* stack of fleets */
static Fleet* fleet_stack = NULL;
static int nfleets = 0;


/*
 * prototyes
 */
/* external */
extern void ai_destroy( Pilot* p ); /* ai.c */
extern void player_think( Pilot* pilot ); /* player.c */
extern void ai_think( Pilot* pilot ); /* ai.c */
/* internal */
static void pilot_update( Pilot* pilot, const double dt );
void pilot_render( Pilot* pilot ); /* externed in player.c */
static void pilot_free( Pilot* p );
static Fleet* fleet_parse( const xmlNodePtr parent );


/*
 * gets the next pilot based on player_id
 */
unsigned int pilot_getNext( const unsigned int id )
{
/* Regular Search */
	int i;
	for ( i=0; i < pilots; i++ )
		if (pilot_stack[i]->id == id)
			break;

/* Dichotomical search */
	/*int i,n;
	for (i=0, n=pilots/2; n > 0; n /= 2 ) 
		i += (pilot_stack[i+n]->id > id) ? 0 : n ;*/

	if (i==pilots-1) return 0;

	return pilot_stack[i+1]->id;
}


/*
 * pulls a pilot out of the pilot_stack based on id
 */
Pilot* pilot_get( const unsigned int id )
{
/* Regular search */
	int i;
	for ( i=0; i < pilots; i++ )
		if (pilot_stack[i]->id == id)
			return pilot_stack[i];
	return NULL;

	if (id==0) return player;

	DEBUG("id=%d",id);

/* Dichotomical search */
	/*int i,n;
	for (i=0, n=pilots/2; n > 0; n /= 2 )
		i += (pilot_stack[i+n]->id > id) ? 0 : n ;

	return (pilot_stack[i]->id == id) ? pilot_stack[i] : NULL ;*/
}


/*
 * makes the pilot shoot
 *
 * @param p the pilot which is shooting
 * @param secondary whether they are shooting secondary weapons or primary weapons
 */
void pilot_shoot( Pilot* p, const int secondary )
{
	int i;
	if (!secondary) { /* primary weapons */

		if (!p->outfits) return; /* no outfits */

		for (i=0; i<p->noutfits; i++) /* cycles through outfits to find weapons */
			if (outfit_isWeapon(p->outfits[i].outfit) || /* is a weapon or launche */
					outfit_isLauncher(p->outfits[i].outfit))
				/* ready to shoot again */
				if ((SDL_GetTicks()-p->outfits[i].timer) >
						(p->outfits[i].outfit->delay/p->outfits[i].quantity))

					/* different weapons, different behaviours */
					switch (p->outfits[i].outfit->type) {
						case OUTFIT_TYPE_BOLT:
							weapon_add( p->outfits[i].outfit, p->solid->dir,
									&p->solid->pos, &p->solid->vel, p->id,
									(p==player) ? WEAPON_LAYER_FG : WEAPON_LAYER_BG );
							p->outfits[i].timer = SDL_GetTicks();
							break;

						default:
							break;
					}
	}
}


/*
 * damages the pilot
 */
void pilot_hit( Pilot* p, const double damage_shield, const double damage_armor )
{
	if (p->shield-damage_shield > 0.)
		p->shield -= damage_shield;
	else if (p->shield > 0.) { /* shields can take part of the blow */
		p->armor -= p->shield/damage_shield*damage_armor;
		p->shield = 0.;
	}
	else if (p->armor-damage_armor > 0.)
		p->armor -= damage_armor;
	else
		p->armor = 0.;
}


/*
 * renders the pilot
 */
void pilot_render( Pilot* p )
{
	int sx,sy;

	/* get the sprite corresponding to the direction facing */
	gl_getSpriteFromDir( &sx, &sy, p->ship->gfx_space, p->solid->dir );

	gl_blitSprite( p->ship->gfx_space, &p->solid->pos, sx, sy );
}


/*
 * updates the Pilot
 */
static void pilot_update( Pilot* pilot, const double dt )
{
	/* regeneration */
	if (pilot->armor < pilot->armor_max)
		pilot->armor += pilot->ship->armor_regen * dt;
	else
		pilot->shield += pilot->ship->shield_regen * dt;
	
	if (pilot->armor > pilot->armor_max) pilot->armor = pilot->armor_max;
	if (pilot->shield > pilot->shield_max) pilot->shield = pilot->shield_max;

	if ((pilot->solid->dir > 2.*M_PI) || (pilot->solid->dir < 0.0))
		pilot->solid->dir = fmod(pilot->solid->dir,2.*M_PI);

	/* update the solid */
	pilot->solid->update( pilot->solid, dt );

	if (VMOD(pilot->solid->vel) > pilot->ship->speed) /* shouldn't go faster */
		vect_pset( &pilot->solid->vel, pilot->ship->speed, VANGLE(pilot->solid->vel) );

	pilot_render( pilot );
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
		pilot->id = 0;
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

	/* max shields/armor */
	pilot->armor_max = ship->armor;
	pilot->shield_max = ship->shield;
	pilot->energy_max = ship->energy;
	pilot->armor = pilot->armor_max;
	pilot->shield = pilot->shield_max;
	pilot->energy = pilot->energy_max;

	/* initially idle */
	pilot->task = NULL;

	/* outfits */
	pilot->outfits = NULL;
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
		}
		pilot_stack[0] = dyn;
	}
	else { /* add to the stack */
		pilot_stack = realloc( pilot_stack, ++pilots*sizeof(Pilot*) );
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
}


/*
 * updates all the pilots
 */
void pilots_update( double dt )
{
	int i;
	for ( i=0; i < pilots; i++ ) {
		if ( pilot_stack[i]->think)
			pilot_stack[i]->think(pilot_stack[i]);
		if (pilot_stack[i]->update)
			pilot_stack[i]->update( pilot_stack[i], dt );
		if (pilot_stack[i]->render)
			pilot_stack[i]->render(pilot_stack[i]);
	}
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

	while ((node = node->next)) { /* load all the data */
		if (strcmp((char*)node->name,"faction")==0)
			temp->faction = faction_get((char*)node->children->content);
		else if (strcmp((char*)node->name,"ai")==0)
			temp->ai = ai_getProfile((char*)node->children->content);
		else if (strcmp((char*)node->name,"pilots")==0) {
			cur = node->children;     
			while ((cur = cur->next)) {
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
			}
		}
	}

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
	}
	nfleets = 0;
}
