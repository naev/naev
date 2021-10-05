--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Assault">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>3</priority>
   <done>The Patrol</done>
   <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
   <chance>100</chance>
   <location>Bar</location>
   <planet>The Wringer</planet>
  </avail>
  <notes>
   <campaign>Heretic</campaign>
  </notes>
 </mission>
 --]]
--[[misn title - the assault]]
--[[in this mission, the wringer is assaulted by a full assault fleet
   sent in by the threatened Sirius. The player attempts to defend,
   when is instead ordered back to The Wringer to escape Sirius controlled
   space. thanks to nloewen and viashimo for help!]]

local fleet = require "fleet"
local fmt = require "format"
local srs = require "common.sirius"


--beginning messages

--ending messages

--mission OSD
osd = {}
osd[1] = _("Defend %s against the Sirius assault")
osd[2] = _("Return to %s")
--random odds and ends
misn_title = _("The Assault")
npc_name = _("Draga")
misn_desc = _([[A Sirius assault fleet has just jumped into %s. You are to assist Nasin in destroying this fleet.]])

log_text = _([[You helped Draga in an attempt to protect Nasin from the Sirius military. Draga ordered you to get your ship ready for another battle and meet him at the bar.]])


function create()
   --this mission makes one mission claim, in Suna.
   --initialize your variables
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker")
   reward = math.floor((100000+(math.random(5,8)*2000)*(nasin_rep^1.315))*.01+.5)/.01
   planding = 0
   homeasset, homesys = planet.cur()
   msg_checker = 0
   --set the mission stuff
   if not misn.claim(homesys) then
      misn.finish(false)
   end
   misn.setReward( fmt.credits( reward ) )
   misn.setTitle( misn_title )
   misn.setNPC(npc_name, "sirius/unique/draga.webp", _("The familiar form of Draga is at a table with some officers. They look busy."))

   osd[1] = osd[1]:format(homeasset:name())
   osd[2] = osd[2]:format(homeasset:name())

   misn_desc = misn_desc:format(homesys:name())
end

function accept()
   if tk.yesno(misn_title, _([[Draga is sitting at a table with a couple other people who appear to be official military types. They look at you as you approach. Draga stands and greets you. "Hello, %s," he says. "We have a situation, and we need your help.
    "The Sirii are starting to take us seriously as a threat. This isn't what we hoped for; we hoped we could go a little longer underground, but it seems we're being forced into battle. As such, we need you to help us defend our system. Our goal here isn't to completely wipe out the Sirius threat, but rather just to drive them off and show them that we mean business. We want them to feel it.
    "You will be outnumbered, outgunned, and officially declared an enemy of the state. Will you help us?"]]):format(player.name())) then
      tk.msg( misn_title, _([["Excellent! See, folks? I told you this one was a keeper! Our forces will meet you out there. Ah, and while I'm sure you would never do this, as before, desertion will not be tolerated. Do not land or leave the system until your mission is completed."]]) )

      misn.accept()
      misn.setDesc(misn_desc)
      misn.markerAdd(homesys,"high")
      misn.osdCreate(misn_title,osd)
      misn.osdActive(1)

      --hook time.
      hook.takeoff("takeoff")
      hook.jumpin("out_sys_failure")
      hook.land("return_to_base")
   else
      tk.msg( misn_title, _([["Do you not understand the seriousness of this situation?! I thought you were better than this!" He grumbles and shoos you away.]]) )
      misn.finish( false )
   end
end

