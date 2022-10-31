--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Point of Interest">
 <location>none</location>
 <chance>0</chance>
</mission>
--]]
--[[

   Point of Interest missions. These are obtained through various means and make the player do a bit of exploration in order to obtain fancy rewards.

   General approach is:
   1. Go to Point of Interest
   2. Activate scanning outfit
   3. Follow trails (with potential hostiles) to goal
   4. Loot goal!

   TODO
   - Give expensive lock override (500k?) if the player fails too many times (5 > ?) from NPC
   - Give a couple interesting outfit rewards, some that are repeatable
   - Add lots of lore rewards, not sure if best to store in ship log or somewhere else

--]]
local fmt = require "format"
--local luaspfx = require "luaspfx"
--local tut = require "common.tutorial"
local der = require 'common.derelict'
local poi = require "common.poi"
local vn = require "vn"
local lf = require "love.filesystem"


function create ()
   mem.sys, mem.risk = poi.start()
   mem.rewardrisk = mem.risk

   -- We do a soft claim on the final system
   if not misn.claim( {mem.sys}, nil, true ) then
      return
   end

   --[[
   -- Chance of being locked with less risk
   if rnd.rnd() < 0.2*mem.risk then
      mem.locked = true
      mem.risk = mem.risk-1
   end
   --]]
   mem.risk = 2
   mem.locked = true

   -- Roll for rewards here to disallow save scumming
   local reward_list = {
      {
         type = "credits",
         value = (100e3 + 100e3*rnd.rnd()) * (mem.rewardrisk*0.5+1),
      },
   }
   if poi.data_get_gained() > 0 then
      table.insert( reward_list, { type="data" } )
   end

   -- Parse directory to add potential rewards
   for k,v in ipairs(lf.enumerate("missions/neutral/poi")) do
      local requirename = "missions.neutral.poi."..string.gsub(v,".lua","")
      local reward = require( requirename )( mem )
      if reward then
         reward.requirename = requirename
         table.insert( reward_list, reward )
      end
   end

   -- Choose a random reward and stick to it
   mem.reward = reward_list[ rnd.rnd(1,#reward_list) ]

   poi.misnSetup{ sys=mem.sys, found="found", risk=mem.risk }
end

-- luacheck: globals found (passed to POI framework for hooks)
function found ()
   player.msg(_("You have found something!"),true)

   -- TODO something more interesting
   local shiptype = mem.reward.ship or "Mule"
   local p = pilot.add( shiptype, "Derelict", mem.goal, _("Pristine Derelict"), {naked=true} )
   p:disable()
   p:setInvincible()
   p:setHilight()
   p:effectAdd( "Fade-In" )
   hook.pilot( p, "board", "board" )
end

function board( p )
   local failed = false

   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()

   -- Have to resolve lock or bad thing happens (tm)
   if mem.locked then
      local stringguess = require "minigames.stringguess"
      vn.na(_([[You board the ship and enter the airlock. When you attempt to enter, an authorization prompt opens up. Looking at the make of the ship, it seems heavily reinforced. It looks like you're going to have to break the code to gain complete access to the ship.]]))
      stringguess.vn()
      vn.func( function ()
         if stringguess.completed() then
            vn.jump("unlocked")
            return
         end
         vn.jump("unlock_failed")
      end )

      vn.label("unlocked")
      vn.na(_([[You deftly crack the code and the screen flashes with '#gAUTHORIZATION GRANTED#0'. Time to see what goodness awaits you!]]))
      vn.jump("reward")

      vn.label("unlock_failed")
      vn.na(_([[A brief '#rAUTHORIZATION DENIED#0' flashes on the screen and you hear the ship internals groan as the emergency security protocol kicks in and everything gets locked down. It looks like you won't be getting anywhere here; the ship is as good as debris. You have no option but to return dejectedly to your ship. Maybe next time.]]))
      vn.func( function () failed = true end )
      vn.done()
   else
      vn.na(_([[You board the derelict which seems oddly in pretty good condition. Furthermore, it seems like there is no access lock in place. What a lucky find!]]))
   end

   vn.label("reward")
   if mem.reward.type == "function" then
      local rwd = require( mem.reward.requirename )( mem )
      if rwd then
         rwd.func()
      else -- Failed to get a reward, just default to data
         mem.reward.type = "data"
      end
   end
   if mem.reward.type == "credits" then
      local msg = _([[You access the main computer and are able to login to find a hefty amount of credits. This will come in handy.]])
      msg = msg .. "\n\n" .. fmt.reward(mem.reward.value)
      vn.na( msg )
      vn.func( function ()
         player.pay( mem.reward.value )
         poi.log(fmt.f(_([[You found a pristine derelict with large amounts of credits in the {sys} system..]]),
            {sys=mem.sys}))
      end )
   elseif mem.reward.type == "data" then
      local msg = _([[You access the main computer and are able to extract some Encrypted Data Matrices. It does not seem like you can de-encrypt them without damaging them, but they may have some other use.]])
      msg = msg .. "\n\n" .. fmt.reward(_("Encrypted Data Matrix"))
      vn.na( msg )
      vn.func( function ()
         poi.data_give( 1 )
         poi.log(fmt.f(_([[You found a pristine derelict with an Encrypted Data Matrix in the {sys} system..]]),
            {sys=mem.sys}))
      end )
   elseif mem.reward.type == "outfit" then
      local msg = mem.reward.msg or _([[Exploring the cargo bay, you find something that might be of use to you.]])
      msg = msg .. "\n\n" .. fmt.reward(mem.reward.value)
      vn.na( msg )
      vn.func( function ()
         player.outfitAdd( mem.reward.value )
      end )
   end
   vn.sfxVictory()
   vn.na(_([[You explore the rest of the ship but do not find anything else of interest. Although the ship is in very good condition, it is still not space-worthy, and there is not anything that you can do with it. You let it rest among the stars.]]))
   vn.sfx( der.sfx.unboard )
   vn.run()

   -- Clean up stuff
   poi.misnDone( failed )
   p:setHilight(false)
   player.unboard()
   misn.finish( not failed )
end
