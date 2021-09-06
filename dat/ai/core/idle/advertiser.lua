require 'ai.core.idle.civilian'

control_funcs.loiter = function ()
   local lastspammed = mem.adspamlast or 0
   local curtime = naev.ticks() / 1e3
   local delay = mem.adspamdelay or 15

   print( string.format( "curtime-lastspammed = %.1f, delay = %.1f", curtime-lastspammed, delay) )
   if curtime - lastspammed > delay then
      ai.pilot():broadcast(mem.ad)
      mem.adspamlast = curtime
      mem.adspamdelay = 15 + 30*rnd.rnd()
   end
end
