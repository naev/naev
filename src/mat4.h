/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct mat4_ {
   /* Column-major; m[x][y] */
   GLfloat m[4][4];
} mat4;

void mat4_Print( mat4 m );
__attribute__((const)) mat4 mat4_Mult( mat4 m1, mat4 m2 );
__attribute__((const)) mat4 mat4_Identity( void );
__attribute__((const)) mat4 mat4_Ortho( double left, double right,
                           double bottom, double top, double nearVal, double farVal );
__attribute__((const)) mat4 mat4_Scale( mat4 m, double x, double y, double z );
__attribute__((const)) mat4 mat4_Translate( mat4 m, double x, double y, double z );
__attribute__((const)) mat4 mat4_Rotate( mat4 m, double angle, double x, double y, double z );
__attribute__((const)) mat4 mat4_Rotate2d( mat4 m, double angle );
__attribute__((const)) mat4 mat4_Rotate2dv( mat4 m, double x, double y );
GLfloat *mat4_Ptr( mat4 *m );
void mat4_Uniform( GLint location, mat4 m );
