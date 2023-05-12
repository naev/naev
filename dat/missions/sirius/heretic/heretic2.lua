--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Patrol">
 <unique />
 <priority>3</priority>
 <done>The Return</done>
 <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
 <chance>100</chance>
 <location>Bar</location>
 <spob>The Wringer</spob>
 <notes>
   <campaign>Heretic</campaign>
   <tier>3</tier>
 </notes>
</mission>
 --]]
--[[misn title - the patrol]]
--[[in this mission, the player will be guarding the "high command" of the
   nasin, The Wringer / Suna. House Sirius is sending in recon parties.
   The player's job is to take out any and all Sirius in the system.]]

local fleet = require "fleet"
local fmt = require "format"
local srs = require "common.sirius"

local attackers, recon -- Non-persistent state

function create()
   --this mission does make one system claim, in Suna.
   --initialize the variables
   mem.homeasset, mem.homesys = spob.cur()
   if not misn.claim(mem.homesys) then
      misn.finish(false)
   end
   mem.nasin_rep = faction.playerStanding("Nasin")
   mem.misn_tracker = var.peek("heretic_misn_tracker")
   mem.reward = math.floor((100e3+(math.random(5,8)*2e3)*(mem.nasin_rep^1.315))*.01+.5)/.01
   mem.chronic = 0
   mem.finished = 0
   mem.takeoff_counter = 0
   mem.draga_convo = 0
   mem.deathcount = 0
   --set the mission stuff
   misn.setTitle(_("The Patrol"))
   misn.setReward(mem.reward)
   misn.setNPC(_("An Imposing Man"), "sirius/unique/draga.webp", _("This man leans against the bar while looking right at you."))
end

function accept()
   if tk.yesno( _("The Patrol"), fmt.f(_([[You walk up to the intimidating man. He's dressed smartly in cool, dark black business attire, with a large smile spread across his face.
    "Ahh, so you're {player}! Everyone has been talking about you," he says. "Allow me to introduce myself. My name is Draga, high commander of Nasin's operations. You remember us, right? People in our organization have started to take notice of your actions. Maybe it is the will of Sirichana that you come to us! If that is the case, then I have an offer you can't refuse. A chance to really prove yourself as more than a glorified courier. You see... we are expecting a Sirian patrol any hectosecond now, and we want you to... take care of it. What do you say? We could really use your help."]]), {player=player.name()} ) ) then
      misn.accept()
      tk.msg( _("The Patrol"), _([["Marvelous! I knew I could count on you! Don't you worry; you'll be fighting alongside some of our finest pilots. I know you can drive those Sirii off!
    "Oh, and one last thing: don't even think of bailing out on us at the last second. If you jump out or land before your mission is completed, consider yourself fired."]]) )

      misn.setDesc(fmt.f(_("You have been hired once again by Nasin, this time to destroy a Sirius patrol that has entered {sys}."), {sys=mem.homesys}))
      misn.markerAdd(mem.homeasset, "high")
      misn.osdCreate(_("The Patrol"), {
         _("Destroy the Sirius patrol"),
         fmt.f(_("Land on {pnt}"), {pnt=mem.homeasset} ),
      })
      misn.osdActive(1)
      hook.takeoff("takeoff")
      hook.jumpin("out_sys_failure")
      hook.land("land")
   else
      tk.msg( _("The Patrol"), _([["Gah! I should have known you would be so spineless! Get out of my sight!"]]) )
   end
end

function takeoff()
   pilot.clear()
   pilot.toggleSpawn("Sirius",false) --the only Sirius i want in the system currently is the recon force
   recon = fleet.add( 1, {"Sirius Preacher", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity"}, "Sirius", system.get("Herakin") )
   attackers = fleet.add( 1, {"Admonisher", "Lancelot", "Lancelot", "Lancelot"},
                            "Nasin", mem.homeasset,
                            {_("Nasin Admonisher"), _("Nasin Lancelot"), _("Nasin Lancelot"), _("Nasin Lancelot")} ) --a little assistance
   mem.n_recon = #recon --using a deathcounter to track success
   for i,p in ipairs(recon) do
      p:setHilight(true)
      p:setNoJump(true) --don't want the enemy to jump or land, which might happen if the player is slow, or neutral to Sirius.
      p:setNoLand(true)
      p:setHostile(true)
   end
   for i,p in ipairs(attackers) do
      p:setNoJump(true)
      p:setNoLand(true)
      p:setFriendly(true)
   end
   hook.pilot(nil,"death","death")
end

function death(p)
   for i,v in ipairs(recon) do
      if v == p then
         mem.deathcount = mem.deathcount + 1
      end
   end
   if mem.deathcount == mem.n_recon then --checks if all the recon pilots are dead.
      misn.osdActive(2)
      mem.finished = 1
   end
end

function land()
   if mem.finished ~= 1 then
      tk.msg(_("The Patrol"),_([[Draga's face goes red with fury when he sees you. For a moment you start to worry he might beat you into a pulp for abandoning your mission, but he moves along, fuming. You breathe a sigh of relief; you may have angered Nasin, but at least you're still alive.]])) --landing pre-emptively is a bad thing.
      faction.modPlayerSingle("Nasin",-20)
      misn.finish(false)
   elseif spob.cur() == mem.homeasset and mem.finished == 1 then
      tk.msg(_("The Patrol"),_([[You land, having defeated the small recon force, and find Draga with a smile on his face. "Great job!" he says. "I see you really are what you're made out to be and not just some overblown merchant!" He hands you a credit chip. "Thank you for your services. Meet us in the bar again sometime. We will certainly have another mission for you."]]))
      player.pay(mem.reward)
      mem.misn_tracker = mem.misn_tracker + 1
      faction.modPlayer("Nasin",7)
      var.push("heretic_misn_tracker", mem.misn_tracker)
      srs.addHereticLog( _([[You eliminated a Sirian patrol for Draga, high commander of Nasin's operations. He said that Nasin will have another mission for you if you meet him in the bar on The Wringer again.]]) )
      misn.finish(true)
   end
end

function out_sys_failure() --jumping pre-emptively is a bad thing.
   if system.cur() ~= mem.homesys then
      tk.msg(_("The Patrol"), _([[As you abandon your mission, you receive a message from Draga saying that Nasin has no need for deserters. You hope you made the right decision.]]))
      faction.modPlayerSingle("Nasin",-20)
      misn.finish(false)
   end
end
