--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Adblocker">
 <unique />
 <chance>10</chance>
 <cond>
   local mtest = require("misn_test")
   if not mtest.cargo(true) or not mtest.reweight_active() then
      return false
   end
   return true
 </cond>
 <location>Bar</location>
</mission>
--]]
--[[
    Adblocker
    Author: Lemuria

    Player has to kill a ship that's been spamming the local comms with
    excessive advertisements.

   I was bored and had nothing better to do, so I made this Naev mission,
   my first.
--]]

local fmt = require "format"
local neu = require 'common.neutral'
local vn = require "vn"
local vnimage  = require "vnimage"
local ads = require "scripts.common.ads"

local mission = {
   name = _("Adblocker"),
   description = _("Destroy a ship spamming the {sys} system with excessive advertising transmissions"),
   reward = 300e3,
   npc = {
      name = _("Desperate captain"),
      description = _("You see a desperate looking captain.")
   }
}

function create()
   mem.npc_image, mem.npc_portrait = vnimage.genericMale()

   mem.current_system = system.cur()
   mem.current_spob = spob.cur()
   misn.setNPC(mission.npc.name, mem.npc_portrait, mission.npc.description)

   if not misn.claim(mem.current_system,true) then
      misn.finish(false)
   end
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local man = vn.newCharacter(mission.npc.name, { image = mem.npc_image } )
   vn.transition()
   man(fmt.f(_([["Look, I don't have much time! There's this annoying ship that's been spamming the local comms with tons of advertisements! I can't take it any more! Please, you've got to stop it! I'll give you {creds} if you stop it!"]])
      , { creds = fmt.credits(mission.reward) }))
   vn.menu {
      { _([[Accept]]), "accept" },
      { _([[Refuse]]), "refuse" },
   }

   vn.label("refuse")
   vn.na(_("You walk away, ignoring him."))
   vn.done()

   vn.label("accept")
   man(_([["Thank you! Again, just stop that thing! Do what you have to!"]]))
   vn.func(function()
      accepted = true
   end)
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.setTitle(mission.name)
   misn.setDesc(fmt.f(_("A ship is currently spamming {sys} with tons of unwanted advertisements. A desperate captain has asked you to destroy, or disable it.")
      , { sys = mem.current_system }))
   misn.setReward(mission.reward)
   misn.osdCreate( mission.name, {
      fmt.f(mission.description, { sys = mem.current_system }),
      fmt.f(_("Return to {spob} ({sys} system)"), { spob = mem.current_spob, sys = mem.current_system })
   })
   hook.enter("enter")
end

local spammer
function enter()
   if system.cur() == mem.current_system then
      local location = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
      local fct = faction.dynAdd("Independent", "adspammer", _("Independent"), { clear_enemies = true,
         clear_allies = true })
      spammer = pilot.add("Gawain", fct, location, _("Advertiser 108CK"))
      spammer:control()
      spammer:memory().aggressive = true
      spammer:setHilight(true)
      spammer:setVisplayer(true)
      spammer:setHostile(true)
      mem.hk_advert_spam = hook.timer(1, "timer_advert_spam")
      hook.pilot(spammer, "exploded", "on_target_stopped")
      hook.pilot(spammer, "death", "on_target_stopped")
   end
end

local adlist
function timer_advert_spam()
   if not spammer:exists() then return end

   -- Generate ads if not available
   if not adlist then
      adlist = rnd.permutation( ads.system_ads() )
   end

   -- Only spam if not disabled
   if not spammer:flags("disabled") then
      mem.spammer = mem.spammer or 0
      mem.spammer = math.fmod(mem.spammer, #adlist) + 1
      spammer:broadcast(adlist[mem.spammer], true)
   end

   mem.hk_advert_spam = hook.timer(1, "timer_advert_spam")
end

local stopped
function on_target_stopped()
   if stopped then
      return
   end
   misn.osdActive(2)
   mem.marker_return = misn.markerAdd(mem.current_spob, "low")
   hook.rm(mem.hk_advert_spam)
   hook.land("on_land")
   stopped = true
end

function on_land()
   if spob.cur() ~= mem.current_spob then
      return
   end

   vn.clear()
   vn.scene()
   local man = vn.newCharacter(mission.npc.name, { image = mem.npc_image })
   vn.transition()
   man(_([[The man runs towards you. "Thank you so much for destroying that ship! The advertisements were about to drive me crazy! Man they're so annoying!"]]))
   vn.sfxVictory()
   vn.func(function()
      player.pay(mission.reward)
   end)
   vn.na(fmt.reward(mission.reward))
   vn.run()

   neu.addMiscLog(_("You destroyed a ship spamming advertisements."))
   misn.finish(true)
end
