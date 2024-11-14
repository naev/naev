--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Old Friends at Protera Husk">
 <unique />
 <chance>100</chance>
 <location>Bar</location>
 <spob>Hypergate Protera</spob>
 <tags>
  <tag>fleetcap_10</tag>
 </tags>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
  <done_evt name="Welcome to Wild Space" />
 </notes>
</mission>
--]]
--[[
   Wild Space Introduction

   Small mission to recover something from Protera Husk. Mainly for Lore / Flavour.
]]--
local vn = require "vn"
local vni = require "vnimage"
local vne = require "vnextras"
local fmt = require "format"
local tut = require "common.tutorial"
local ws = require "common.wildspace"

local title = _("Old Friends at Protera Husk")
local target, targetsys = spob.getS( "Protera Husk" )
local main, mainsys = spob.getS( "Hypergate Protera" )
local flost = faction.get("Lost")

--[[
   Mission states:
0: started
1: landed on target
2: spawned chasers
--]]
mem.state = 0

local npc
local function wildspace_start_misn ()
   misn.accept()
   if npc then
      misn.npcRm( npc )
      npc = nil
   end

   misn.setDesc(fmt.f(_("You've been asked to find a parcel on {target} ({targetsys} system) and bring it to {main} ({mainsys} system)."),
      {target=target, targetsys=targetsys, main=main, mainsys=mainsys}))
   misn.setReward(_("Unknown"))

   misn.markerAdd( target )
   misn.osdCreate( title, {
      fmt.f(_([[Land on {spb} ({sys} system).]]), {spb=target, sys=targetsys}),
      fmt.f(_([[Return to {spb} ({sys} system).]]), {spb=main, sys=mainsys}),
   })
   mem.state = 0

   var.push("protera_husk_canland", true)
   hook.land( "land" )
   hook.enter( "enter" )
end

function create ()
   if var.peek("wildspace_start_misn") then
      wildspace_start_misn()
      var.pop("wildspace_start_misn")
      return
   end

   npc = misn.npcAdd( "approach", _("C"), "unknown.webp", _("Get in touch with the voice over the speakers.") )
end

