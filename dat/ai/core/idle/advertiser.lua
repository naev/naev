require 'ai.core.idle.civilian'

mem.adid = nil
mem.adspamdelayalpha = 15
mem.adspamdelaybeta = 30

control_funcs.loiter = function ()
   if not mem.ad then
      return
   end

   local lastspammed = mem.adspamlast or 0
   local curtime = naev.ticks()
   local delay = mem.adspamdelay or (mem.adspamdelayalpha + mem.adspamdelaybeta*rnd.rnd())

   if curtime - lastspammed > delay then
      local ad = mem.ad
      if type(ad) == "table" then -- If table we just cycle through them from random start
         mem.adid = mem.adid or rnd.rnd(1,#ad)
         mem.adid = math.fmod( mem.adid, #ad )+1
         ad = ad[ mem.adid ]
      end

      -- Broadcast for all to hear!
      ai.pilot():broadcast( ad )

      -- Delay again
      mem.adspamlast = curtime
      mem.adspamdelay = mem.adspamdelayalpha + mem.adspamdelaybeta*rnd.rnd()
   end
end
