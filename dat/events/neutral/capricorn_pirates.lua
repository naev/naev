--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Capricorn Pirates">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Save Durea!

   Similar to the Levo Pirates, but now in Capricorn.
--]]
local vn = require 'vn'
local fmt = require 'format'
local ccomm = require "common.comm"
local lmisn = require "lmisn"

local mainspb, mainsys = spob.getS("Durea")
local piratename = _("Brown Goatee")

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
   hook.hail_spob( "comm_durea" )
   player.allowLand( false, _("You can not land until you clear the pirates from the system!") )

   -- Spawn pirates
   local function spawn_pirate( shipname, pos, boss )
      local p = pilot.add( shipname, "Marauder", pos + vec2.newP( 100*rnd.rnd(), rnd.angle() ), nil, {ai="guard"} )
      p:setHostile(true)
      local m = p:memory()
      m.bribe_no = _([["You ain't payin' yer wait outta this one!"]])
      m.refual_no = _([["Do I look like a fuel station?"]])
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

   -- On the way to the Spob from the south
   local pos1 = vec2.new( 0, -16e3 )
   local miniboss1 = spawn_pirate( "Pirate Rhino", pos1)
   spawn_pirate( "Pirate Hyena", pos1, miniboss1 )
   spawn_pirate( "Pirate Ancestor", pos1, miniboss1 )

   -- On the way to the spob from the north
   local pos2 = vec2.new( -10e3, 11e3 )
   local miniboss2 = spawn_pirate( "Pirate Rhino", pos2 )
   spawn_pirate( "Pirate Vendetta", pos2, miniboss2 )

   -- Near uninhabited planet
   local pos3 = vec2.new( 13e3, 3e3 )
   local miniboss3 = spawn_pirate( "Pirate Admonisher", pos3 )
   spawn_pirate( "Pirate Shark", pos3, miniboss3 )
   spawn_pirate( "Pirate Hyena", pos3, miniboss3 )

   hook.timer( 5, "pirate_spam" )
end

function pirate_spam ()
   if not pirboss or not pirboss:exists() then
      return
   end

   pirboss:broadcast(_([["Har har har. Nothing like system domination!"]]))
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
   player.allowLand( true )
   diff.apply( "Capricorn Safe" ) -- Removes 'restricted' tag
   evt.finish(true)
end

function comm_durea( commspb )
   if commspb ~= mainspb then
      return
   end

   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, mainspb, false )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the authorities at {spb}."),
      {spb=mainspb}))

   spb(_([[Although the line is connected, it is strangely silent. You can only hear the faint noise of someone chewing on the other side.]]))
   vn.menu{
      {_([["Hello?"]]), "cont01"},
      {_([["Are you all right?"]]), "cont01"},
      {_([[...]]), "cont01_silent"},
   }

   vn.label("cont01_silent")
   vn.na(_([[The mutual silent continues for an uncomfortably long time before the person on the line coughs and breaks the mood.]]))

   vn.label("cont01")
   spb(_([["You aren't with them pirates are ya?"]]))
   vn.menu{
      {_([["Of course not!"]]), "cont02"},
      {_([["Maybe."]]), "cont02"},
      {_([[...]]), "cont02_silent"},
   }

   vn.label("cont02_silent")
   vn.na(_([[You continue giving them the silent treatment. Eventually the communication channel gets cut. Maybe you should get rid of the pirates in the system for them.]]))
   vn.done()

   vn.label("cont02")
   spb(_([["Not sure if I can trust ya."
The person speaks quite nonchalantly, as if not really affected by the fact that the system is currently blockaded by a bunch of pirates.]]))
   spb(_([["You could be or could be not with the bunch of pirates out there makin' a ruckus. Not really my problem."]]))
   spb(_([["Anyway, we've got the base clamped down, and unless you fancy landing on the abrasive rocks and ice in a raging blizzard, you gotta' wait until our scanner says no pirates abroad."]]))
   spb(_([[You hear a loud yawn, and the communication cuts off.]]))
   vn.na(fmt.f(_([[Looks like you'll have to clear the pirates out of the system if you want to be able to land on {spb}.]]),
      {spb=mainspb}))

   vn.run()

   player.commClose()
end
