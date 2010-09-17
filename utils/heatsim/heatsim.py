#!/usr/bin/env python


#######################################################
#
#        SIM CODE
#
#######################################################

# Imports
from frange import *
import math


class heatsim:

   def __init__( self, shipname = "llama", weapname = "laser", sim_on = 60., sim_total = 120., filename=None ):
      # Sim parameters
      self.STEFAN_BOLZMANN = 5.67e-8
      self.SPACE_TEMP  = 250.
      self.STEEL_COND  = 54.
      self.STEEL_CAP   = 0.49
      self.STEEL_DENS  = 7.88e3
      # Sim info
      self.sim_dt      = 1./50. # Delta tick
      self.sim_on      = sim_on
      self.sim_total   = sim_total
      self.filename    = filename

      # Load some data
      self.ship_mass, self.ship_weaps = self.loadship( shipname )
      self.weap_mass, self.weap_delay, self.weap_energy = self.loadweap( weapname )

   def saveto( self, filename ):
      self.filename    = filename

   def loadship( self, shipname ):
      "Returns mass, number of weaps."
      if shipname == "llama":
         return 80., 2
      elif shipname == "lancelot":
         return 180., 4
      elif shipname == "pacifier":
         return 730., 5 
      elif shipname == "hawking":
         return 3750., 7
      elif shipname == "peacemaker":
         return 6200., 8
      else:
         raise ValueError

   def loadweap( self, weapname ):
      "Returns mass, delay, energy."
      if weapname == "laser":
         return 2., 0.9, 4.25
      elif weapname == "plasma":
         return 4., 0.675, 3.75
      elif weapname == "ion":
         return 6., 1.440, 15.
      elif weapname == "laser turret":
         return 16., 0.540, 6.12
      elif weapname == "ion turret":
         return 42., 0.765, 25.
      else:
         raise ValueError

   def prepare( self ):
      # Calculate ship parameters
      ship_kg          = self.ship_mass * 1000.
      self.ship_emis   = 0.8
      self.ship_cond   = self.STEEL_COND
      self.ship_C      = self.STEEL_CAP * ship_kg
      self.ship_area   = pow( ship_kg / self.STEEL_DENS, 2./3. )
      self.ship_T      = self.SPACE_TEMP

      # Calculate weapon parameters
      weap_kg          = self.weap_mass * 1000.
      self.weap_C      = self.STEEL_CAP * weap_kg
      self.weap_area   = pow( weap_kg / self.STEEL_DENS, 2./3. )
      self.weap_list   = []
      self.weap_T      = []
      for i in range(self.ship_weaps):
         self.weap_list.append( i*self.weap_delay / self.ship_weaps )
         self.weap_T.append( self.SPACE_TEMP )

   def simulate( self ):
      "Begins the simulation."

      # Prepare it
      self.prepare()

      # Write to file if necessary
      if self.filename != None:
         f = open( self.filename, 'w' )

      # Run simulation
      dt          = self.sim_dt
      sim_elapsed = 0.
      while sim_elapsed < self.sim_total:

         Q_cond = 0.

         if self.filename != None:
            f.write( str(sim_elapsed) + ' ' )

         # Check weapons
         for i in range(len(self.weap_list)):
            self.weap_list[i] -= dt

            # Check if shot
            if sim_elapsed < self.sim_on and self.weap_list[i] < 0.:
               self.weap_T[i]     += 1000. * self.weap_energy / self.weap_C
               self.weap_list[i]  += self.weap_delay

            # Do heat movement (conduction)
            Q           = -self.ship_cond * (self.weap_T[i] - self.ship_T) * self.weap_area * dt
            self.weap_T[i]  += Q / self.weap_C
            Q_cond     += Q

            if self.filename != None:
               f.write( str(self.weap_T[i]) + ' ' )

         # Do ship heat (radiation)
         Q_rad    = self.STEFAN_BOLZMANN * self.ship_area * self.ship_emis * (pow(self.SPACE_TEMP,4.) - pow(self.ship_T,4.)) * dt
         Q        = Q_rad - Q_cond
         self.ship_T  += Q / self.ship_C

         if self.filename != None:
            f.write( str(self.ship_T) + '\n' )

         # Elapsed time
         sim_elapsed += dt;

      # Close file
      if self.filename != None:
         f.close()

   def display( self ):
      print("Ship Temp: "+str(hs.ship_T)+" K")
      for i in range(len(hs.weap_list)):
         print("Outfit["+str(i)+"] Temp: "+str(hs.weap_T[i])+" K")


if __name__ == "__main__":
   print("NAEV HeatSim\n")
   hs = heatsim( "llama", "laser", 60., 120., "llama.dat" )
   hs.simulate()
   hs.display()



