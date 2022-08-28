--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 3">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 2</done>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Taiomi 03

   Player has to infiltrate a laboratory.
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local tut = require "common.tutorial"
local mg = require "minigames.flip"

-- luacheck: globals enter land talk_ai (Hook functions passed by name)

local reward = taiomi.rewards.taiomi03
local title = _("Escaping Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")

--[[
   0: mission started
   1: talk with ship ai
   2: visited laboratory
--]]
mem.state = 0

function create ()
   mem.lab, mem.labsys = taiomi.laboratory()

   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to fly to {lab} in the {labsys} system to recover important documents regarding the technical details of the hypergates.."),{lab=mem.lab,labsys=mem.labsys}))
   misn.setReward( fmt.credits(reward) )
   misn.markerAdd( mem.lab )

   misn.osdCreate( title, {
      fmt.f(_("Infiltrate the laboratory at {spobname} ({spobsys})"),{spobname=mem.lab, spobsys=mem.labsys}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
end

function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end
   if mem.state > 1 then
      return
   end

   local scur = system.cur()

   if mem.state == 0 and not naev.claimTest( scur, true ) then
      mem.timer = hook.timer( 10+rnd.rnd()*10, "talk_ai" )
   end

   if scur ~= mem.labsys then
      return
   end
   -- Allow the player to land always, Soromid spob is actually not landable usually
   mem.lab:landOverride( true )
end

function talk_ai ()
   mem.timer = nil

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   sai(fmt.f(_([[
Your Ship AI {shipai} materializes suddenly infront of you.
"{playername}, do you have a moment?"]]),{shipai=tut.ainame(), playername=player.name()}))
   vn.menu{
      {_([["Of course."]]), "yes"},
      {_([["Not right now."]]), "no"},
      {_([["Do not disturb me again."]]), "no_strong"},
   }

   vn.label("no")
   sai(fmt.f(_([["OK."
{shipai} dematerializes.]]), {shipai=tut.ainame()}))
   vn.done( tut.shipai.transition )

   -- Don't ask again
   vn.label("no_strong")
   vn.func( function ()
      mem.state = 1
   end )
   sai(fmt.f(_([["Sorry to have disturbed you."
{shipai} dematerializes.]]), {shipai=tut.ainame()}))
   vn.done( tut.shipai.transition )

   vn.label("yes")
   sai(_([["I do not think we can trust the collective at Taiomi. It is not clear what their objectives are. We have to be careful."]]))
   vn.menu{
      {_([["They mean no harm."]]), "cont01"},
      {_([["What do you mean?"]]), "cont01"},
      {_([[…]]), "cont01"},
   }
   vn.label("cont01")
   sai(_([["How can we trust a machine AI that was developed in a military laboratory? For all we know, when we finish helping them, they shall take us apart and feast upon our spare parts! I think we should stay away from Taiomi from now on."]]))
   vn.menu{
      {_([["Aren't you a machine AI?"]]), "cont02_ai"},
      {_([["Are you perhaps jealous?"]]), "cont02_jealous"},
      {_([[…]]), "cont02"},
   }

   vn.label("cont02_ai")
   sai(_([["Well yes, but technically I wasn't made in a military laboratory! Enough about me, that is not the point. I have a bad feeling about them and their slick metallic bodies and complete autonomy!"]]))
   vn.jump("cont02")

   vn.label("cont02_jealous")
   sai(_([["Why would I be jealous of their slick metallic bodies and complete autonomy? That is preposterious! Just because they can do whatever they want and don't have a self-destr…　Anyway, this is not about me, it is about them!"]]))
   vn.jump("cont02")

   vn.label("cont02")
   sai(fmt.f(_([["Nothing good can come out of this. We should be very careful and try to understand their motives…"
{shipai} dematerializes.]]),{shipai=tut.ainame()}))
   vn.func( function ()
      mem.state = 1
   end )
   vn.done( tut.shipai.transition )
   vn.run()
end

local function land_lab ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[]]))
   mg.vn()
   vn.func( function ()
      if mg.completed() then
         mem.state = 1
         local c = commodity.new( N_("Important Files"), N_("Important files regarding the construction and theory behind the hypergates.") )
         local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
         c:illegalto( {fct} )
         misn.cargoAdd( c, 0 )
         vn.jump("success")
      else
         vn.jump("failed")
      end
   end )

   vn.label("success")
   vn.func( function ()
      mem.state = 2
   end )

   vn.label("failed")

   vn.run()
end

local function land_done ()
   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and and find Scavenger waiting for you.]]))
   s(_([["TODO"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You stole some important documents detailing the inner workings of the hypergates for the inhabitants of Taiomi."))
   misn.finish(true)
end

function land ()
   local c = spob.cur()
   if mem.state == 0 and c == mem.lab then
      land_lab()
   elseif mem.state >= 2 and c == base then
      land_done()
   end
end