function approach ()
   local accept
   vn.clear()
   vn.scene()
   local c = vn.newCharacter( vni.soundonly( p_("character","C") ) )
   vn.transition()

   c(fmt.f(_([["Will you do me a favour and pick up the important items that my roommate Ben left behind on {spb}?"]]),{spb=target}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline for now"), "decline"},
   }

   vn.label("decline")
   vn.na(_([[You decline for now to get more time to prepare.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accept = true end )
   c(_([[They cough.
"Here, take the coordinates. Be careful."]]))
   vn.run()

   if not accept then return end

   wildspace_start_misn()
end

function land ()
   if mem.state==0 and spob.cur() == target then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[You try to minimize your ship visibility during landing, but it's likely that it won't be long until you get unwanted local attention. You will have to be swift. Just in case, you double-check your hand weapons.]]))
      vn.na(_([[Without the niceties of a proper landing area, you try to find a clearing, but at the end, have to make do with precariously setting your ship on the rubble of a once majestic building. A bit more visible than what you were hoping for. Checking with the coordinates you were given, it doesn't seem like you are too far. Now to make haste.]]))
      vn.na(_([[Staying low you make your way across the rubble. No sign of life for now other some fungus that grows on the cracks and crevices throughout the decrepit structures. You notice some sort of tattered flag nearby, it seems like that is your destination.]]))
      vn.na(_([[You sneak up closer and survey the surroundings. It looks there is a ruined building at your destination. You can't make out a clear door, but there seems to be a hole where a window once was that you should be able to squeeze through.]]))
      vn.na(_([[You get closer and are about to squeeze through, when you hear a growl from behind you. You only make up some glowing purple eyes among a mass of black rags charging at you, as you quickly discharge your blaster into the assailant.]]))
      vn.na(_([[You wince at the familiar smell of burning flesh and reverberating echoes of blaster fire. Looks like you're going to have to make a break for it, or you may not be as lucky next time.]]))
      vn.na(_([[You manage to scrape through the window and quickly turn on your light. There's a layer of dust all over, but seems to be no life forms. You quickly turn on a scanner and manage to find a satchel that seems to have some sort of holodrive or something in it. As efficient as you were, howls in the distant indicate that it may be too late.]]))
      vn.na(_([[Without time to spare, you toss the satchel out the window you came in, and squeeze back after it. Time to get out of here.]]))

      vn.scene()
      local sai = tut.vn_shipai()
      vn.newCharacter( sai )
      vn.transition()
      sai(fmt.f(_([[A holo-image flashes in the corner of your retina.
"{player}, there seems to be many lifeforms converging on your location. My preliminary analysis indicates that they want to skin you alive."]]),
         {player=player.name()}))
      vn.na(fmt.f(_([[No shit. You tell {shipai} to get your ship over to you, or at least provide cover fire and make themselves useful.]]),
         {shipai=tut.ainame()}))

      vn.scene()
      vn.transition()
      vn.na(_([[You grab the satchel and turn a corner, face to face with another two Lost, they hiss and you don't hesitate to drop them both with a couple of well-placed shots. Maybe you'll pull it off. You run towards your ship location, as you notice it seems to be on the move. You pick up the pace when you are suddenly thrown into the air... what the hell was that?]]))
      vn.na(_([[You roll over, and see some shapes charging at you. Trying to focus your blurred vision, you pick up your blaster and fire a few shots... nothing but air. Shit.]]))
      vn.na(_([[You try to take aim again, when another nearby explosion showers you with debris.]]))

      vn.scene()
      vn.newCharacter( sai )
      vn.transition()
      sai(_([["I would recommend against rolling on the ground at the current time."]]))

      vn.scene()
      vn.transition()
      vn.na(_([[You grab the satchel and run onto the ship, just in time it seems. You can make out more figures as the land gears close and you take off. Time to get out of here.]]))
      vn.na(_([[You rush to the command chair and take control, pushing a sharp atmospheric ascent. The ship rattling as it tries to break free from the gravitational pull. As you climb, you hear an unwelcome lock-on warning. Your ship is rocked by some sort of ground-to-air missile. That's going to leave a blemish. You pull the throttle harder and eventually manage to break free.]]))
      vn.na(_([[Although you want a break, your ship reminds you that more take-offs are being detected. Your break will have to wait a bit more.]]))

      vn.run()

      -- Advance mission
      mem.state = 1
      player.takeoff()
      misn.markerRm()
      misn.markerAdd( main )

   elseif mem.state>=1 and spob.cur() == main then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[After your ordeal, you once again make your way through the airlock. It is oddly silent. You arrive the cantina, but hear no welcome. You try to make some noise to catch the attention of the inhabitant but to no avail. Not really knowing what you do, you decide to take a look and delve deeper into the structure.]]))
      vn.na(_([[You find a door that seems to have been remotely unlocked and go through a winding hallway that twists and turns following the damage to the hull. You reach another door, this time it looks more ad-hoc and less reinforced. Out of courtesy, you knock with your weapon to see if anyone responds.]]))
      vn.na(_([[You seemingly have no choice but to kick it down to see what is on the other side. You brace for impact, but the door easily gives under the force of your boot. However, the scene you uncover almost makes you wish you hadn't.]]))
      vn.na(_([[You run back to your ship without looking back and lock yourself in the captain's room as you try to recover.]]))

      vn.scene()
      local sai = tut.vn_shipai()
      vn.newCharacter( sai )
      vn.transition()
      sai(_([[Your shock is interrupted by a certain ship AI.
"I've received a preprogrammed message, would you like me to play it?"]]))
      vn.na(_([[Without anything better to do, you agree.]]))
      sai(_([["I have edited the recording to be more concise... and less coughy."]]))
      vn.na(_([[The recording starts playing.]]))

      local c = vne.flashbackTextStart(_("C"))
      c(_([[Seems like it is recording. Hello, again. I don't think I introduced me formally, my name is Claude. Sadly, I didn't get a chance to get your name nor what Autarch you serve under.

I wish... *sigh* ... I wish we would have met under different circumstances.

Let me be clear, the packet you have hopefully recovered. It is not for me. It is too late for me, but it is a parting gift for you.]]))
      c(_([[You may have wondered why I have not become Lost. That may have been true when we first met. Now, not so much. My body is failing, I can see my eyes become more purple every day. That is the last stage. I am not myself any more.

I do not want to succumb. Ben gave his everything, so I could survive until now. I did not want it to be in vain, I persisted, fought with all my strength. It was not enough, it is never enough.

When I saw you at first, I knew it was a sign. I could pass all that Ben gave me to you, and finally be free of this curse to join him.]]))
      c(_([[The package you should have, contains the blueprints for counteracting this disease. It can not cure, but it can prolong and delay. It was too late for us, but it is not so for you. Please analyze the documents, and do not become what we have become.

When you receive this message, there is no need to come look for me. I will no longer be among the living, but I will also not be among the Lost. I will be finally free... finally with...]]))
      vne.flashbackTextEnd()

      vn.scene()
      vn.newCharacter( sai )
      vn.transition()
      sai(_([["That was the entire audible message. I have taken the liberty of analyzing the data in the satchel you brought back. It seems to contain the blueprints for a trivial modification of the shield software that modulates the phase in hyper-spectral patterns. It seems like the modification counteracts some local effects in the so called 'wild space'. If you wish, I can apply them to your ships."]]))
      vn.na(_([[You agree to the modifications. Claude and Ben's deaths should not be in vain.]]))

      if not jump.get("Scholz's Star", "Haered"):known() then
         sai(_([["I have run an analysis of the so-called Wild Space. Based on the symmetric properties, it seems like there should be another exit. It may be worth pursuing the possibility."]]))
      end

      vn.run()

      -- TODO log
      local msg = _([[You recovered a package for Claude from {spb} ({sys} system), who, sacrificed himself before turning Lost. The package contained a blueprint to make your ship resistant to the Wild Space local effects making people Lost, allowing you to safely survive.]])
      if not jump.get("Scholz's Star", "Haered"):known() then
         msg = fmt.f(_([[{msg} Furthermore, your Ship AI indicated that it is likely that there is a second exit from Wild Space, which may warrant more exploration.]]), {msg=msg})
      end
      ws.log(fmt.f(msg, {spb=target, sys=targetsys}))

      misn.finish(true)
   end
end

local fnewlost
function enter ()
   if mem.state == 1 then
      -- Took damage
      player.pilot():setHealth( 60, 30 )
      player.landAllow( false, _("You can not land right now!") )
      hook.timer( 5, "company1" )
      hook.timer( 15, "company2" )
      hook.timer( 27, "company3" )
      hook.timer( 44, "company4" )
      hook.timer( 14, "extrachaos" )
      mem.state = 2
      fnewlost = faction.dynAdd( flost, "newlost" )
      fnewlost:dynEnemy( flost )
   end
end

local function addlost( ships )
   if system.cur() ~= targetsys then return end
   for k,s in ipairs(ships) do
      local p = pilot.add( s, flost, target )
      p:setHostile()
   end
end

function company1 ()
   addlost{
      "Proteron Dalton",
      "Tristan",
      "Llama",
   }
end

function company2 ()
   addlost{
      "Empire Admonisher",
      "Proteron Gauss",
      "Soromid Brigand",
      "Koala",
   }
end

function company3 ()
   addlost{
      "Dvaered Retribution",
      "Hyena",
   }
end

function company4 ()
   addlost{
      "Proteron Watson",
      "Pirate Starbridge",
   }
end

function extrachaos ()
   if system.cur() ~= targetsys then return end
   local plts = {}
   for k,p in ipairs(pilot.get( {flost, fnewlost} )) do
      if not p:mothership() then
         table.insert( plts, p )
      end
   end

   local p = plts[ rnd.rnd(1,#plts) ]
   if p then
      p:setLeader() -- Clear leader in case deployed
      p:taskClear()
      -- Flip faction
      if p:faction() == flost then
         p:setFaction( fnewlost )
      else
         p:setFaction( flost )
      end
   end
   hook.timer( 8 + 12*rnd.rnd(), "extrachaos" )
end

function abort ()
   var.pop("protera_husk_canland")
end
