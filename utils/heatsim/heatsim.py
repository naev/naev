#!/usr/bin/env python


#######################################################
#
#        SIM CODE
#
#######################################################

# Imports
import math
import matplotlib.pyplot as plt


def clamp( a, b, x ):
   return min( b, max( a, x ) )

def frange(start, end=None, inc=None):
    if end is None:
        start, end = 0., start + 0.
    if inc is None:
        inc = 1.
    return [i*inc+start for i in range(1+int((end-start)//inc))]


class heatsim:
   def __init__( self, shipname = "llama", weapname = "laser", simulation = [ 60., 120. ] ):
      # Sim parameters
      self.STEFAN_BOLZMANN = 5.67e-8
      self.SPACE_TEMP  = 250.
      self.STEEL_COND  = 54.
      self.STEEL_CAP   = 0.49
      self.STEEL_DENS  = 7.88e3
      self.ACCURACY_LIMIT = 500
      self.FIRERATE_LIMIT = 800
      self.shipname    = shipname
      self.weapname    = weapname
      # Sim info
      self.sim_dt      = 1./50. # Delta tick
      self.setSimulation( simulation )

      # Load some data
      self.ship_mass, self.ship_weaps = self.loadship( shipname )
      self.weap_mass, self.weap_delay, self.weap_energy = self.loadweap( weapname )

   def setSimulation( self, simulation ):
      self.simulation  = simulation
      self.sim_total   = simulation[-1]

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
      elif weapname == "railgun turret":
         return 60., 1.102, 66.
      else:
         raise ValueError

   def prepare( self ):
      # Time stuff
      self.time_data   = []

      # Calculate ship parameters
      ship_kg          = self.ship_mass * 1000.
      self.ship_emis   = 0.8
      self.ship_cond   = self.STEEL_COND
      self.ship_C      = self.STEEL_CAP * ship_kg
      #self.ship_area   = pow( ship_kg / self.STEEL_DENS, 2./3. )
      self.ship_area   = 4.*math.pi*pow( 3./4.*ship_kg/self.STEEL_DENS/math.pi, 2./3. )
      self.ship_T      = self.SPACE_TEMP
      self.ship_data   = []

      # Calculate weapon parameters
      weap_kg          = self.weap_mass * 1000.
      self.weap_C      = self.STEEL_CAP * weap_kg
      #self.weap_area   = pow( weap_kg / self.STEEL_DENS, 2./3. )
      self.weap_area   = 2.*math.pi*pow( 3./4.*weap_kg/self.STEEL_DENS/math.pi, 2./3. )
      self.weap_list   = []
      self.weap_T      = []
      self.weap_data   = []
      for i in range(self.ship_weaps):
         self.weap_list.append( i*self.weap_delay / self.ship_weaps )
         self.weap_T.append( self.SPACE_TEMP )
         self.weap_data.append( [] )

   def __accMod( self, T ):
      return clamp( 0., 1., (T-500.)/600. )

   def __frMod( self, T ):
      return clamp( 0., 1., (1100.-T)/300. )

   def simulate( self ):
      "Begins the simulation."
      # Prepare it
      self.prepare()

      # Run simulation
      weap_on     = True
      sim_index   = 0
      dt          = self.sim_dt
      sim_elapsed = 0.
      while sim_elapsed < self.sim_total:
         Q_cond = 0.

         # Check weapons
         for i in range(len(self.weap_list)):
            # Check if we should start/stop shooting
            if self.simulation[ sim_index ] < sim_elapsed:
               weap_on     = not weap_on
               sim_index  += 1

            # Check if shot
            if weap_on:
               self.weap_list[i] -= dt * self.__frMod( self.weap_T[i] )
               if self.weap_list[i] < 0.:
                  self.weap_T[i]     += 1e4 * self.weap_energy / self.weap_C
                  self.weap_list[i]  += self.weap_delay

            # Do heat movement (conduction)
            Q           = -self.ship_cond * (self.weap_T[i] - self.ship_T) * self.weap_area * dt
            self.weap_T[i]  += Q / self.weap_C
            Q_cond     += Q
            self.weap_data[i].append( self.weap_T[i] )

         # Do ship heat (radiation)
         Q_rad    = self.STEFAN_BOLZMANN * self.ship_area * self.ship_emis * (pow(self.SPACE_TEMP,4.) - pow(self.ship_T,4.)) * dt
         Q        = Q_rad - Q_cond
         self.ship_T  += Q / self.ship_C
         self.time_data.append( sim_elapsed )
         self.ship_data.append( self.ship_T )

         # Elapsed time
         sim_elapsed += dt;

   def save( self, filename ):
      "Saves the results to a file."
      with open( self.filename, 'w' ) as f:
         for time, ship, weap in zip(self.time_data, self.ship_data, self.weap_data):
            f.write( f'{time} {ship_data}')
            for wj in weap:
               f.write( f' {wj}' )
            f.write( '\n' )

   def display( self ):
      print("Ship Temp:", hs.ship_T, "K")
      for i, T in enumerate(hs.weap_T):
         print(f"Outfit[{i}] Temp: {T} K")


   def plot( self, filename=None ):
      plt.figure()

      # Plot 1 Data
      plt.subplot(211)
      plt.plot( self.time_data, self.ship_data, '-' )
      
      # Plot 1 Info
      plt.axis( [0, self.sim_total, 0, 1100] )
      plt.title( 'NAEV Heat Simulation ('+self.shipname+' with '+self.weapname+')' )
      plt.legend( ('Ship', 'Accuracy Limit', 'Fire Rate Limit'), loc='upper left')
      plt.ylabel( 'Temperature [K]' )
      plt.grid( True )

      # Plot 1 Data
      plt.subplot(212)
      plt.plot( self.time_data, self.weap_data[0], '-' )
      plt_data = [self.ACCURACY_LIMIT] * len(self.weap_data[0])
      plt.plot( self.time_data, plt_data, '--' )
      plt_data = [self.FIRERATE_LIMIT] * len(self.weap_data[0])
      plt.plot( self.time_data, plt_data, '-.' )

      # Plot 2 Info
      plt.axis( [0, self.sim_total, 0, 1100] )
      plt.legend( ('Weapon', 'Accuracy Limit', 'Fire Rate Limit'), loc='upper right')
      plt.ylabel( 'Temperature [K]' )
      plt.xlabel( 'Time [s]' )
      plt.grid( True )
      if filename is None:
         plt.show()
      else:
         plt.savefig( filename )


if __name__ == "__main__":
   print("NAEV HeatSim\n")
   shp_lst = { 'llama' : 'laser',
               'lancelot' : 'ion',
               'pacifier' : 'laser turret',
               'hawking' : 'ion turret',
               'peacemaker' : 'railgun turret' }
   
   for shp,wpn in shp_lst.items():
      hs = heatsim( shp, wpn, (60., 120.) )
      #hs = heatsim( shp, wpn, frange( 30., 600., 30. ) )
      hs.simulate()
      hs.plot( f'{shp}_{wpn}_60_60.png' )
      hs.setSimulation( (30., 90.) )
      hs.simulate()
      hs.plot( f'{shp}_{wpn}_30_60.png' )
      hs.setSimulation( (30., 90., 120., 180.) )
      hs.simulate()
      hs.plot( f'{shp}_{wpn}_30_60_30_60.png' )
      print( '  ', shp, 'with', wpn, 'done!' )
