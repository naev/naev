--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Antivirus">
 <unique />
 <chance>10</chance>
 <done>Adblocker</done>
 <chapter>[^0]</chapter>
 <cond>
   local scur = spob.cur()
   if system.get("Straight Row"):jumpDist( system.cur() ) &gt; 8 then
      return false
   elseif not scur:faction():tags():generic() then
      return false
   end
   local maxsize = player.pilot():ship():size()
   for k,s in ipairs(player.ships()) do
      maxsize = math.max( maxsize, s.ship:size() )
   end
   if maxsize &lt; 6 then
      return false
   end
   local misn_test = require "misn_test"
   return misn_test.mercenary() and misn_test.reweight_active()
 </cond>
 <location>Bar</location>
</mission>
--]]
--[[
   Sequel to/edit of Adblocker; player has to fight the 4DV3RT1Z3R capship.
   - Zivi (hsza)
--]]

local fmt = require "format"
local neu = require 'common.neutral'
local vn = require "vn"
local vnimage  = require "vnimage"
local ads = require "scripts.common.ads"
local pilotai = require "pilotai"
local equipopt = require "equipopt"
local pp_shaders = require 'pp_shaders'
local tut = require "common.tutorial"

local mission = {
   name = _("Antivirus"),
   description = _("Take down the advertising bandit in {sys}"),
   reward = 1e6,
   npc = {
      name = _("Officer"),
      description = _("You see a representative of local law enforcement, considering you with a thoughtful expression.")
   }
}

function create()
   mem.npc_image, mem.npc_portrait = vnimage.genericMale()

   mem.starting_system = system.cur()
   mem.starting_spob = spob.cur()
   mem.fight_system = system.get("Straight Row")
   misn.setNPC(mission.npc.name, mem.npc_portrait, mission.npc.description)

   if not misn.claim( { mem.starting_system, mem.fight_system }, true ) then
      misn.finish(false)
   end
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local man = vn.newCharacter(mission.npc.name, { image = mem.npc_image } )
   vn.transition()
   man(fmt.f(_([["On behalf of the {spob} government, I have been investigating some of the advertiser ships in this sector of space. You see, sometimes they behave all too erratically, sending hundreds of advertisements a hectostu, and when we board one of them, the captain is none the wiser!"]]), { spob = spob.cur() }))
   man(fmt.f(_([["Someone has beem planting malware on those ships to fulfil dozens of advertising contracts simultaneously and funnel all of the profits to themselves. Too bad for them - we have managed to track the head of the operation down to the {sys} system. Several of our own agent barely got out of there alive, so we need a more than capable pilot to take the bandit down. Pay is {creds}. Willing to give it a go?"]]), { sys = mem.fight_system, creds = fmt.credits(mission.reward) }))
   vn.menu {
      { _([[Accept]]), "accept" },
      { _([[Refuse]]), "refuse" },
   }

   vn.label("refuse")
   man(_("Ah, too bad. Might have been wise of you to pass on this job, though."))
   vn.done()

   vn.label("accept")
   man(fmt.f(_([["Again, the target should be somewhere in {sys} at the moment. Come prepared."]]), { sys = mem.fight_system }))
   vn.func(function()
      accepted = true
   end)
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.setTitle(mission.name)
   mem.marker_boss = misn.markerAdd(mem.fight_system)
   misn.setDesc(fmt.f(_("Battle the orchestrator of advertising spam in {sys}.")
      , { sys = mem.fight_system }))
   misn.setReward(mission.reward)
   misn.osdCreate( mission.name, {
      fmt.f(mission.description, { sys = mem.fight_system }),
      fmt.f(_("Return to {spob} ({sys} system)"), { spob = mem.starting_spob, sys = mem.starting_system })
   })
   hook.enter("enter")
end

