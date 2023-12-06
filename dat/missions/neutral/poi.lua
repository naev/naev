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
local fmt = require "format"

function create ()
   mem.sys, mem.risk, mem.reward = poi.start()
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

   -- Choose the reward, which basically means the type of POI
   -- It can either be forced from the outside, or chosen randomly from a generated list
   -- Generated here to disallow save scumming
   if mem.reward then
      -- Specified during creation
      local reward = require( mem.reward )( mem )
      if not reward then
         error(fmt.f(_("Something went wrong when starting to force start POI '{poiname}'!"),{poiname=mem.reward}))
      end
      reward.requirename = mem.reward
      mem.reward = reward
   else
      -- Create a random reward
      local reward_list = {
         {
            type = "credits",
            value = (100e3 + 100e3*rnd.rnd()) * (mem.rewardrisk*0.5+1),
            weight = 0.5, -- less likely
         },
      }
      if poi.data_get_gained() > 0 then
         table.insert( reward_list, {
            type="data",
         } )
      end

      -- Parse directory to add potential rewards
      for k,v in ipairs(lf.getDirectoryItems("missions/neutral/poi")) do
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
            mem.reward = v
            break
         end
      end
   end
   if not mem.reward then
      warn("poi: started up without reward!")
      misn.finish(false)
   end

   poi.misnSetup{ sys=mem.sys, found="found", risk=mem.risk }
end

-- luacheck: globals found (passed to POI framework for hooks)
function found ()
   player.msg(_("You have found something!"),true)

   -- TODO something more interesting
   local shiptype = mem.reward.ship or "Mule"
   local shipname = mem.reward.shipname or _("Unusual Derelict")
   local p = pilot.add( shiptype, "Derelict", mem.goal, shipname, {naked=true} )
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
