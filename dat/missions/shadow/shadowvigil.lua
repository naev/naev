--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Shadow Vigil">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Shadowcomm">Triggers</done_evt>
  <campaign>Shadow</campaign>
 </notes>
</mission>
--]]
--[[
   This is the second mission in the "shadow" series.
--]]
local lmisn = require "lmisn"
require "proximity"
local fleet = require "fleet"
local fmt = require "format"
local shadow = require "common.shadow"
local pir = require "common.pirate"
local cinema = require "cinema"
local ai_setup = require "ai.core.setup"
local vn = require "vn"
local vntk = require "vntk"

-- Mission constants
local rebinasys = system.get("Pas")
local refuelspob, refuelsys = spob.getS("Semper") -- Qex
local misssys = {
   system.get("Shakar"),        -- Escort meeting point
   refuelsys,                   -- Refuel stop
   system.get("Eneguoz"),       -- Protegee meeting point
   system.get("Ogat"),          -- Final destination
}

local ambush, diplomat, dvaerplomat, escorts, seiryuu -- Non-persistent state
local accept_m -- Forward-declared functions

-- Make a pilot say a line, if he is alive. Mainly useful in sequential chat messages.
-- argument chat: A table containing:
-- pilot: The pilot to say the text
-- text: The text to be said
--
-- Example usage: hook.timer(2.0, "chatter", {pilot = p, text = "Hello, space!"})
function chatter(chat)
   if chat.pilot:exists() then
      chat.pilot:comm(chat.text)
   end
end

-- After having accepted the mission from the hailing Vendetta
function create()
   misn.accept()
   mem.stage = 0
   hook.jumpin("jumpin")
   hook.timer(1.0, "delayedClaim")

   misn.setDesc(fmt.f(_([[You are invited to a meeting in {sys}]]), {sys=rebinasys}))
   misn.setReward(_("???"))
   mem.marker = misn.markerAdd(rebinasys, "low")
   misn.osdCreate(_("Unknown"), {
      fmt.f(_("Fly to the {sys} system."), {sys=rebinasys}),
   })
end

-- Delayed claim to let time for the event's claim to disappear
function delayedClaim()
   if not misn.claim(misssys) then
      abort()
   end
end