function takeoff() --for when the player takes off from the wringer.
   pilot.clear() --clearing out all the pilots, and
   pilot.toggleSpawn("Sirius",false) --making the Sirius not spawn. I want the assault fleet the only Sirius in there.
   deathcounter = 0 -- Counts destroyed Nasin ships.
   local assault_force = {"Sirius Divinity", "Sirius Dogma", "Sirius Dogma", "Sirius Preacher", "Sirius Preacher", "Sirius Preacher", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity"}
   sirius_be_serious = fleet.add( 1, assault_force, "Sirius", system.get("Herakin") )

   for _,p in ipairs(sirius_be_serious) do
      p:setHilight()
      p:setNoJump()
      p:setNoLand()
      p:setHostile() --just in case. makes thing easier.
   end

   de_fence = fleet.add( 2, {"Shark", "Lancelot", "Lancelot", "Admonisher"}, "Nasin", homeasset, {_("Nasin Shark"), _("Nasin Lancelot"), _("Nasin Lancelot"), _("Nasin Admonisher")} )
   de_fence_2 = fleet.add( 2,  {"Shark", "Lancelot", "Lancelot", "Admonisher"}, "Nasin", vec2.new(rnd.rnd(25,75),rnd.rnd(100,350)), {_("Nasin Shark"), _("Nasin Lancelot"), _("Nasin Lancelot"), _("Nasin Admonisher")} )

   for _,p in ipairs(de_fence) do
      p:setNoJump()
      p:setNoLand()
      p:setFriendly() --the green more clearly defines them as allies.
      hook.pilot(p,"death","death")
   end

   for _,p in ipairs(de_fence_2) do
      p:setNoJump()
      p:setNoLand()
      p:setFriendly( true )
      hook.pilot(p,"death","death")
   end
   hook.timer(90.0,"second_coming") --i wanted the player to feel some hope that he'd win, but have that hope come crashing down.
   hook.timer(97.0,"second_coming")
   hook.timer(145.0,"second_coming")
end

function death(p)
   deathcounter = deathcounter + 1
   if deathcounter >= 9 then --9 ships is all the ships in the first fleet minus the 2 cruisers and the carrier. might adjust this later.
      flee()
   end
end

function flee()
   returnchecker = true --used to show that deathcounter has been reached, and that the player is landing 'just because'
   misn.osdActive(2)
   tk.msg(misn_title, _([[You receive a frantic message from Draga. "%s! This is worse than we ever thought. We need you back at the base! Stat!"]]):format( player.name() ))
   -- Send any surviving Nasin ships home.
   for _, j in ipairs(de_fence) do
      if j:exists() then
         j:control()
         j:land(homeasset)
         j:hookClear() -- So we don't trigger death() again.
      end
   end
   for _, j in ipairs(de_fence_2) do
      if j:exists() then
         j:control()
         j:land(homeasset)
         j:hookClear() -- So we don't trigger death() again.
      end
   end
end

function out_sys_failure() --feel like jumping out? AWOL! its easier this way. trust me.
   tk.msg(misn_title,_([[You receive a scathing angry message from Draga chastising you for abandoning your mission. You put it behind you. There's no turning back now.]]))
   faction.modPlayerSingle("Nasin",-50)
   misn.finish(false)
end

function second_coming()
   local assault_force = {"Sirius Divinity", "Sirius Dogma", "Sirius Dogma", "Sirius Preacher", "Sirius Preacher", "Sirius Preacher", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity", "Sirius Fidelity"}
   sirius_be_serious_2 = fleet.add( 1, assault_force, "Sirius", system.get("Herakin") )
   for i,p in ipairs(sirius_be_serious_2) do
      table.insert(sirius_be_serious,p) --inserting into the original table, for the death function.
      p:setHilight()
      p:setNoJump()
      p:setNoLand()
      p:setHostile(true)
   end
end

function return_to_base()
   if not returnchecker then --feel like landing early? AWOL!
      tk.msg(misn_title,_([[As you land, Draga sees you. He seems just about ready to kill you on the spot. "You abandon us now? When we need you the most?! I should never have put my trust in you! Filth! Get out of my sight before I kill you where you stand!" You do as he says, beginning to question your decision to abandon your mission at the very place Draga was. Nonetheless, you bury your head and make a mental note to get out of here as soon as possible.]]))
      faction.modPlayerSingle("Nasin",-50)
      misn.finish(false) --mwahahahahaha!
   else
      player.pay(reward)
      tk.msg(misn_title,_([[As you land, you see the Nasin forces desperately trying to regroup. "Hurry and get your ship ready for another battle," he says, "and meet me at the bar when you're ready! Payment has been transferred into your account. More importantly, we have a dire situation!"]]):format( player.name() ))
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",10)
      var.push("heretic_misn_tracker",misn_tracker)
      srs.addHereticLog( log_text )
      misn.finish(true)
   end
end
