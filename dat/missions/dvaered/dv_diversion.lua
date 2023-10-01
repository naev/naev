--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Small Diversion">
 <unique />
 <priority>4</priority>
 <cond>
   if faction.playerStanding("Dvaered") &lt; 5 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <chance>10</chance>
 <location>Bar</location>
 <spob>Doranthex Prime</spob>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   This is a one-off mission where you help a new Dvaered Warlord takeover a planet
   To Do: fix fighters being idle after mission ends
   Other editors, feel free to update dialog to make it more dvaered like.
--]]

local fleet = require "fleet"
local fmt = require "format"
local ai_setup = require "ai.core.setup"
local vn = require "vn"
local vni = require "vnimage"
local vntk = require "vntk"
local love_shaders = require "love_shaders"

local destsys = system.get("Torg")
local destplanet = spob.get("Jorcan")
local destjump = system.get("Doranthex")

local broadcast_first, cleanup, update_fleet -- our local functions
local attkfaction, fleetdv, fleethooks, hawk, hawkfaction, jump_fleet -- Non-persistent state

local chatter = {}
chatter[5] = _("Khan is dead! Who will be our warlord now?")
chatter[6] = _("Obviously the one who killed him!")
chatter[7] = _("I will never serve a different warlord than Khan! Die, you traitors!")

local npc_name = _("Dvaered Liaison")
local npc_image, npc_portrait

local reward = 800e3

