--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Levo Pirates">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Save Levo!

   Creates a small blockade of pirates in the Levo system. The player has to
   clear them to be able to land.

   This is triggered on "load" to be able to claim the system so that other
   missions don't mess with it. Furthermore, the spob is set as restricted to
   avoid having missions go to it, but it gets cleared when the event is
   finished.
--]]
local vn = require 'vn'
local fmt = require 'format'
local ccomm = require "common.comm"
local lmisn = require "lmisn"

local mainspb, mainsys = spob.getS("Levo")
local piratename = _("Red Goatee")

function create ()
   if not evt.claim{ mainsys } then
      warn(fmt.f(_("Unable to claim {sys} system!"),{sys=mainsys}))
      return
   end

   hook.enter( "enter" )
end

local pirboss
local baddies = {}
function enter ()
   if system.cur()~=mainsys then
      return
   end

   -- Get rid of spawns
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Hail hook
   hook.hail_spob( "comm_levo" )
   player.landAllow( false, _("You can not land until you clear the pirates from the system!") )

   -- Spawn pirates
   local function spawn_pirate( shipname, pos, boss )
      local p = pilot.add( shipname, "Marauder", pos + vec2.newP( 100*rnd.rnd(), rnd.angle() ), nil, {ai="guard"} )
      p:setHostile(true)
      local m = p:memory()
      m.bribe_no = _([["You ain't payin' yer wait outta this one!"]])
      m.refuel_no = _([["Do I look like a fuel station?"]])
      if boss then
         p:setLeader( boss )
      end
      hook.pilot( p, "exploded", "pir_gone" )
      hook.pilot( p, "jump",     "pir_gone" )
      hook.pilot( p, "land",     "pir_gone" )
      table.insert( baddies, p )
      return p
   end

   local mainpos = mainspb:pos() + vec2.newP( 200, rnd.angle() )
   pirboss = spawn_pirate( "Pirate Starbridge", mainpos )
   pirboss:rename(piratename)
   pirboss:setVisplayer(true)
   spawn_pirate( "Pirate Shark", mainpos, pirboss )
   spawn_pirate( "Pirate Vendetta", mainpos, pirboss )
   spawn_pirate( "Pirate Hyena", mainpos, pirboss )

   -- On the way to the Spob
   local pos1 = vec2.new( -5e3, 5e3 )
   local miniboss1 = spawn_pirate( "Pirate Vendetta", pos1)
   spawn_pirate( "Pirate Hyena", pos1, miniboss1 )
   spawn_pirate( "Pirate Ancestor", pos1, miniboss1 )

   -- In the asteroid field
   local pos2 = vec2.new( -3e3, -3e3 )
   local miniboss2 = spawn_pirate( "Pirate Rhino", pos2 )
   spawn_pirate( "Pirate Vendetta", pos2, miniboss2 )

   hook.timer( 5, "pirate_spam" )
end

function pirate_spam ()
   if not pirboss or not pirboss:exists() then
      return
   end

   pirboss:broadcast(_("This system is now mine! Prepare to be dominated!"))
   player.autonavReset( 5 )
end

function pir_gone ()
   -- Check if done
   local left = 0
   for k,p in ipairs(baddies) do
      if p:exists() then
         left = left+1
      end
   end
   if left > 0 then
      player.msg(fmt.f(n_(
         [[{left} marauder left to clear the blockade.]],
         [[{left} marauders left to clear the blockade.]], left),{left=left}))
      return
   end

   lmisn.sfxVictory()
   player.msg(fmt.f(_("You have cleared the blockade on {spb}!"),{spb=mainspb}))
   player.landAllow( true )
   diff.apply( "Levo Safe" ) -- Removes 'restricted' tag
   evt.finish(true)
end

function comm_levo( commspb )
   if commspb ~= mainspb then
      return
   end

   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, mainspb )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the authorities at {spb}."),
      {spb=mainspb}))

   spb(fmt.f(_([["You aren't with the pirates are you?"
After a quick confirmation they continue.
"Help us! Some pirate called '{pirate}' is trying to dominate us!"]]),
      {pirate=piratename}))
   spb(_([["Please clear out marauders in the system and we'll let you land!"]]))
   vn.na(_([[The communication abruptly cuts off. It seems like they could use your help.]]))

   vn.run()

   player.commClose()
end
