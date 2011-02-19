/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef GUI_OMSG_H
#  define GUI_OMSG_H



#define OMSG_FONT_DEFAULT_SIZE      16
#define OMSG_FONT_DEFAULT_PATH      "dat/mono.ttf"


/*
 * Creation and management.
 */
unsigned int omsg_add( const char *msg, double duration, int fontsize );
int omsg_change( unsigned int id, const char *msg, double duration );
int omsg_exists( unsigned int id );
void omsg_rm( unsigned int id );

/*
 * Global stuff.
 */
void omsg_position( double center_x, double center_y, double width );
void omsg_cleanup (void);
void omsg_render( double dt );


#endif /* GUI_OMSG_H */