local boss
local location
function enter()
   if system.cur() == mem.fight_system and not mem.stopped then
      location = vec2.newP(rnd.rnd() * 0.5 * system.cur():radius(), rnd.angle())
      local fct = faction.dynAdd("Dummy", "adspammer", _("???"), { clear_enemies = true, clear_allies = true, ai="baddie" })
      boss = pilot.add("Dealbreaker", fct, location, _("4DV3RT1Z3R >:)"))
      pilotai.guard( boss, location )
      boss:memory().aggressive = true
      boss:setHilight(true)
      boss:setHostile(true)
      boss:setNoDisable(true)
      boss:setNoJump(true)
      boss:setNoLand(true)
      boss:intrinsicSet('shield_mod', 30)
      boss:intrinsicSet('armour_mod', 25)
      boss:intrinsicSet('ew_detect', 100)
      boss:intrinsicSet('ew_hide', 200)
      boss:intrinsicSet('jam_chance', 60)
      -- boss:intrinsicSet('fbay_capacity', 20)
      boss:intrinsicSet('cpu_mod', 20)
      local params = {
         rnd = 0,
         launcher = 0.9,
         pointdefence = 0.6,
         fighterbay = 3,
         outfits_add = {
            "Berserk Chip",
         },
         prefer = {
            ["Milspec Scrambler"]         = 100,
            ["Za'lek Bomber Drone Bay"]   = 2,
            ["Faraday Tempest Coating"]   = 0,
            ["Emergency Shield Booster"]  = 2,
            ["Berserk Chip"]              = 2,
         },
         cores = {
            systems = "Milspec Aegis 8501 Core System",
            systems_secondary = "Milspec Thalos 8502 Core System",
            engines = "Melendez Mammoth Engine",
            engines_secondary = "Tricon Typhoon Engine",
            hull = "Red Star Large Cargo Hull",
            hull_secondary = "Red Star Large Cargo Hull",
         },
      }
      equipopt.zalek( boss, params )

      mem.hk_advert_spam = hook.timer(1, "timer_advert_spam")
      mem.hk_ew_attack_start = hook.timer(60, "timer_ew_attack_start")
      hook.pilot(boss, "exploded", "on_target_stopped")
      hook.pilot(boss, "death", "on_target_stopped")
   end
end

-- Generate ads if not available
local adlist = rnd.permutation( ads.system_ads(true) )

