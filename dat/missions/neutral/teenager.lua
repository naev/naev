--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The macho teenager">
 <unique />
 <priority>4</priority>
 <chance>10</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <cond>
   local misn_test = require "misn_test"
   if spob.cur():tags().station then
      return false
   end
   if not misn_test.mercenary() then
      return false
   end
   return misn_test.reweight_active()
 </cond>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   MISSION: The macho teenager
   DESCRIPTION: A man tells you that his son has taken one of his yachts without permission and
   is joyriding it with his girlfriend to impress her. Disable the yacht and board it, then take
   the couple back to the planet (destroying the yacht incurs a penalty)

--]]
local fmt = require "format"
local neu = require 'common.neutral'
local vn = require "vn"
local vntk = require "vntk"
local portrait = require "portrait"

local reward = 300e3

local target -- Non-persistent state

local title = _("The Macho Teenager")
local npc_name = _("A middle-aged man")
local npc_portrait = "neutral/unique/middleaged.webp"
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   mem.cursys = system.cur()
   mem.curplanet = spob.cur()
   misn.setNPC( npc_name, npc_portrait, _("You see a middle-aged man, who appears to be one of the locals, looking around the bar, apparently in search of a suitable pilot."))
end


function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local man = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   man(_([["Excuse me," the man says as you approach him. "I'm looking for a capable pilot to resolve a small matter for me. Perhaps you can help me? You see, it's my son. He's taken my yacht, along with his girlfriend, to space without my permission. That boy is such a handful. I'm sure he's trying to show off his piloting skills to impress her. I need you to get out there, disable the yacht, and take them both back here. Can you do this for me? I'll make it worth your while."]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   vn.done()

   vn.label("accept")
   man(_([["Thank you! The yacht doesn't have a working hyperdrive, so they won't have left the system. It's a Gawain named Credence. Just disable it, board it, and then transport my disobedient son and his girlfriend back here. Don't worry about the yacht, I'll have it recovered later. Oh, and one more thing, though it should go without saying: whatever you do, don't destroy the yacht! I don't want to lose my son over this. Well then, I hope to see you again soon."]]))
   vn.func( function ()
      accepted = true
   end )
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.setTitle(title)
   misn.setDesc(fmt.f(_("A disgruntled parent has asked you to fetch his son and his son's girlfriend, who have taken a yacht and are joyriding it in the {sys} system."), {sys=mem.cursys}))
   misn.setReward(_("You will be compensated for your efforts."))
   misn.osdCreate(title, {
      _("Disable Gawain Credence"),
      fmt.f(_("Bring the teenagers back to planet {pnt}"), {pnt=mem.curplanet}),
   })
   hook.enter("enter")
   mem.targetlive = true
end

local hk_exploded
function enter()
   if system.cur() == mem.cursys and mem.targetlive then
      local location = vec2.newP(rnd.rnd() * system.cur():radius(), rnd.angle())
      local fct = faction.dynAdd( "Independent", "teenager", _("Independent"), {clear_enemies=true, clear_allies=true} )
      target = pilot.add( "Gawain", fct, location, _("Credence") )
      target:control()
      target:memory().aggressive = true
      target:setHilight(true)
      target:setVisplayer(true)
      mem.hidle = hook.pilot(target, "idle", "targetIdle")
      hk_exploded = hook.pilot(target, "exploded", "targetExploded")
      hook.pilot(target, "board", "targetBoard")
      targetIdle()
   end
end

function targetIdle()
   if not target:exists() then -- Tear down now-useless hooks.
      hook.rm(mem.hidle)
      return
   end
   local location = target:pos()
   local newlocation = vec2.newP(750, rnd.angle())
   target:taskClear()
   target:moveto(location + newlocation, false, false)
   hook.timer(5.0, "targetIdle")
end

function targetExploded()
   hook.timer( 2.0, "targetDeath" )
end

function targetDeath()
   local fine = math.max(-20e3, -player.credits())
   vntk.msg(_("Whoops!"), fmt.f(_([[You have destroyed the Gawain! The family presses charges, and you are sentenced to a {credits} fine in absentia.]]), {credits=fmt.credits(-fine)}))
   player.pay(fine) -- I love this statement.
   misn.finish(false) -- be nice and let them repeat
end

function targetBoard()
   player.unboard()
   vntk.msg(_("End of the line, boyo"), _([[You board the Gawain and find an enraged teenage boy and a disillusioned teenage girl. The boy is furious that you attacked and disabled his ship, but when you mention that his father is quite upset and wants him to come home right now, he quickly pipes down. You march the young couple onto your ship and seal the airlock behind you.]]))
   target:setHilight(false)
   target:setVisplayer(false)
   target:disable()
   hook.rm( hk_exploded )
   local c = commodity.new( N_("Teenagers"), N_("Disillusioned teenagers.") )
   mem.cargoID = misn.cargoAdd(c,0)
   misn.osdActive(2)
   hook.land("land")
end

function land()
   if spob.cur() ~= mem.curplanet then
      return
   end

   vn.clear()
   vn.scene()
   local man = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   man(_([[The boy's father awaits you at the spaceport. He gives his son and the young lady a stern look and curtly commands them to wait for him in the spaceport hall. The couple despondently walks off, and the father turns to face you.
"You've done me a service, Captain," he says. "As promised, I have a reward for a job well done. You'll find it in your bank account. I'm going to give my son a reprimand he'll not soon forget, so hopefully he won't repeat this little stunt anytime soon. Well then, I must be going. Thank you again, and good luck on your travels."]]))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( reward )
   end )
   vn.na(fmt.reward(reward))
   vn.run()

   neu.addMiscLog( _("Through force, you helped bring some young teenagers back from a joyride.") )
   misn.finish(true)
end

function abort ()
   if target then
      target:setHilight(false)
      target:setVisplayer(false)
   end
   misn.finish(false)
end
