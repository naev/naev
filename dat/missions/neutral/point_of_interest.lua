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
   - Use minigames? stringguess is a good candidate
   - Give a couple interesting outfit rewards, some that are repeatable
   - Add lots of lore rewards, not sure if best to store in ship log or somewhere else

--]]
local fmt = require "format"
--local luaspfx = require "luaspfx"
--local tut = require "common.tutorial"
local der = require 'common.derelict'
local poi = require "common.poi"
local vn = require "vn"

-- luacheck: globals found board (Hook functions passed by name)

function create ()
   mem.sys, mem.risk, mem.reward = poi.start()

   -- We do a soft claim on the final system
   if not misn.claim( {mem.sys}, nil, true ) then
      return
   end

   -- Roll for rewards here to disallow save scumming
   local reward_list = {
      {
         type = "credits",
         value = 200e3 + 200e3*rnd.rnd(),
      },
   }
   local function add_unique_reward( oname )
      if player.numOutfit( oname ) <= 0 then
         table.insert( reward_list, {
            type = "outfit",
            value = oname,
         } )
      end
   end
   add_unique_reward( "Laser Cannon MK1" )

   -- Choose a random reward and stick to it
   mem.reward = reward_list[ rnd.rnd(1,#reward_list) ]

   poi.misnSetup{ sys=mem.sys, found="found" }
end

function found ()
   player.msg(_("You have found something!"),true)

   -- TODO something more interesting
   local p = pilot.add( "Mule", "Derelict", mem.goal )
   p:rename(_("Pristine Derelict"))
   p:disable()
   p:setInvincible()
   p:setHilight()
   p:effectAdd( "Fade-In" )
   hook.pilot( p, "board", "board" )
end

function board( p )
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()
   vn.na(_("You board the derelict which seems oddly in pretty good condition. What a lucky find!"))
   if mem.reward.type == "credits" then
      vn.na(_("You access the main computer and are able to login to find a hefty amount of credits. This will come in handy."))
   elseif mem.reward.type == "outfit" then
      vn.na(_("Exploring the cargo bay, you find something that might be of use to you."))
   end
   vn.sfxVictory()
   vn.na(fmt.reward(mem.reward.value))
   vn.na(_("You explore the rest of the ship but do not find anything else of interest."))
   vn.sfx( der.sfx.unboard )
   vn.run()

   -- Actually give reward
   if mem.reward.type == "credits" then
      player.pay( mem.reward.value )
   elseif mem.reward.type == "outfit" then
      player.outfitAdd( mem.reward.value )
   end

   -- Clean up stuff
   poi.misnDone()
   p:setHilight(false)
   player.unboard()
   misn.finish(true)
end
