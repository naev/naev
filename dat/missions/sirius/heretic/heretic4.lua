--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Egress">
 <unique />
 <priority>3</priority>
 <done>The Assault</done>
 <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
 <chance>100</chance>
 <location>Bar</location>
 <spob>The Wringer</spob>
 <notes>
  <campaign>Heretic</campaign>
 </notes>
</mission>
--]]
--[[misn title - the egress]]
--[[this mission begins with the frenetic Nasin wanting to escape
   The Wringer due to being overwhelmed by House Sirius. The player
   loads up with as many Nasin as their vessel will carry, and takes
   them to seek refuge in the ingot system on planet Ulios, where they
   begin to rebuild and plan.... (ominous music).
   this mission is designed to be the end of part 1, and is supposed
   to be very hard, and slightly combat oriented, but more supposed to
   involve smuggling elements.]]

local fleet = require "fleet"
local fmt = require "format"
local srs = require "common.sirius"


-- Mission constants
local targetasset, targetsys = spob.getS("Ulios") --this will be the new HQ for the Nasin in the next part.

function create()
   --this mission make no system claims.
   --initialize your variables
   mem.nasin_rep = faction.playerStanding("Nasin")
   mem.misn_tracker = var.peek("heretic_misn_tracker")
   mem.reward = math.floor((100e3+(math.random(5,8)*2e3)*(mem.nasin_rep^1.315))*.01+.5)/.01
   mem.homeasset = spob.cur()
   --set some mission stuff
   misn.setNPC(_("Draga"), "sirius/unique/draga.webp", _("Draga is running around, helping the few Nasin in the bar to get stuff together and get out."))
end

function accept()
   --initial convo. Kept it a yes/no to help with the urgent feeling of the situation.

   local msg = fmt.f(_([[You run up to Draga, who has a look of desperation on his face. "We need to go, now," he says. "The Sirii are overwhelming us, they're about to finish us off. Will you take me, and as many Nasin as you can carry, to {pnt} in the {sys} system? This is our most desperate hour!"]]), {pnt=targetasset, sys=targetsys} )
   if not tk.yesno(_("The Egress"), msg) then
      return
   end
   misn.accept()
   player.allowSave(false) -- so the player won't get stuck with a mission they can't complete.
   tk.msg(_("The Egress"),_([["Thank you! I knew you would do it!" Draga then proceeds to file as many people as can possibly fit onto your ship, enough to fill your ship's cargo hold to the brim. The number of Nasin members shocks you as they are packed into your ship.
    As the Sirii approach ever closer, Draga yells at you to get the ship going and take off. You begin taking off just in time to see Draga under fire by a Sirian soldier who has infiltrated the base. The last thing you see as you take off is him lying on the ground, lifeless.]]))
   --convo over. time to finish setting the mission stuff.
   misn.markerAdd(targetasset, "high")
   local free_cargo = player.pilot():cargoFree()
   mem.people_carried =  (16 * free_cargo) + 7 --average weight per person is 62kg. one ton / 62 is 16. added the +7 for ships with 0 cargo.
   misn.setTitle(_("The Egress"))
   misn.setReward(mem.reward)
   misn.setDesc(fmt.f(_("Assist the Nasin refugees by flying to {pnt} in {sys}, and unloading them there."), {pnt=targetasset, sys=targetsys}))
   misn.osdCreate(_("The Egress"), {
      fmt.f(_("Fly the refugees to {pnt} in the {sys} system."), {pnt=targetasset, sys=targetsys}),
   })
   local c = commodity.new( N_("Refugees"), N_("Nasin refugees.") )
   mem.refugees = misn.cargoAdd(c,free_cargo)
   player.takeoff()
   --get the hooks.
   hook.takeoff("takeoff")
   hook.jumpin("attacked")
   hook.jumpout("lastsys")
   hook.land("misn_over")
end

