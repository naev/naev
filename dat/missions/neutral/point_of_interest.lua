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
local luaspfx = require "luaspfx"
--local tut = require "common.tutorial"
local der = require 'common.derelict'
local poi = require "common.poi"
local vn = require "vn"

-- luacheck: globals enter scan heartbeat_nooutfit heartbeat board (Hook functions passed by name)

function create ()
   mem.sys, mem.risk, mem.reward = poi.start()

   local riskstr = "Low"
   local rewardstr = "Unknown"

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
   add_unique_reward( "Ion Cannon MK1" )

   -- Choose a random reward and stick to it
   mem.reward = reward_list[ rnd.rnd(1,#reward_list) ]

   -- Accept and set up mission
   misn.accept()

   misn.setTitle(_("Point of Interest")) -- TODO maybe randomize somewhat?
   misn.setReward(_("Unknown")) -- TODO give some hint?
   misn.setDesc(fmt.f(_([[A point of interest has been found in the {sys} system. It is not clear what can be foound, however, it warrants investigation. You should bring an outfit that can perform scans such as a #bPulse Scanner#0.

#nEstimated Risk:#0 {risk}
#nEstimated Reward:#0 {reward}]])),
      {sys=mem.sys, risk=riskstr, reward=rewardstr} )

   misn.markerAdd( mem.sys, "low" )

   hook.enter( "enter" )
   if system.cur() == mem.sys then
      enter()
   end
end

local pos, timer, path, goal
function enter ()
   if system.cur() ~= mem.sys then
      pos = nil
      goal = nil
      if mem.chook then
         hook.rm( mem.chook )
         mem.chook = nil
      end
      if timer then
         hook.rm( timer )
         timer = nil
      end
      return
   end

   -- Find the initial point of interest, and path to the real location
   pos = vec2.newP( system.cur():radius()*0.6, rnd.angle() ) -- TODO better initialization
   path = {}
   local angle = rnd.angle()
   local mpos = pos
   for i=1,rnd.rnd(13,17) do -- should average 15*375 = 5625 units
      mpos = mpos + vec2.newP( 250+250*rnd.rnd(), angle )
      angle = angle + rnd.sigma()
      table.insert( path, mpos )
   end
   goal = mpos + vec2.newP( 250+250*rnd.rnd(), angle )

   system.mrkAdd( pos, _("Point of Interest") )

   -- Custom hook for when the player scans
   mem.chook = hook.custom( "poi_scan", "scan" )

   -- If the player has no scanning outfit we have to help them out
   local pp = player.pilot()
   local haspoi = false
   for k,v in ipairs(pp:outfits()) do
      if v:tags().poi_scan then
         haspoi = true
         break
      end
   end
   if not haspoi then
      timer = hook.timer( 5, "heartbeat_nooutfit" )
   end
end

function heartbeat_nooutfit ()
   if player.pos():dist( pos ) < 3e3 then
      -- TODO ship AI message
      player.msg(_("You lack an outtfit no scan the point of interest."),true)
      return
   end
   timer = hook.timer( 1, "heartbeat_nooutfit" )
end

local started, path_spfx
function scan ()
   -- Ignore if not close
   if started or player.pos():dist( pos ) > 3e3 then
      return
   end

   -- Starts marks
   started = true
   path_spfx = {}
   for k,v in ipairs(path) do
      table.insert( path_spfx, luaspfx.trail( v ) )
   end

   timer = hook.timer( 1, "heartbeat" )
end

function heartbeat ()
   for k,v in ipairs(path) do
      -- Have to follow the path
      if not path_spfx[k] and (k<=1 or path_spfx[k-1]) and player.pos():dist( v ) < 1e3 then
         path_spfx[k] = luaspfx.trail( v )
      end
   end

   -- Found goal
   if path_spfx[ #path ] and player.pos():dist( goal ) < 1e3 then
      player.msg(_("You have found something!"),true)

      -- TODO something more interesting
      local p = pilot.add( "Mule", "Derelict" )
      p:rename(_("Pristine Derelict"))
      p:disable()
      p:setInvincible()
      hook.pilot( p, "board", "board" )
      return
   end

   timer = hook.timer( 1, "heartbeat" )
end

function board ()
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
   vn.na(fmt.reward_(mem.reward.value))
   vn.na(_("You explore the rest of the ship but do not find anything else of interest."))
   vn.sfx( der.sfx.unboard )
   vn.run()

   -- Actually give reward
   if mem.reward.type == "credits" then
      player.pay( mem.reward.value )
   elseif mem.reward.type == "outfit" then
      player.outfitAdd( mem.reward.value )
   end

   player.unboard()
end
