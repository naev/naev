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
      local p = ai.pilot()
      local ad = mem.ad
      local tad = type(ad)
      if tad == "table" then -- If table we just cycle through them from random start
         mem.adid = mem.adid or rnd.rnd(1,#ad)
         mem.adid = math.fmod( mem.adid, #ad )+1
         ad = ad[ mem.adid ]
      elseif tad == "function" then
         ad = ad( p )
      end

      -- Broadcast for all to hear!
      if ad then
         p:broadcast( ad )
      end

      -- Delay again
      mem.adspamlast = curtime
      mem.adspamdelay = mem.adspamdelayalpha + mem.adspamdelaybeta*rnd.rnd()
   end
end