function create()
   local missys = {destsys}
   if not misn.claim(missys) then
      abort()
   end

   npc_image, npc_portrait = vni.dvaeredMilitaryMale()

   misn.setNPC( npc_name, npc_portrait, _("A high ranking Dvaered officer. It looks like he might have a job offer for you."))
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local dv = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   dv(_([[You walk up to the Dvaered official at his table. He mentions that he is looking for a pilot like yourself.
"I am looking for a skilled pilot to do a simple job for me, interested?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   dv(_([["I see. In that case, I'm going to have to ask you to leave. My job is to recruit a civilian, but you're clearly not the pilot I'm looking for. You may excuse yourself, citizen."]]))
   vn.done()

   vn.label("accept")
   dv(_([["My General has just retired from the High Command and is now looking to become the Warlord of a planetary system. Unfortunately, our loyal forces seem insufficient to take on any existing planetary defence forces head on."]]))
   dv(fmt.f(_([["However, it looks like there may be an opportunity for us in {sys}. Warlord Khan of {pnt} has been building his newest flagship, the Hawk, and will be onboard the Hawk as it tests its hyperspace capabilities. Since its engines and weapons have not been fully installed yet, it will be substantially slower than normal and unable to defend itself."]]),
      {sys=destsys, pnt=destplanet}))
   dv(_([["To protect himself and the Hawk, Khan will have deployed a substantial fleet of escort fighters fleet to defend against any surprise attacks."]]))
   dv(fmt.f(_([["That is where you come in. You will jump into {sys} and find the Hawk and its escorts. Before the Hawk is able to reach hyperspace, you will fire on it, and cause the fighters to engage with you. At this point, you should run away from the Hawk and the jump point, so that the fighters will give chase. Then we will jump into the system and destroy the Hawk before the fighters can return."]]),
      {sys=destsys}))
   dv(fmt.f(_([["We will jump in approximately 80 hectoseconds after you jump into {sys}, so the fighters must be far enough away by then not to come back and attack us."]]),
      {sys=destsys}))

   vn.run()

   if not accepted then return end

   misn.accept()
   misn.osdCreate(_("A Small Distraction"), {
      fmt.f(_("Fly to the {sys} system"), {sys=destsys}),
      _("Fire on the Hawk and flee from the fighter escorts until the Dvaered fleet jumps in and destroys the Hawk"),
   })
   misn.setDesc(_("You have been recruited to distract the Dvaered fighter escorts and lead them away from the jump gate and the capital ship, Hawk. The Dvaered task force will jump in and attempt to destroy the Hawk before the escort ships can return. The mission will fail if the Hawk survives or the Dvaered task force is eliminated."))
   misn.setTitle(_("A Small Distraction"))
   mem.marker = misn.markerAdd( destsys, "low" )

   mem.missionstarted = false
   mem.jump_fleet_entered = false

   hook.jumpout("jumpout")
   hook.enter("enter")
   hook.land("land")
end

function jumpout()
   mem.last_sys = system.cur()
end

local function player_left_mission_theater()
   if mem.missionstarted then -- The player has landed, which instantly ends the mission.
      vntk.msg(_("You ran away!"), _("You have left the system without first completing your mission. The operation ended in failure."))
      faction.get("Dvaered"):modPlayerSingle(-5)
      abort()
   end
end

function enter()
   if system.cur() == destsys then
      -- Create the custom factions
      hawkfaction = faction.dynAdd( "Dummy", "The Hawk", _("The Hawk"), {ai="dvaered_norun"} )
      attkfaction = faction.dynAdd( "Dummy", "Attackers", _("Attackers"), {ai="dvaered_norun"} )
      faction.dynEnemy( hawkfaction, attkfaction )

      -- Spawn people
      pilot.toggleSpawn(false)
      pilot.clear()
      misn.osdActive(2)
      mem.missionstarted = true
      local j = jump.get(destsys, destjump)
      local v = j:pos()
      hawk = pilot.add( "Dvaered Goddard", hawkfaction, v-vec2.new(1500,8000), _("Hawk"), {ai="dvaered_norun"} )
      hawk:outfitRm("all")
      hawk:outfitRm("cores")
      hawk:outfitAdd("Unicorp PT-2200 Core System")
      hawk:outfitAdd("Unicorp Eagle 6500 Engine")
      hawk:outfitAdd("TeraCom Mace Launcher", 3) -- Half finished installing weapons. :)
      ai_setup.setup(hawk)
      hawk:setHilight(true)
      hawk:setVisible(true)
      hawk:cargoAdd("Food", 500)
      hawk:control()
      hawk:hyperspace(destjump)
      hawk:broadcast(fmt.f(_("Alright folks, this will be Hawk's maiden jump. Continue on course to the {sys} jump gate."), {sys=destjump}))
      fleethooks = {}
      fleetdv = fleet.add( 14, "Dvaered Vendetta", hawkfaction, hawk:pos()-vec2.new(1000,1500), nil, {ai="dvaered_norun"} )
      for i, bi in ipairs(fleetdv) do
         bi:setHilight(true)
         bi:setVisible(true)
         bi:control()
         bi:moveto(v)
         table.insert(fleethooks, hook.pilot(bi, "attacked", "fleetdv_attacked"))
      end

      hook.pilot( hawk, "jump", "hawk_jump" )
      hook.pilot( hawk, "land", "hawk_land" )
      hook.pilot( hawk, "attacked", "hawk_attacked")
      hook.pilot( hawk, "death", "hawk_dead" )
      hook.timer(80.0, "spawn_fleet")
   else
      player_left_mission_theater()
   end
end

function land()
   player_left_mission_theater()
end

function hawk_jump () -- Got away
   tk.msg(_("The Hawk got away!"), _("The Hawk jumped out of the system. You have failed your mission."))
   faction.get("Dvaered"):modPlayerSingle(-5)
   hook.timer(10, "abort")
end

function hawk_land(_plt, pnt) -- Got away
   tk.msg(_("The Hawk got away!"), fmt.f(_("The Hawk landed back on {pnt}. You have failed your mission."), {pnt=pnt}))
   faction.get("Dvaered"):modPlayerSingle(-5)
   hook.timer(10, "abort")
end

function hawk_attacked () -- chased
   if not mem.jump_fleet_entered then
      hawk:broadcast(_("How dare they attack me! Get them!"))
      hawk:control()
      hawk:hyperspace(destjump)
      broadcast_first(fleetdv, _("You heard Warlord Khan, blow them to pieces!"))
   end

   update_fleet()
end

function fleetdv_attacked () -- chased
   if not mem.jump_fleet_entered then
      hawk:control()
      hawk:hyperspace(destjump)
      broadcast_first(fleetdv, _("They're attacking us, blow them to pieces!"))
   end

   update_fleet()
end

function broadcast_first(flt, msg) -- Find the first alive ship and broadcast a message
   for k, v in ipairs(flt) do
      if v:exists() then
         local _armour, _shield, disabled = v:health()
         if not disabled then
            v:broadcast(msg)
            break
         end
      end
   end
end

function hawk_dead () -- mission accomplished
   hawk:broadcast(_("Arrgh!"))

   faction.dynEnemy( hawkfaction, attkfaction, true )
   faction.dynEnemy( attkfaction, hawkfaction, true )
   local messages = {5, 6, 7}
   for k, v in ipairs(fleetdv) do
      if v:exists() then
         local _armour, _shield, disabled = v:health()
         if not disabled then
            local msg = table.remove(messages, 1)
            if msg then
               v:broadcast(chatter[msg])
            end

            v:control(false)
            v:setVisible(false)
            v:setHilight(false)
         end
      end
   end

   jump_fleet[6]:setNoDeath()

   hook.timer(10, "complete")
   for i, j in ipairs(jump_fleet) do
      if j:exists() then
         j:land(destplanet)
      end
   end
end

function update_fleet() -- Wrangles the fleet defending the Hawk
   -- Nothing to do if we're already in the final phase of the mission
   if not fleethooks then
      return
   end

   for i, j in ipairs(fleetdv) do
      if j:exists() then
         j:control()
         if mem.jump_fleet_entered then
            j:changeAI("dvaered_norun")
            j:control(false)
         else
            j:taskClear()
            j:attack(player.pilot())
         end
      end
   end

   if mem.jump_fleet_entered then
      for k, v in ipairs(fleethooks) do
         hook.rm(v)
      end

      fleethooks = nil
   end
end

function spawn_fleet() -- spawn warlord killing fleet
   -- Cancel autonav.
   player.autonavAbort()
   mem.jump_fleet_entered = true
   local dv_med_force = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance" }
   jump_fleet = fleet.add( 1, dv_med_force, attkfaction, destjump, nil, {ai="dvaered_norun"} )
   broadcast_first(jump_fleet, fmt.f(_("{pnt} will be ours! Khan, prepare to die!"), {pnt=destplanet}))
   for i, j in ipairs(jump_fleet) do
      j:setHilight(true)
      j:setVisible()
      j:control()
      j:attack(hawk)
   end
   hook.pilot( jump_fleet[6], "death", "jump_fleet_cap_dead")
   camera.set( hawk, false )
   hawk:broadcast(_("All units, defend Hawk, we are under attack!"))
   broadcast_first(fleetdv, _("All units, defend Hawk, we are under attack!"))
   hawk:control()
   hawk:land(destplanet)

   for i, j in ipairs(fleetdv) do
      if j:exists() then
         j:changeAI("dvaered_norun")
         j:control(false)
         j:setFriendly()
         j:setInvincible(true)
      end
   end

   -- Give the escorts a few seconds to get away from the player.
   hook.timer(3.0, "undo_invuln")
end

function undo_invuln()
   for k, v in ipairs(fleetdv) do
      if v:exists() then
         v:setInvincible(false)
      end
   end
end

function jump_fleet_cap_dead () -- mission failed
   jump_fleet[6]:broadcast(_("Arrgh!"))

   hawk:broadcast(_("Pathetic, can't even take down an unarmed ship."))
   hawk:setNoDeath()
   tk.msg(_("The Hawk is safe."), _("The Hawk was able to fend off the attackers and destroy their flagship. You have failed your mission."))
   faction.get("Dvaered"):modPlayerSingle(-5)
   hawk:land(destplanet)
   for i, j in ipairs(fleetdv) do
      if j:exists() then
         j:control()
         j:follow(hawk)
         j:setHilight(false)
      end
   end
   for i, j in ipairs(jump_fleet) do
      if j:exists() then
         j:control()
         j:follow(hawk)
         j:setHilight(false)
      end
   end
   hook.timer(10.0, "abort")
end

function cleanup()
   if jump_fleet then
      for k, v in ipairs(jump_fleet) do
         if v:exists() then
            v:setHilight(false)
            v:setVisible(false)

            if hawk and not hawk:exists() then
               v:setFriendly()
            end
         end
      end
   end

   if not fleetdv then
      return
   end

   for k, v in ipairs(fleetdv) do
      if v:exists() then
         v:setHilight(false)
         v:setVisible(false)
      end
   end
end

function complete()
   cleanup()

   vn.clear()
   vn.scene()
   local dv = vn.newCharacter( npc_name, {image=npc_image, shader=love_shaders.hologram()} )
   vn.transition("electric")

   vn.na(_([[The Dvaered official opens a communication channel with your ship.]]))
   dv(fmt.f(_([["Thanks for the distraction. I've sent you a picture of all the medals I was awarded. Oh, and I also deposited {credits} in your account."]]),
      {credits=fmt.credits(reward)}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay(reward)
   end )
   vn.na(fmt.reward(reward))
   vn.done("electric")

   vn.run()

   camera.set( nil, false )
   jump_fleet[6]:broadcast(fmt.f(_("I declare myself the Warlord of {pnt}!"), {pnt=destplanet}))
   jump_fleet[6]:setNoDeath(false)
   misn.finish(true)
end

function abort()
   cleanup()
   camera.set( nil, true )
   misn.finish(false)
end
