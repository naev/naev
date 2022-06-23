--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Point of Interest - Intro">
 <location>none</location>
 <chance>0</chance>
</mission>
--]]
--[[

   Introduction to Point of Interest missions.

   We assume it is started from a derelict event.

   1. Follow NPC to point of interest.
   2. NPC activates scanning outfit and is attacked by a bandit hyena.
   3. Destroy hyena and follow trails.
   4. Loot goal and get Pulse Scanner!

--]]
local fmt = require "format"
--local luaspfx = require "luaspfx"
--local tut = require "common.tutorial"
local der = require 'common.derelict'
local poi = require "common.poi"
local tut = require "common.tut"
local tutnel= require "common.tut_nelly"
local pir = require "common.pirate"
local vn = require "vn"
local lmisn = require "lmisn"

-- luacheck: globals land approach_nelly enter enter_delay found board (Hook functions passed by name)

--[[
States
   0: Mission start, POI is marked
   1: Land and Nelly appears
--]]

function create ()
   local syscand = lmisn.getSysAtDistance( nil, 1, 3, function( sys )
      -- Must be claimable
      if not naev.claimTest( {sys} ) then -- Full claim here
         return
      end

      -- Want no inhabited spobs
      for k,p in ipairs(sys:spobs()) do
         local s = p:services()
         if s.land and s.inhabitable then
            return false
         end
      end
      return true
   end )

   -- Failed to start
   if #syscand <= 0 then misn.finish(false) end

   mem.sys = syscand[ rnd.rnd(1,#syscand) ]
   mem.risk = 0
   mem.rewardrisk = mem.risk

   -- We do a full claim on the final system
   if not misn.claim( {mem.sys, "nelly"} ) then
      return
   end

   local accept = false
   vn.clear()
   vn.scene()
   vn.sfx( der.sfx.board )
   vn.music( der.sfx.ambient )
   vn.transition()

   vn.na(_([[You carefully explore the heavily damaged derelict. It looks like most of it has been picked clean. You are about to give up when the system computer suddenly flickers and seems to boot up.]]))

   local sai = tut.vn_shipai()
   vn.appear( sai, tut.shipai.transition )

   vn.na(fmt.f(_([[As the ship's operating system is starting, {shipai} materializes in front of you.]]),{shipai=tut.ainame()}))
   if not var.peek( "poi_sai_intro" ) then
      -- Worded to be a bit like the explosion messages in EVC
      sai(_([["While you were exploring I managed to bootstrap the ship's systems. No, nothing bad could have happened. I estimated under 10% chance of triggering the ship's security self-destruct mechasim and blowing up the ship. Oh…"]]))
      sai(_([["Anyway, it seems like my work paid off, there seems to be some data marking a point of interest. Should I download the data so that we can explore it?"]]))
      vn.func( function ()
         var.push( "poi_sai_intro", true )
      end )
   else
      sai(_([["Hey, it looks like there is data marking a point of interest stored on the ship computer. Should I download the data so that we can explore it?"]]))
   end

   vn.menu{
      {_("Download the data"), "accept"},
      {_("Leave."), "leave"},
   }

   vn.label("accept")
   vn.func( function () accept = true end )
   if mem.sys:known() then
      sai(fmt.f(_([[{shipai} flickers slightly as they download the data.
"It looks like the data points to a location in the nearby system of {sys}. I have marked the location on your map. However, I'm not certain the ship's current systems would allow it to follow such a faint hint. Maybe it would be wise to land and see if we can improve our systems somehow."]]),{shipai=tut.ainame(),sys=mem.sys}))
   else
      sai(fmt.f(_([[{shipai} flickers slightly as they download the data.
"It looks like the data points to a location in a nearby system. I have marked the location on your map. However, I'm not certain the ship's current systems would allow it to follow such a faint hint. Maybe it would be wise to land and see if we can improve our systems somehow."]]),{shipai=tut.ainame()}))
   end
   vn.na(_([[With nothing else to do on the derelict, you leave it behind, and return to your ship.]]))
   vn.jump("done")

   vn.label("leave")
   vn.na(_([[You decide to leave the information alone and leave the derelict.]]))

   vn.label("done")
   vn.sfx( der.sfx.unboard )
   vn.run()
   player.unboard()

   if accept then
      der.addMiscLog(fmt.f(_([[You found information on a point of interest aboard a derelict in the {sys} system.]]),{sys=system.cur()}))
   else
      der.addMiscLog(_([[You found information about a point of interest aboard a derelict, but decided not to download it.]]))
      misn.finish(false)
      return
   end

   misn.osdCreate( _("Point of Interest"), {
      _("Improve your ship's systems"),
      _("Head to the location marked on your map"),
   } )

   mem.state = 0
   mem.locked = true

   poi.misnSetup{ sys=mem.sys, found="found", risk=mem.risk }

   hook.enter( "enter" )
   hook.land( "land" )
end

function land ()
   local spb = spob.cur()
   local f = spb:faction()
   if spb:tags().restricted or not f or pir.factionIsPirate(f) then
      return
   end

   local desc
   if var.peek("nelly_met") then
      desc = _("Nelly is motioning you to come join her at the table.")
   else
      desc = _("")
   end
   mem.npc_nel = misn.npcAdd( "approach_nelly", tutnel.nelly.name, tutnel.nelly.portrait, desc )
end

function approach_nelly ()
end

local nelly
function enter ()
   if system.cur() ~= mem.sys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn( false )

   local shipname
   if player.misnDone( "Helping Nelly Out 2" ) then
      shipname = _("Llaminator MK3")
   else
      shipname = _("Llaminator MK2")
   end

   -- Spawn ship
   local jumpin = vec2.new()
   nelly = pilot.add( "Llama", "Independent", shipname, jumpin )
   nelly:setInvincPlayer(true)

   hook.timer( 1, "enter_delay" )
end

local pos
function enter_delay ()
   pos = poi.misnPos()

   nelly:control()
   nelly:moveto( pos )
end

function found ()
   player.msg(_("You have found something!"),true)

   -- TODO something more interesting
   local p = pilot.add( "Mule", "Derelict", mem.goal, _("Pristine Derelict"), {naked=true} )
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
         if stringguess.completed then
            vn.jump("unlocked")
            return
         end
         vn.jump("unlock_failed")
      end )

      vn.label("unlocked")
      vn.na(_([[You deftly crack the code and the screen flashes with '#gAUTHORIZATION GRANTED#0'. Time to see what goodness awaits you!]]))
      vn.jump("reward")

      vn.label("unlock_failed")
      vn.na(_([["A brief '#rAUTHORIZATION DENIED#0' flashes on the screen and you hear the ship internals groan as the emergency security protocol kicks in and everything gets locked down. It looks like you won't be getting anywhere hre, the ship is as good as debris. You have no option but to return dejectedly to your ship. Maybe next time."]]))
      vn.func( function () failed = true end )
      vn.done()
   else
      vn.na(_([[You board the derelict which seems oddly in pretty good condition. Furthermore, it seems like there is no access lock in place. What a lucky find!]]))
   end

   vn.label("reward")
   if mem.reward.type == "credits" then
      local msg = _([[You access the main computer and are able to login to find a hefty amount of credits. This will come in handy.]])
      msg = msg .. "\n\n" .. fmt.reward(mem.reward.value)
      vn.na( msg )
      vn.func( function ()
         player.pay( mem.reward.value )
      end )
   elseif mem.reward.type == "outfit" then
      local msg = mem.reward.msg or _([[Exploring the cargo bay, you find something that might be of use to you.]])
      msg = msg .. "\n\n" .. fmt.reward(mem.reward.value)
      vn.na( msg )
      vn.func( function ()
         player.outfitAdd( mem.reward.value )
      end )
   elseif mem.reward.type == "function" then
      local rwd = require( mem.reward.requirename )( mem )
      rwd.func()
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
