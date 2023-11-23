--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Crimelord">
 <unique />
 <priority>3</priority>
 <chance>10</chance>
 <location>Bar</location>
 <cond>
   if system.get("Ogat"):jumpDist() ~= 4 or player.jumps() &lt; 4 then
      return false
   end
   local misn_test = require("misn_test")
   if not misn_test.heavy_combat_vessel(true) then
      return false
   end
   return misn_test.reweight_active()
 </cond>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[ Test for a chase mission
In this mission, you will be chased by a pirate fleet across several systems.

MISSION: Chase Test
DESCRIPTION: Pirates chase you to Ogat.
]]--
local fmt = require "format"
local fleet = require "fleet"
local equipopt = require "equipopt"
local vn = require "vn"
local vntk = require "vntk"
local portrait = require "portrait"
local ccomm = require "common.comm"

local reward = 600e3

local capship, defenders, thugs -- Non-persistent state

local npc_name = _("A detective")
local npc_portrait = "neutral/unique/hunter.webp"
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   mem.targetsystem = system.get("Ogat")
   if not misn.claim( mem.targetsystem, true ) then
      misn.finish( false )
   end
   misn.setNPC( npc_name, npc_portrait, _("A private detective is signalling you to come speak with him.") )
end

function accept ()
   -- Note: this mission does not make any system claims.
   local accepted = false
   vn.clear()
   vn.scene()
   local det = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   det(fmt.f(_([[The private detective greets you and gets right down to business.
   "I have tracked down and collected evidence against a local crime lord," he says. "The evidence is on this data disk. They would love nothing more than to get their hands on this. I want you to bring this to my associates in the {sys} system. While the local authorities have proven corruptible, my associates will ensure that this criminal ends up in prison, where they belong."]]),
      {sys=mem.targetsystem}))
   det(fmt.f(_([["I must warn you, however: This criminal has considerable influence and many friends. There's no doubt they will send some of their mercenaries to stop you. You'll need a fast ship to shake them off. My associates will compensate you generously when you reach {sys}.
Regrettably, you are not the first pilot I've contacted regarding this matter. Your predecessor was intercepted when they landed en route to {sys}. The crime lord has many underlings lurking in nearby spaceports -- you must NOT land until you've delivered the data."
Given the dangers, you're not sure whether the reward will make this worth your while. Do you accept?]]),
      {sys=mem.targetsystem}))
   vn.menu{
      {_("Accept"),"accept"},
      {_("Decline"),"decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   det(_([[After quickly glancing around to make sure nobody's taken a particular interest, the detective presses the data stick into your hand.
"Be careful out there. I doubt you'll be able to get far without being noticed."]]))

   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Crimelord") )
   misn.setReward( _("A generous compensation") )
   misn.setDesc( fmt.f( _("Evade the thugs and deliver the evidence to {sys}"), {sys=mem.targetsystem} ) )
   misn.markerAdd( mem.targetsystem, "low" )
   misn.osdCreate(_("Crimelord"), {fmt.f(_("Evade the thugs and deliver the evidence to {sys}"), {sys=mem.targetsystem})})

   mem.startsystem = system.cur() --needed to make thugs appear random in the first system
   mem.last_system = system.cur() --ignore this one, it's just the initialization of the variable

   hook.enter("enter")
   hook.jumpout("jumpout")
   hook.land("land")
end

local fct_baddie, fct_goodie
function enter ()
   fct_goodie = faction.dynAdd( nil, "crimelord_associates", _("Associates"), {ai="dvaered"} )
   fct_baddie = faction.dynAdd( nil, "crimelord_thugs", _("Thugs"), {ai="dvaered" } )
   fct_baddie:dynEnemy( fct_goodie )

   -- Only spawn if system can handle inclusive claims
   if naev.claimTest( system.cur(), true ) then
      hook.timer(4.0, "spawnBaddies")
   end

   if system.cur() == mem.targetsystem then
      local defenderships = { "Lancelot", "Lancelot", "Admonisher", "Pacifier", "Hawking", "Kestrel" }
      local jumpin = jump.pos(mem.targetsystem, mem.last_system)
      defenders = fleet.add( 1, defenderships, fct_goodie, jumpin )
      for pilot_number, pilot_object in pairs(defenders) do
         local rn = pilot_object:ship():nameRaw()
         if rn == "Lancelot" then
            pilot_object:rename(_("Associate Lancelot"))
         elseif rn == "Admonisher" then
            pilot_object:rename(_("Associate Admonisher"))
         elseif rn == "Pacifier" then
            pilot_object:rename(_("Associate Pacifier"))
         elseif rn == "Hawking" then
            pilot_object:rename(_("Associate Hawking"))
         elseif rn == "Kestrel" then
            pilot_object:rename(_("Associate Kestrel"))
         end

         pilot_object:setFriendly()
         pilot_object:setPos( pilot_object:pos() +
         vec2.new( rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2,
         rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2))
      end

      capship = defenders[#defenders]
      capship:setInvincible()
      capship:comm(_("We've got your back. Engaging hostiles."), true )
   end
end

function jumpout ()
   mem.last_system = system.cur()
end

function spawnBaddies ()

   local ai
   if system.cur() ~= mem.targetsystem then
      ai = "baddiepos"
   elseif mem.last_system == mem.startsystem then
      ai = "baddie"
   else
      ai = "baddie_norun"
   end

   local sp = nil
   if mem.last_system ~= system.cur() then
      sp = mem.last_system
   end

   local pp = player.pilot()
   thugs = fleet.add( 4, "Admonisher", fct_baddie, sp, _("Thug"), {ai=ai, naked=true} )
   for pilot_number, pilot_object in ipairs(thugs) do
      pilot_object:setHostile(true)
      equipopt.dvaered( pilot_object, { launcher=0, turret=0, fighterbay=0 } )
      if ai=="baddiepos" then
         pilot_object:memory().guardpos = pp:pos()
      else
         mem.thugs_alive = #thugs
         hook.pilot(pilot_object, "exploded", "pilotKilled")
      end
   end
   local threats = {
      _("Surrender now and we'll let you live."),
      _("You're dead!"),
      _("You won't make it out alive!"),
      _("Get back here!")
   }
   thugs[1]:comm(threats[rnd.rnd(1,#threats)],true)
end

function pilotKilled ()
   mem.thugs_alive = mem.thugs_alive - 1
   if mem.thugs_alive == 0 then
      capship:hailPlayer()
      hook.pilot(capship, "hail", "capHailed")
   end
end

function capHailed ()
   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, capship )
   vn.transition()
   p(fmt.f(_([["Excellent work. This data will ensure an arrest and swift prosecution. You've certainly done your part towards cleaning up the region. As for your compensation, I've had {credits} transferred to you."]]),
      {credits=fmt.credits(reward)}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( reward )
   end )
   vn.na(fmt.reward(reward))
   vn.run()

   player.commClose()
   misn.finish(true)
end

function land ()
   vntk.msg( _("He told you so1…"), _("As you step out of your ship and seal the airlock, you spot a burly man purposefully heading towards you. You turn to flee, but there are others closing in on your position. Surrounded, and with several laser pistols trained on you, you see no option but to surrender the evidence."))
   misn.finish(false)
end
