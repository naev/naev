/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NEBULA_H
#  define NEBULA_H


/*
 * Init/Exit
 */
int nebu_init (void);
void nebu_vbo_init (void);
void nebu_exit (void);

/*
 * Render
 */
void nebu_render( const double dt );
void nebu_renderOverlay( const double dt );

/*
 * Misc
 */
int nebu_isLoaded (void);
void nebu_genOverlay (void);
double nebu_getSightRadius (void);
void nebu_prep( double density, double volatility );
void nebu_forceGenerate (void);
void nebu_movePuffs( double x, double y );


#endif /* NEBULA_H */
