--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Hitman 2">
 <unique />
 <priority>2</priority>
 <chance>10</chance>
 <location>Bar</location>
 <cond>system.cur() == system.get("Alteris")</cond>
 <done>Hitman</done>
</mission>
--]]
--[[

   Pirate Hitman 2

   Corrupt Merchant wants you to destroy competition

   Author: nloewen

--]]
local fmt = require "format"
local pir = require "common.pirate"
local portrait = require "portrait"
local vn = require "vn"

-- Mission constants
local targetsystem = system.get("Delta Pavonis")

local reward = 500e3

local npc_name = _("Shifty Trader")
local npc_portrait = "neutral/unique/shifty_merchant.webp"
local npc_image = portrait.getFullPath( npc_portrait )

local attack_finished -- Forward-declared functions

function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( npc_name, npc_portrait, _("You see the shifty merchant who hired you previously. He looks somewhat anxious, perhaps he has more business to discuss."))
end


--[[
Mission entry point.
--]]
function accept ()
   local accepted = false
   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   m(_([[As you approach, the man turns to face you and his anxiety seems to abate somewhat. As you take a seat he greets you, "Ah, so we meet again. My, shall we say… problem, has recurred." Leaning closer, he continues, "This will be somewhat bloodier than last time, but I'll pay you more for your trouble. Are you up for it?"]]))
   vn.menu{
      {_("Accept"),"accept"},
      {_("Decline"),"decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   m(_([[He nods approvingly. "It seems that the traders are rather stubborn. They didn't get the message last time and their presence is increasing." He lets out a brief sigh before continuing, "That simply won't do. It's bad for business. Perhaps if a few of their ships disappear, they'll get the hint." With the arrangement in place, he gets up. "I look forward to seeing you soon. Hopefully this will be the end of my problems."]]) )

   vn.run()

   if not accepted then return end

   misn.accept()

   -- Some variables for keeping track of the mission
   mem.misn_done      = false
   mem.attackedTraders = {}
   mem.deadTraders = 0
   mem.misn_base, mem.misn_base_sys = spob.cur()

   -- Set mission details
   misn.setTitle( _("Assassin") )
   misn.setReward( _("Some easy money") )
   misn.setDesc( fmt.f( _("A shifty businessman has tasked you with killing merchant competition in the {sys} system."), {sys=targetsystem} ) )
   mem.misn_marker = misn.markerAdd( targetsystem, "low" )
   misn.osdCreate( _("Assassin"), {
      fmt.f(_("Kill Trader pilots in the {sys} system"), {sys=targetsystem} ),
      fmt.f(_("Return to {pnt} in the {sys} system for payment"), {pnt=mem.misn_base, sys=mem.misn_base_sys} ),
   } )

   -- Set hooks
   hook.enter("sys_enter")
end

-- Entering a system
function sys_enter ()
   mem.cur_sys = system.cur()
   -- Check to see if reaching target system
   if mem.cur_sys == targetsystem then
      hook.pilot(nil, "death", "trader_death")
   end
end

-- killed a trader
function trader_death (hook_pilot, hook_attacker, _arg)
   if mem.misn_done then
      return
   end
   local pp = player.pilot()

   if ( hook_pilot:faction() == faction.get("Trader")
            or hook_pilot:faction() == faction.get("Traders Society") )
         and ( hook_attacker == pp
            or hook_attacker:leader() == pp ) then
      attack_finished()
   end
end

-- attack finished
function attack_finished()
   if mem.misn_done then
      return
   end
   mem.misn_done = true
   player.msg( _("MISSION SUCCESS! Return for payment.") )
   misn.markerRm( mem.misn_marker )
   mem.misn_marker = misn.markerAdd( mem.misn_base, "low" )
   misn.osdActive(2)
   hook.land("landed")
end

-- landed
function landed()
   if spob.cur() ~= mem.misn_base then
      return
   end

   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   m(_([[You glance around, looking for your acquaintance, but he has noticed you first, motioning for you to join him. As you approach the table, he smirks. "I hope the Empire didn't give you too much trouble." After a short pause, he continues, "The payment has been transferred. Much as I enjoy working with you, hopefully this is the last time I'll require your services."]]))

   vn.func( function ()
      player.pay(reward)
      pir.modDecayFloor(3)
      pir.modReputation(3)
      faction.modPlayerSingle("Pirate", 5)
   end )
   vn.sfxVictory()
   vn.na(fmt.reward(reward))

   vn.run()

   pir.addMiscLog(_([[You assassinated some of the shifty merchant's competition and were paid a sum of credits for your services. He said that he should hopefully not require further services from you.]]))
   misn.finish(true)
end
