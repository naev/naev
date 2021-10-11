require 'ai.core.idle.civilian'

control_funcs.loiter = function ()
   local lastspammed = mem.adspamlast or 0
   local curtime = naev.ticks()
   local delay = mem.adspamdelay or 15

   if curtime - lastspammed > delay then
      ai.pilot():broadcast(mem.ad)
      mem.adspamlast = curtime
      mem.adspamdelay = 15 + 30*rnd.rnd()
   end
end