-- Boarding the Seiryuu at the beginning of the mission
local function meeting()
   local accepted = false

   vn.clear()
   vn.scene()
   local rebina = vn.newCharacter( shadow.vn_rebina() )
   vn.transition("hexagon")

   local first = var.peek("shadowvigil_first") == nil -- nil acts as true in this case.
   if first then
      var.push("shadowvigil_first", false)

      vn.na(_([[You dock with the Seiryuu and shut down your engines. At the airlock, you are welcomed by two nondescript crewmen in gray uniforms who tell you to follow them into the ship. They lead you through corridors and passages that seem to lead to the bridge. On the way, you can't help but look around you in wonder. The ship isn't anything you're used to seeing. While some parts can be identified as such common features as doors and viewports, a lot of the equipment in the compartments and niches seems strange, almost alien to you. Clearly the Seiryuu is not just any other Kestrel.]]))
      vn.na(_([[On the bridge, you immediately spot - who else - the Seiryuu's captain, Rebina, seated in the captain's chair. The chair, too, is designed in the strange fashion that you've been seeing all over the ship. It sports several controls that you can't place, despite the fact that you're an experienced pilot yourself. The rest of the bridge is no different. All the regular stations and consoles seem to be there, but there are some others whose purpose you can only guess.]]))
      rebina(fmt.f(_([[Rebina swivels the chair around and smiles when she sees you. "Ah, {player}," she says. "How good of you to come. I was hoping you'd get my invitation, since I was quite pleased with your performance last time. And I'm not the only one. As it turns out Jorek seems to have taken a liking to you as well. He may seem rough, but he's a good man at heart."]]),
         {player=player.name()}))
      rebina(_([[You choose not to say anything, but Rebina seems to have no trouble reading what's on your mind. "Ah yes, the ship. It's understandable that you're surprised at how it looks. I can't divulge too much about this technology or how we came to possess it, but suffice to say that we don't buy from the regular outlets. We have need for... an edge in our line of business."]]))
      vn.na(_([[Grateful for the opening, you ask Rebina what exactly this line of business is. Rebina flashes you a quick smile and settles into the chair for the explanation.]]))
      rebina(_([["The organization I'm part of is known as the Four Winds, or rather," she gestures dismissively, "not known as the Four Winds. We keep a low profile. You won't have heard of us before, I'm sure. At this point I should add that many who do know us refer to us as the 'Shadows', but this is purely a colloquial name. It doesn't cover what we do, certainly. In any event, you can think of us as a private operation with highly specific objectives. At this point that is all I can tell you." She leans forward and fixes you with a level stare. "Speaking of specific objectives, I have one such objective for you."]]))
   else
      rebina(fmt.f(_([[Again, you set foot on the Seiryuu's decks, and again you find yourself surrounded by the unfamiliar technology on board. The ship's crewmen guide you to the bridge, where Rebina is waiting for you. She says, "Welcome back, {player}. I hope you've come to reconsider my offer. Let me explain to you again what it is we need from you."]]),
         {player=player.name()}))
   end

   rebina(_([["You may not know this, but there are tensions between the Imperial and Dvaered militaries. For some time now there have been incidents on the border, conflicts about customs, pilots disrespecting each other's flight trajectories, that sort of thing. It hasn't become a public affair yet, and the respective authorities don't want it to come to that. This is why they've arranged a secret diplomatic meeting to smooth things over and make arrangements to de-escalate the situation."]]))
   rebina(_([["This is where we come in. Without going into the details, suffice to say, we have an interest in making sure that this meeting does not meet with any unfortunate accidents. However, for reasons I can't explain to you now, we can't become involved directly. That's why I want you to go on our behalf."]]))
   rebina(_([["You will essentially be flying an escort mission. You will rendezvous with a small wing of private fighters, who will take you to your charge, the Imperial representative. Once there, you will protect him from any threats you might encounter and see him safely to Dvaered space. As soon as the Imperial representative has joined his Dvaered colleague, your mission will be complete and you will report back here."]]))
   rebina(_([["That will be all. I offer you a suitable monetary reward should you choose to accept. Can I count on you to undertake this task?"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("accept")
   vn.func( function () accepted = true end )
   rebina(fmt.f(_([["Excellent, {player}." Rebina smiles at you. "I've told my crew to provide your ship's computer with the necessary navigation data. Also, note that I've taken the liberty of installing a specialized IFF transponder onto your ship. Don't pay it any heed, it will only serve to identify you as one of the escorts. For various reasons, it is best that you refrain from communication with the other escorts as much as possible. I think you might have an inkling as to why."]]),
      {player=player.name()}))
   rebina(fmt.f(_([[Rebina straightens up. "That will be all for now, {player}," she says in a more formal, captain-like manner. "You have your assignment; I suggest you go about it."]]),
      {player=player.name()}))
   vn.na(_([[You are politely but efficiently escorted off the Seiryuu's bridge. Soon you settle back in your own cockpit chair, ready to do what was asked of you.]]))
   vn.done("hexagon")

   vn.label("decline")
   rebina(_([[Captain Rebina sighs. "I see. I don't mind admitting that I hoped you would accept, but it's your decision. I won't force you to do anything you feel uncomfortable with. However, I still hold out the hope that you will change your mind. If you do, come back to see me. You know where to find the Seiryuu."]]))
   vn.na(_([[Mere hectoseconds later you find yourself back in your cockpit, and the Seiryuu is leaving. It doesn't really come as a surprise that you can't find any reference to your rendezvous with the Seiryuu in your flight logs...]]))
   vn.done("hexagon")

   vn.run()

   if accepted then
      accept_m()
   else
      abort()
   end

   player.unboard()
end

function accept_m()
   mem.alive = {true, true, true} -- Keep track of the escorts. Update this when they die.
   mem.stage = 1 -- Keeps track of the mission stage
   mem.nextsys = lmisn.getNextSystem(system.cur(), misssys[mem.stage]) -- This variable holds the system the player is supposed to jump to NEXT.
   mem.seirsys = system.cur() -- Remember where the Seiryuu is.
   mem.origin = system.cur() -- The place where the AI ships spawn from.
   mem.chattered = false
   mem.kills = 0 -- Counter to keep track of enemy kills.
   seiryuu:setHilight(false) -- No need to highlight anymore

   mem.accepted = false
   mem.missend = false
   mem.landfail = false

   shadow.addLog( _([[Captain Rebina has revealed some information about the organization she works for. "The organization I'm part of is known as the Four Winds, or rather, not known as the Four Winds. We keep a low profile. You won't have heard of us before, I'm sure. At this point I should add that many who do know us refer to us as the 'Shadows', but this is purely a colloquial name. It doesn't cover what we do, certainly. In any event, you can think of us as a private operation with highly specific objectives. At this point that is all I can tell you."]]) )

   misn.setDesc(_([[Captain Rebina, of the Four Winds, has asked you to help Four Winds agents protect an Imperial diplomat.]]))
   misn.setReward(_("A sum of money."))
   mem.marker = misn.markerAdd(misssys[1], "low")

   misn.osdCreate(_("Shadow Vigil"), {
     fmt.f(_("Fly to the {sys} system and join the other escorts"), {sys=misssys[1]}),
     fmt.f(_("Follow the group to {spb} and land"),{spb=refuelsys}),
     _("Follow the flight leader to the rendezvous location"),
     _("Escort the Imperial diplomat"),
     fmt.f(_("Report back to Rebina ({sys} system)"),{sys=rebinasys}),
   })

   hook.land("land")
   hook.enter("enter")
   hook.jumpout("jumpout")
   hook.takeoff("takeoff")
end

-- Function hooked to jumpout. Used to retain information about the previously visited system.
function jumpout()
   if mem.stage == 4 and not mem.dpjump then
      vntk.msg(_("You have left your charge behind!"), _([[You have jumped before the diplomat you were supposed to be protecting did. By doing so you have abandoned your duties and failed your mission.]]))
      shadow.addLog( _([[You failed to escort a diplomat to safety for the Four Winds.]]) )
      abort()
   end
   mem.origin = system.cur()
   mem.nextsys = lmisn.getNextSystem(system.cur(), misssys[mem.stage])
end

-- Function hooked to landing. Only used to prevent a fringe case.
function land()
   if mem.landfail then
      vntk.msg(_("You abandoned your charge!"), _("You have landed, but you were supposed to escort the diplomat. Your mission is a failure!"))
   elseif spob.cur()==refuelspob and mem.stage == 2 then
      local dist = system.cur():jumpDist(misssys[4])
      local refueltext = n_(
         "While you handle the post-land and refuel operations, you get a comm from the flight leader, audio only. He tells you that this will be the last place where you can refuel, and that you need to make sure to have at least %d jump worth of fuel on board for the next leg of the journey. You will be left behind if you can't keep up.",
         "While you handle the post-land and refuel operations, you get a comm from the flight leader, audio only. He tells you that this will be the last place where you can refuel, and that you need to make sure to have at least %d jumps worth of fuel on board for the next leg of the journey. You will be left behind if you can't keep up.", dist )
      vntk.msg(_("Preparing for the job"), refueltext:format(dist))
      mem.stage = 3 -- Fly to the diplomat rendezvous point.
      misn.osdActive(3)
      mem.landfail = true
      mem.origin = spob.cur()
   end
end

-- Function hooked to takeoff.
function takeoff()
   if mem.stage == 3 and mem.landfail then -- We're taking off from refuelspob. Can't be anything else.
      jumpin()
   end
end

-- Function hooked to jumpin AND takeoff. Handles events that should occur in either case.
function enter()
   if system.cur() == misssys[1] and mem.stage == 1 and mem.missend == false then
      -- case enter system where escorts wait
      escorts = fleet.add( 3, "Lancelot", shadow.fct_fourwinds(), vec2.new(0, 0), _("Four Winds Escort"), {ai="baddie_norun"} )
      for i, j in ipairs(escorts) do
         if not mem.alive[i] then j:rm() end -- Dead escorts stay dead.
         if j:exists() then
            j:control()
            j:setInvincPlayer()
            hook.pilot(j, "death", "escortDeath")
         end
      end
      local rend_point = vec2.new(0,0)
      mem.start_marker = system.markerAdd( rend_point, _("Rendezvous point") )
      mem.proxy = hook.timer(0.5, "proximity", {location = rend_point, radius = 500, funcname = "escortStart"})
   end
end

-- Function hooked to jumpin. Handles most of the events in the various systems.
function jumpin()
   mem.sysclear = false -- We've just jumped in, so the ambushers, if any, are not dead.

   if mem.pahook then -- Remove the hook on the player being attacked, if needed
      hook.rm( mem.pahook )
   end

   if mem.stage == 0 and system.cur() == rebinasys then -- put Rebina's ship
      seiryuu = pilot.add( "Pirate Kestrel", shadow.fct_fourwinds(), vec2.new(0, -2000), _("Seiryuu"), {ai="trader"} )
      seiryuu:control(true)
      seiryuu:setActiveBoard(true)
      seiryuu:setInvincible(true)
      seiryuu:setHilight(true)
      seiryuu:setVisplayer(true)
      hook.pilot(seiryuu, "board", "board")
   end

   if mem.stage >= 3 and system.cur() ~= mem.nextsys then -- case player is escorting AND jumped to somewhere other than the next escort destination
      vntk.msg(_("You diverged!"), _([[You have jumped to the wrong system! You are no longer part of the mission to escort the diplomat.]]))
      shadow.addLog( _([[You failed to escort a diplomat to safety for the Four Winds.]]) )
      abort()
   end

   if mem.stage == 4 then
      -- Spawn the diplomat.
      diplomat = pilot.add( "Gawain", shadow.fct_diplomatic(), mem.origin, _("Imperial Diplomat") )
      hook.pilot(diplomat, "death", "diplomatDeath")
      hook.pilot(diplomat, "jump", "diplomatJump")
      hook.pilot(diplomat, "attacked", "diplomatAttacked")
      diplomat:setSpeedLimit( 130 )
      diplomat:intrinsicSet( "armour", 300 )
      diplomat:intrinsicSet( "shield", 300 )
      diplomat:control()
      diplomat:setInvincPlayer()
      diplomat:setHilight(true)
      diplomat:setVisplayer()
      diplomat:setVisible() -- Hack to make ambushes more reliable.
      mem.dpjump = false
      misn.markerRm(mem.marker) -- No mem.marker. Player has to follow the NPCs.
   end
   if mem.stage >= 2 then
      for k,f in ipairs(pir.factions) do
         pilot.toggleSpawn(f)
         pilot.clearSelect(f) -- Not sure if we need a claim for this.
      end

      -- Spawn the escorts.
      escorts = fleet.add( 3, "Lancelot", shadow.fct_fourwinds(), mem.origin, _("Four Winds Escort"), {ai="baddie_norun"} )
      for i, j in ipairs(escorts) do
         if not mem.alive[i] then j:rm() end -- Dead escorts stay dead.
         if j:exists() then
            j:control()
            j:setHilight(true)
            j:setInvincPlayer()
            hook.pilot(j, "death", "escortDeath")
            mem.controlled = true
            j:memory().angle = (i-2)*math.pi/2
         end
      end

      -- Ships spawned, now decide what to do with them.
      if system.cur() == misssys[2] and mem.stage == 2 then -- case land on Nova Shakar
         for i, j in ipairs(escorts) do
            if not mem.alive[i] then j:rm() end -- Dead escorts stay dead.
            if j:exists() then
               j:land(refuelspob)
            end
         end
      elseif system.cur() == misssys[3] then -- case join up with diplomat
         diplomat = pilot.add( "Gawain", shadow.fct_diplomatic(), vec2.new(0, 0), _("Imperial Diplomat") )
         hook.pilot(diplomat, "death", "diplomatDeath")
         hook.pilot(diplomat, "jump", "diplomatJump")
         diplomat:setSpeedLimit( 130 )
         diplomat:intrinsicSet( "armour", 300 )
         diplomat:intrinsicSet( "shield", 300 )
         diplomat:control()
         diplomat:setInvincPlayer()
         diplomat:setHilight(true)
         diplomat:setVisplayer()
         diplomat:setVisible() -- Hack to make ambushes more reliable.
         mem.proxy = hook.timer(0.5, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "escortNext"})
         for i, j in ipairs(escorts) do
            if j:exists() then
               j:follow(diplomat,true) -- Follow the diplomat.
            end
         end
         hook.timer(5.0, "chatter", {pilot = escorts[1], text = _("Alright folks, there he is. You know your orders. Stick to him. Don't let anyone touch him on the way to the rendezvous.")})
         hook.timer(12.0, "chatter", {pilot = escorts[2], text = _("Two, copy.")})
         hook.timer(14.0, "chatter", {pilot = escorts[3], text = _("Three, copy.")})
     elseif system.cur() == misssys[4] then -- case rendezvous with Dvaered diplomat
         for i, j in ipairs(escorts) do
            if j:exists() then
               j:follow(diplomat,true) -- Follow the diplomat.
            end
         end
         dvaerplomat = pilot.add( "Dvaered Vigilance", "Dvaered", vec2.new(2000, 4000) )
         dvaerplomat:control()
         dvaerplomat:setHilight(true)
         dvaerplomat:setVisplayer()
         dvaerplomat:setDir(math.pi)
         dvaerplomat:setFaction( shadow.fct_diplomatic() )
         diplomat:setInvincible(true)
         diplomat:moveto(vec2.new(1850, 4000), true)
         mem.diplomatidle = hook.pilot(diplomat, "idle", "diplomatIdle")
      else -- case en route, handle escorts flying to the next system, possibly combat
         for i, j in ipairs(escorts) do
            if j:exists() then
               if mem.stage == 4 then
                  diplomat:hyperspace(lmisn.getNextSystem(system.cur(), misssys[mem.stage])) -- Hyperspace toward the next destination system.
                  j:follow(diplomat,true) -- Follow the diplomat.
               else
                  j:hyperspace(lmisn.getNextSystem(system.cur(), misssys[mem.stage])) -- Hyperspace toward the next destination system.
               end
            end
         end
         if not mem.chattered then
            hook.timer(10.0, "chatter", {pilot = escorts[2], text = _("So do you guys think we'll run into any trouble?")})
            hook.timer(20.0, "chatter", {pilot = escorts[3], text = _("Not if we all follow the plan. I didn't hear of any trouble coming our way from any of the others.")})
            hook.timer(30.0, "chatter", {pilot = escorts[2], text = _("I just hope Z. knows what he's doing.")})
            hook.timer(35.0, "chatter", {pilot = escorts[1], text = _("Cut the chatter, two, three. This is a low-profile operation. Act the part, please.")})
            mem.chattered = true
         end
         mem.jp2go = system.cur():jumpDist(misssys[4])
         if mem.jp2go <= 2 and mem.jp2go > 0 then -- Encounter
            local ambush_ships = {
               {"Pirate Ancestor", "Pirate Hyena", "Pirate Hyena"},
               {"Pirate Ancestor", "Pirate Vendetta", "Pirate Hyena", "Pirate Hyena"}
            }
            ambush = fleet.add( 1,  ambush_ships[3-mem.jp2go], shadow.fct_pirates(), vec2.new(0, 0), _("Pirate Attacker"), {ai="baddie_norun"} )
            mem.kills = 0
            for i, j in ipairs(ambush) do
               if j:exists() then
                  j:setHilight(true)
                  j:setHostile(true)
                  hook.pilot(j, "death", "attackerDeath")

                  -- Just in case.
                  hook.pilot(j, "jump", "attackerDeath")
                  hook.pilot(j, "land", "attackerDeath")
               end
            end
            for i, j in ipairs(escorts) do
               if j:exists() then
                  hook.pilot(j, "attacked", "diplomatAttacked")
               end
            end
            mem.pahook = hook.pilot(player.pilot(), "attacked", "diplomatAttacked")
         end
      end

   elseif system.cur()==mem.seirsys then -- not escorting.
      -- case enter system where Seiryuu is
      seiryuu = pilot.add( "Pirate Kestrel", shadow.fct_fourwinds(), vec2.new(0, -2000), _("Seiryuu"), {ai="trader"} )
      seiryuu:setInvincible(true)
      if mem.missend then
         seiryuu:setActiveBoard(true)
         seiryuu:setHilight(true)
         seiryuu:setVisplayer(true)
         seiryuu:control()
         hook.pilot(seiryuu, "board", "board")
      end
   else
      if mem.proxy then
         hook.rm( mem.proxy )
      end
   end
end

-- The player has successfully joined up with the escort fleet. Cutscene -> departure.
function escortStart()
   if mem.start_marker ~= nil then
      system.markerRm( mem.start_marker )
   end
   mem.stage = 2 -- Fly to the refuel planet.
   misn.osdActive(2)
   misn.markerRm(mem.marker) -- No marker. Player has to follow the NPCs.
   escorts[1]:comm(fmt.f(_("There you are at last. Fancy boat you've got there. We're gonna head to {spb} first, to grab some fuel. Just stick with us, okay?"),{spb=refuelspob}))
   local nextjump = lmisn.getNextSystem( system.cur(), misssys[mem.stage] )
   for i, j in pairs(escorts) do
      if j:exists() then
         j:setHilight(true)
         j:hyperspace( nextjump ) -- Hyperspace toward the next destination system.
      end
   end
end

-- The player has successfully rendezvoused with the diplomat. Now the real work begins.
function escortNext()
   mem.stage = 4 -- The actual escort begins here.
   misn.osdActive(4)
   diplomat:hyperspace(lmisn.getNextSystem(system.cur(), misssys[mem.stage])) -- Hyperspace toward the next destination system.
   mem.dpjump = false
end

-- Handle the death of the scripted attackers. Once they're dead, recall the escorts.
function attackerDeath()
   mem.kills = mem.kills + 1

   if mem.kills < #ambush then return end

   local myj
   for i, j in ipairs(escorts) do
      if j:exists() then
         myj = j
         j:changeAI("baddie_norun")
         j:memory().angle = (i-2)*math.pi/2
         j:control()
         j:follow(diplomat,true)
         diplomat:hyperspace(lmisn.getNextSystem(system.cur(), misssys[mem.stage])) -- Hyperspace toward the next destination system.
         mem.controlled = true
      end
   end

   myj:comm(_("All hostiles eliminated, resume standing orders."))
   mem.sysclear = true -- safety flag to prevent the escorts from being released twice.
end

-- Puts the escorts under AI control again, and makes them fight.
local function escortFree()
   for i, j in ipairs(escorts) do
      if j:exists() then
         j:control(false)
         j:changeAI("baddie_norun")
      end
   end
end

-- Handle the death of the escorts. Abort the mission if all escorts die.
function escortDeath()
   if mem.diplomatkilled then return end

   if mem.alive[3] then mem.alive[3] = false
   elseif mem.alive[2] then mem.alive[2] = false
   else -- all escorts dead
      vntk.msg(_("The escorts are dead!"), _([[All of the escorts have been destroyed. With the flight leader out of the picture, the diplomat has decided to call off the mission.]]))
      shadow.addLog( _([[You failed to escort a diplomat to safety for the Four Winds.]]) )
      abort()
   end
end

-- Handle the death of the diplomat. Abort the mission if the diplomat dies.
function diplomatDeath()
   vntk.msg(_("The diplomat is dead!"), _([[The diplomat you were supposed to be protecting has perished! Your mission has failed.]]))
   for i, j in ipairs(escorts) do
      if j:exists() then
         j:control(false)
      end
   end
   shadow.addLog( _([[You failed to escort a diplomat to safety for the Four Winds.]]) )
   abort()
end

-- Handle the departure of the diplomat. Escorts will follow.
function diplomatJump()
   mem.dpjump = true
   misn.markerRm(mem.marker)
   mem.marker = misn.markerAdd(lmisn.getNextSystem(system.cur(), misssys[mem.stage]), "low")
   for i, j in ipairs(escorts) do
      if j:exists() then
         j:control(true)
         j:taskClear()
         j:hyperspace(lmisn.getNextSystem(system.cur(), misssys[mem.stage])) -- Hyperspace toward the next destination system.
      end
   end
   player.msg(fmt.f(_("Mission update: The diplomat has jumped to {sys}."), {sys=lmisn.getNextSystem(system.cur(), misssys[mem.stage])}))
end

-- Handle the diplomat getting attacked.
function diplomatAttacked()
   player.autonavReset(5)
   if mem.controlled and not mem.sysclear then
      chatter({pilot = escorts[1], text = _("Those rats are eyeballing us - take them out!")})
      escortFree()
      diplomat:taskClear()
      diplomat:brake()
      mem.controlled = false
   end
   if mem.shuttingup == true then
      return
   else
      mem.shuttingup = true
      diplomat:comm(_("Diplomatic vessel under fire!"))
      hook.timer(10.0, "diplomatShutup") -- Shuts him up for at least 10s.
   end
end

-- As soon as the diplomat is at its destination, set up final cutscene.
function diplomatIdle()
   local mypos = {} -- Relative positions to the Dvaered diplomat.
   mypos[1] = vec2.new(0, 130)
   mypos[2] = vec2.new(-85, -65)
   mypos[3] = vec2.new(85, -65)

   for i, j in ipairs(escorts) do
      if j:exists() then
         j:setInvincible(true)
         j:taskClear()
         j:moveto(dvaerplomat:pos() + mypos[i], true)
         j:face(dvaerplomat:pos())
      end
   end

   mem.proxy = hook.timer(0.1, "proximity", {location = diplomat:pos(), radius = 400, funcname = "diplomatCutscene"})
end

-- This is the final cutscene.
function diplomatCutscene()
   cinema.on()

   camera.set(dvaerplomat, false, 500)

   hook.timer(1.0, "chatter", {pilot = diplomat, text = _("This is Empire zero-zero-four. Transmitting clearance code now.")})
   hook.timer(10.0, "chatter", {pilot = dvaerplomat, text = _("Empire zero-zero-four, your code checks out. Commence boarding maneuvers.")})
   hook.timer(17.0, "diplomatGo")
   hook.timer(21.0, "chatter", {pilot = escorts[1], text = _("This is your leader, you're all clear. Execute, execute, execute!")})
   hook.timer(21.5, "killDiplomats")

end

function diplomatShutup()
   mem.shuttingup = false
end

function diplomatGo()
   diplomat:moveto(dvaerplomat:pos(), true)
   hook.rm(mem.diplomatidle)
end

function killDiplomats()
   for i, j in ipairs(escorts) do
      if j:exists() then
         j:taskClear()
         j:outfitRm("all")
         j:outfitAdd("Cheater's Ragnarok Beam", 1)
         ai_setup.setup(j)
         j:attack(dvaerplomat)
         j:setHilight(false)
      end
   end
   diplomat:hookClear()
   hook.timer(0.5, "diplomatKilled")
   hook.timer(5.0, "escortFlee")
   mem.landfail = false
end

function diplomatKilled()
   mem.diplomatkilled = true
   diplomat:setHealth(0, 0)
   dvaerplomat:setHealth(0, 0)
end

function escortFlee()
   cinema.off()
   camera.set()

   for i, j in ipairs(escorts) do
      if j:exists() then
         j:taskClear()
         j:outfitRm("all")
         j:hyperspace()
         j:setInvincible(false)
         j:setInvincPlayer(false)
         hook.pilot(j, "board", "board_escort")
      end
   end
   mem.boarded_escort = false

   misn.osdActive(5)
   mem.marker = misn.markerAdd(mem.seirsys, "low")
   mem.stage = 1 -- no longer spawn things
   mem.missend = true
end

-- Function hooked to boarding. Only used on the Seiryuu.
function board()
   if mem.stage == 0 then
      misn.markerRm(mem.marker)
      misn.osdDestroy()
      meeting()
   else
      player.unboard()
      seiryuu:control()
      seiryuu:hyperspace()
      seiryuu:setActiveBoard(false)
      seiryuu:setHilight(false)

      vn.clear()
      vn.scene()
      local rebina = vn.newCharacter( shadow.vn_rebina() )
      vn.transition("hexagon")

      rebina(_([[Captain Rebina angrily drums her fingers on her captain's chair as she watches the reconstruction made from your sensor logs. Her eyes narrow when both diplomatic ships explode under the onslaught of weapons the escorts should not have had onboard.]]))
      rebina(fmt.f(_([["This is bad, {player}," she says when the replay shuts down. "Worse than I had even thought possible. The death of the Imperial and Dvaered diplomats is going to spark a political incident, with each faction accusing the other of treachery." She stands up and begins pacing up and down the Seiryuu's bridge. "But that's not the worst of it. You saw what happened. The diplomats were killed by their own escorts - by Four Winds operatives! This is an outrage!"]]),
         {player=player.name()}))
      rebina(fmt.f(_([[Captain Rebina brings herself back under control through an effort of will. "{player}, this does not bode well. We have a problem, and I fear I'm going to need your help again before the end. But not yet. I have a lot to do. I have to get to the bottom of this, and I have to try to keep this situation from escalating into a disaster. I will contact you again when I know more. In the mean time, you will have the time to spend your reward - it's already in your account."]]),
         {player=player.name()}))
      vn.na(_([[Following this, you are swiftly escorted off the Seiryuu. Back in your cockpit, you can't help feeling a little anxious about these Four Winds. Who are they, what do they want, and what is your role in all of it? Time will have to tell.]]))
      vn.disappear( rebina, "hexagon" )
      vn.func( function ()
         player.pay( shadow.rewards.shadowvigil )
      end )
      vn.sfxVictory()
      vn.na( fmt.reward( shadow.rewards.shadowvigil ) )
      vn.run()

      shadow.addLog( _([[Your attempt to escort a diplomat for the Four Winds was thwarted by traitors on the inside. Other Four Winds escorts opened fire on the diplomat, killing him. Captain Rebina has said that she may need your help again at a later date.]]) )
      misn.finish(true)
   end
end

-- The player boards one of the escort at the end of the mission.
function board_escort( pilot )
   if not mem.boarded_escort then
      local rwd = outfit.get("Vacuum Cleaner?")

      vn.clear()
      vn.scene()
      vn.transition()
      -- TODO give the guy an actual image or something
      vn.na(_([[After managing to bypass the ship's security system, you enter the cockpit and notice that the fighter has been sabotaged and might soon explode: you have little time to find and capture the pilot. You head to the living cabin. As you cross the access hatch, you see him, floating weightlessly close to the floor. As he turns his livid face towards yours, you find yourself horrified by the awful mask of pain on his face. His bulging, bloodshot eyes stare at you, as if they wanted to pierce your skin.]]))
      vn.na(_([[His twisted mouth splits a repugnant creamy fluid as he starts to speak, in a groan:
"Good job out there. Hum. But, you see? You won't catch me alive. Oh, no. The pill of silence will make sure of that. Hmf. It looks painful, doesn't it? You know what? It's even worse than it looks. I guarantee it. Gbf. Oh, it makes me throw up... I'm sorry about that."]]))
      vn.na(_([[You approach the man, and ask him why he killed the diplomats. "You have no idea what is going on, right? Uh. Let me tell you something: they are coming! And they won't show any mercy. Oooh! They're so close! They're far worse than your darkest nightmares! And one day they'll get you. That day, you will envy... You will envy my vomit-eating agony! Sooooo much!"]]))
      vn.na(_([[The man stops speaking and moving. His grimacing face still turned towards you makes you wonder if he's fully dead, but in absence of any response, you first think about leaving the ship as soon as possible before it explodes. You then decide instead to take the opportunity to loot around a bit, and finally go back to your ship with what looks like a fancy vacuum cleaner.]]))
      vn.func( function ()
         player.outfitAdd( rwd )
      end )
      vn.sfxBingo()
      vn.na(fmt.reward(rwd))
      vn.run()
      player.unboard()
      pilot:setHealth(-1, -1) -- Make ship explode
      mem.boarded_escort = true
   end
end

-- Handle the unsuccessful end of the mission.
function abort()
   misn.finish(false)
end