function takeoff()
   local assault_force = {
      "Sirius Divinity",
      "Sirius Dogma",
      "Sirius Dogma",
      "Sirius Preacher",
      "Sirius Preacher",
      "Sirius Preacher",
      "Sirius Fidelity",
      "Sirius Fidelity",
      "Sirius Fidelity",
      "Sirius Fidelity",
      "Sirius Fidelity",
      "Sirius Fidelity" }
   fleet.add( 1, assault_force, "Sirius", vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450)) ) --left over fleets from the prior mission.
   fleet.add( 1, assault_force, "Sirius", vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450)) )
   fleet.add( 1, assault_force, "Sirius", vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450)) )
   fleet.add( 3, {"Mule", "Llama", "Llama"}, "Nasin", mem.homeasset, {_("Nasin Mule"), _("Nasin Llama"), _("Nasin Llama")}, {ai="civilian"} ) --other escapees.
   fleet.add( 2, {"Admonisher", "Lancelot", "Lancelot", "Lancelot"}, "Nasin", mem.homeasset, {_("Nasin Admonisher"), _("Nasin Lancelot"), _("Nasin Lancelot"), _("Nasin Lancelot")} ) --these are trying to help.
end

function lastsys()
   mem.last_sys_in = system.cur()
end

function attacked() --several systems where the Sirius have 'strategically placed' an assault fleet to try and kill some Nasin.
   local dangersystems = {
   system.get("Neon"),
   system.get("Pike"),
   system.get("Vanir"),
   system.get("Aesir"),
   system.get("Herakin"),
   system.get("Eiderdown"),
   system.get("Eye of Night"),
   system.get("Lapis"),
   system.get("Ruttwi"),
   system.get("Esker"),
   system.get("Gutter")
   }
   local assault_force = {"Sirius Divinity", "Sirius Dogma", "Sirius Dogma", "Sirius Preacher", "Sirius Preacher", "Sirius Preacher", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity"}
   for i,sys in ipairs(dangersystems) do
      if system.cur() == sys then
         fleet.add( 1, assault_force, "Sirius", vec2.new(rnd.rnd(-300,300),rnd.rnd(-300,300)) )
      end
   end
   local chance_help,chance_civvie = rnd.rnd(1,3),rnd.rnd(1,3) --attack fleet and civvies are meant as a distraction to help the player.
   if chance_help == 1 then
      fleet.add( 1, {"Admonisher", "Lancelot", "Lancelot", "Lancelot"}, "Nasin", mem.last_sys_in, {_("Nasin Admonisher"), _("Nasin Lancelot"), _("Nasin Lancelot"), _("Nasin Lancelot")} )
   end
   for i = 1,chance_civvie do
      fleet.add( 1, {"Mule", "Llama", "Llama"}, "Nasin", mem.last_sys_in, {_("Nasin Mule"), _("Nasin Llama"), _("Nasin Llama")}, {ai="civilian"} )
   end
end

function misn_over() --aren't you glad thats over?
   if spob.cur() == spob.get("Ulios") then
      --introing one of the characters in the next chapter.
      tk.msg(_("The Egress"), fmt.f(_([[You land on {pnt} and open the bay doors. You are still amazed at how many people Draga had helped get into the cargo hold. As you help everyone out of your ship, a man walks up to you. "Hello, my name is Jimmy. Thank you for helping all of these people. I am grateful. I've heard about you from Draga, and I will be forever in your debt. Here, please, take this." He presses a credit chip in your hand just as you finish helping everyone out of your ship. It seems it was a job well done.]]), {pnt=targetasset} ))
      player.pay(mem.reward)
      misn.cargoRm(mem.refugees)
      mem.misn_tracker = mem.misn_tracker + 1
      faction.modPlayer("Nasin",25) --big boost to the Nasin, for completing the prologue
      var.push("heretic_misn_tracker", mem.misn_tracker)
      misn.osdDestroy()
      player.allowSave(true)
      srs.addHereticLog( _([[You helped rescue as many Nasin as your ship could hold to Ulios. Draga was killed by a Sirian soldier as he attempted to rescue his people. When you made it to Ulios, a man named Jimmy gave you a credit chip and said that he "will be forever in your debt".]]) )
      misn.finish(true)
   end
end

function abort()
   tk.msg(_("The Egress"), fmt.f(_([[You decide that this mission is just too much. You open up the cargo doors and jettison all {n} people out into the cold emptiness of space. The Nasin will hate you forever, but you did what you had to do.]]), {n=fmt.number(mem.people_carried)}))
   misn.cargoJet(mem.refugees)
   faction.modPlayerSingle("Nasin",-200)
   player.allowSave(true)
   misn.finish(true)
end
