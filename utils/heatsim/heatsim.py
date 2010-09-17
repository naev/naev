#!/usr/bin/env python

#######################################################
#
#        SIM INFO
#
#######################################################
# Ship info
ship_mass   = 83. # Mass of ship
# Weapon info
weap_num    = 2 # Number of weapons
weap_mass   = 2. # Mass of weapon
weap_delay  = 0.9 # Delay between weapon shots
weap_energy = 4.25 # Energy weapon uses
# Sim parameters
STEFAN_BOLZMANN = 5.67e-8
SPACE_TEMP  = 250.
STEEL_COND  = 54.
STEEL_CAP   = 0.49
STEEL_DENS  = 7.88e3
# Sim info
sim_dt      = 1./50. # Delta tick
sim_total   = 100. # Time in seconds


#######################################################
#
#        SIM CODE
#
#######################################################

# Imports
from frange import *
import math

# Calculate ship parameters
ship_kg     = ship_mass * 1000.
ship_emis   = 0.8
ship_cond   = STEEL_COND
ship_C      = STEEL_CAP * ship_kg
ship_area   = pow( ship_kg / STEEL_DENS, 2./3. )
ship_T      = SPACE_TEMP

# Calculate weapon parameters
weap_kg     = weap_mass * 1000.
weap_C      = STEEL_CAP * weap_kg
weap_area   = pow( weap_kg / STEEL_DENS, 2./3. )
weap_list   = []
weap_T      = []
for i in range(weap_num):
   weap_list.append( i*weap_delay / weap_num )
   weap_T.append( SPACE_TEMP )

# Log
print
print("Starting NAEV HeatSim")
print
print("Ship:")
print("  C: " + str(ship_C) + " J/K")
print("  area: " + str(ship_area) + " m^2")
print
print("Weap:")
print("  C: " + str(weap_C) + " J/K")
print("  area: " + str(weap_area) + " m^2")
print
print("Starting...")

dt          = sim_dt
sim_elapsed = 0.
while sim_elapsed < sim_total:

   Q_cond = 0.

   # Check weapons
   for i in range(len(weap_list)):
      weap_list[i] -= dt

      # Check if shot
      if weap_list[i] < 0.:
         weap_T[i]     += 1000. * weap_energy / weap_C
         weap_list[i]  += weap_delay

      # Do heat movement (conduction)
      Q           = -ship_cond * (weap_T[i] - ship_T) * weap_area * dt
      weap_T[i]  += Q / weap_C
      Q_cond     += Q

   # Do ship heat (radiation)
   Q_rad    = STEFAN_BOLZMANN * ship_area * ship_emis * (pow(SPACE_TEMP,4.) - pow(ship_T,4.)) * dt
   Q        = Q_rad - Q_cond
   ship_T  += Q / ship_C

   # Elapsed time
   sim_elapsed += dt;


print("Finished!")
print
print("Ship Temp: "+str(ship_T)+" K")
for i in range(len(weap_list)):
   print("Outfit["+str(i)+"] Temp: "+str(weap_T[i])+" K")