local minions
local called_minions = false
function timer_advert_spam()
   if not boss:exists() then return end

   -- Only spam if not disabled
   if not boss:disabled() then
      mem.ad = mem.ad or 0
      mem.ad = math.fmod(mem.ad, #adlist) + 1
      local msg = string.upper(adlist[mem.ad])
      boss:broadcast(msg, true)
      for k,v in ipairs(boss:followers()) do
         if rnd.rnd() < 0.25 then v:rename(adlist[mem.ad]) end -- Rename all the drones too for the fun of it
         if rnd.rnd() < 0.25 then
            local m = v:memory()
            m.comm_greet = msg
            m.bribe_no = msg
            m.comm_no = msg
         end
      end
      local m = boss:memory()
      m.comm_greet = msg
      m.bribe_no = msg
      m.comm_no = msg
      if boss:shield() < 65 and not called_minions then -- Good opportunity to check if we want to summon reinforcements yet
         minions()
         called_minions = true
      end
   end

   mem.hk_advert_spam = hook.timer(0.4, "timer_advert_spam")
end

local noise_shader = pp_shaders.corruption( 2.0 )
local victims = {}

function timer_ew_attack_start()
   if not boss:exists() or boss:disabled() then return end

   mem.hk_ew_attack_end = hook.timer(11, "timer_ew_attack_end")
   for k,v in ipairs(boss:getEnemies(nil, nil, nil, nil, true)) do
      v:intrinsicSet("ew_detect", -100, true)
      table.insert(victims, v)
   end
   boss:jamLockons()
   player.msg(_([[The advertisements are overloading your ship!]]), true)
   shader.addPPShader( noise_shader )
end

function timer_ew_attack_end()
   if boss:exists() and not boss:disabled() then
      mem.hk_ew_attack_start = hook.timer(rnd.rnd(31, 34), "timer_ew_attack_start")
   end

   for k,v in ipairs(victims) do
      if v:exists() then v:intrinsicSet("ew_detect", 0, true) end
   end
   victims = {}
   if noise_shader then
      shader.rmPPShader( noise_shader )
   end
end

local minions_tb = {}
local minions_spob = "Thaddius Terminal"
function minions()
   local fct = faction.dynAdd("Dummy", "adspammer", _("???"), { clear_enemies = true, clear_allies = true, ai="baddie" })
   local puppets = {
      'Admonisher', 'Vendetta', 'Gawain', 'Lancelot',
      'Ancestor', 'Phalanx', 'Hyena', 'Shark',
   }
   for i=1,5 do
      -- local loc = vec2.newP(rnd.rnd() * 0.4 * system.cur():radius(), rnd.angle()) + location
      local e = pilot.add(puppets[rnd.rnd( #puppets )], fct, minions_spob, fmt.f(_("Advertiser {n}DK"), { n = tostring(167+(i*rnd.rnd(3, 5)) )}, {naked=true}))
      equipopt.generic( e, nil, "elite" )
      e:setLeader( boss )
      e:setHostile(true)
      local m = e:memory()
      local msg = string.upper(adlist[mem.ad])
      m.comm_greet = msg
      m.bribe_no = msg
      m.comm_no = msg
      table.insert(minions_tb, e)
      hook.pilot(e, "board", "board_puppet")
   end
end

function board_puppet( pilot )
   if not mem.boarded_puppet then
      local sai = tut.vn_shipai()
      local rwd = outfit.get("AD-USB")
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[As you move in to board the ship, it seems as there's no way inside. Docking at all is quite a challenge, and the airlock is completely sealed up. However, with some effort, and at a cost to the ship's hull, you are able to force your way in.]]))
      vn.na(_([[You rush to investigate the interior. The first thing you see is a few partway-decomposed bodies scattered by the airlock. Was this the original crew of the ship? It seems as the airlock and life support systems have been nonfunctional for a considerable amount of time.]]))
      vn.appear( sai, tut.shipai.transition )
      sai(_([["I suppose you would be curious about this ship's logs, but I'm wary of connecting to the systems - most likely, it's under complete control of the malware that investigator was talking about, and who knows what it can do?"]]))

      vn.menu {
      { _([[Try to connect]]), "uhoh" },
      { _([[Do not]]), "uhoh" },
      }

      vn.label("uhoh")
      vn.disappear( sai, tut.shipai.transition )
      vn.na(_([[As you're about to make a decision, the screens in the cockpit flash a warning - "SECURITY SELF-DESTRUCT INITIATED". Welp, that's your cue to get out of here. Better luck next time?]])) -- codespell:ignore welp
      vn.na(_([[Rushing out the compromised ship, you spot a strange device connected to one of the terminals, and curiosity gets the better of you. This must contain the malicious payload - could it be worth studying? You grab it as you run back to your ship.]]))
      vn.func( function ()
         player.outfitAdd( rwd )
      end )
      vn.sfxBingo()
      vn.na(fmt.reward(rwd))
      vn.run()
      mem.boarded_puppet = true
   end
   player.unboard()
   pilot:setHealth(-1, -1)
end

mem.stopped = false
function on_target_stopped()
   if mem.stopped then
      return
   end
   for k,v in ipairs(minions_tb) do
      if v:exists() then v:setDisable() end
   end
   misn.osdActive(2)
   misn.markerRm()
   mem.marker_return_spob = misn.markerAdd(mem.starting_spob, "low")
   mem.marker_return_sys = misn.markerAdd(mem.starting_system, "low")
   hook.rm(mem.hk_advert_spam)
   hook.rm(mem.hk_ew_attack_start)
   hook.land("on_land")
   mem.stopped = true
end

function on_land()
   if spob.cur() ~= mem.starting_spob then
      return
   end

   vn.clear()
   vn.scene()
   local man = vn.newCharacter(mission.npc.name, { image = mem.npc_image })
   vn.transition()
   man(_([["You're back? The target is eliminated? What a relief! You won't believe just how irritating that scheme has been. It will take me a while to write my report, but you're due your reward."]]))
   vn.sfxVictory()
   vn.func(function()
      player.pay(mission.reward)
   end)
   vn.na(fmt.reward(mission.reward))
   vn.run()

   neu.addMiscLog(_("You defeated a malware vendor and advert defrauder to the great relief of ad-loathing captains around the universe."))
   misn.finish(true)
end
