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
--local luaspfx = require "luaspfx"
--local tut = require "common.tutorial"
local poi = require "common.poi"
local lf = require "love.filesystem"


function create ()
   mem.sys, mem.risk = poi.start()
   mem.rewardrisk = mem.risk

   -- We do a soft claim on the final system
   if not misn.claim( {mem.sys}, true ) then
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
      table.insert( reward_list, {
         type="data",
      } )
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

   -- Compute total weight
   local wtotal = 0
   for k,v in ipairs(reward_list) do
      local w = v.weight or 1
      v.weight = w
      wtotal = wtotal + w
   end
   table.sort( reward_list, function( a, b )
      return a.weight > b.weight
   end )

   -- Choose a random reward and stick to it
   local r = wtotal*rnd.rnd()
   local waccum = 0
   for k,v in ipairs(reward_list) do
      waccum = waccum + v.weight
      if r <= waccum then
         mem.reward = r
         break
      end
   end

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
   local failed = poi.board()

   -- Clean up stuff
   poi.cleanup( failed )
   p:setHilight(false)
   player.unboard()
   misn.finish( not failed )
end
