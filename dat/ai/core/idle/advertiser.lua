require 'ai.core.idle.civilian'

mem.adspamdelayalpha = 15
mem.adspamdelaybeta = 30

control_funcs.loiter = function ()
   local lastspammed = mem.adspamlast or 0
   local curtime = naev.ticks()
   local delay = mem.adspamdelay or (mem.adspamdelayalpha + mem.adspamdelaybeta*rnd.rnd())

   if curtime - lastspammed > delay then
      ai.pilot():broadcast(mem.ad)
      mem.adspamlast = curtime
      mem.adspamdelay = mem.adspamdelayalpha + mem.adspamdelaybeta*rnd.rnd()
   end
end
