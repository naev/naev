--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Long Distance Recruitment">
 <unique />
 <priority>4</priority>
 <cond>
   if faction.playerStanding("Empire") &lt; 0 then
      return false
   end
   if spob.cut()==spob.get("Halir") then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <chance>100</chance>
 <done>Sirius Long Distance Recruitment</done>
 <location>Bar</location>
 <faction>Empire</faction>
 <tags>
  <tag>emp_cap_ch01_lrg</tag>
 </tags>
 <notes>
  <campaign>Empire Shipping</campaign>
 </notes>
</mission>
--]]
--[[

   Sixth (and final) mission that explains the Empire long-distance cargo missions.

   Author: micahmumper

]]--
local fmt = require "format"
local emp = require "common.empire"
local vn = require "vn"

-- Mission constants
local targetworld, targetworld_sys = spob.getS("Halir")

function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( emp.czesc.name, emp.czesc.portrait, emp.czesc.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local czesc = vn.newCharacter( emp.vn_czesc() )
   vn.transition( emp.czesc.transition )

   -- Intro Text
   czesc(fmt.f( _([[Lieutenant Czesc slaps you on the back as you take a seat next to him at the bar. "We've done it! We have set up Empire Armada Shipping outposts across quite a bit of the galaxy. I just have one more favour to ask. I need transport back to {pnt} in the {sys} system. Once there, I can authorize you to help out with the long-distance shipping missions. Can I count on you?"]]),
      {pnt=targetworld, sys=targetworld_sys} ) )
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.czesc.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   czesc(_([[Internally you groan from the idea of having to do another haul across the galaxy for more paperwork, but at least you'll have access to new missions. Lieutenant Czesc excitedly gets up from the bar. "Let's get going as soon as possible. There's no place like home!"]]))
   vn.func( function () accepted = true end )

   vn.done( emp.czesc.transition )
   vn.run()

   if not accepted then return end

   ---Accept the mission
   misn.accept()

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setTitle(_("Empire Long Distance Recruitment"))
   misn.setReward( emp.rewards.ldc6 )
   local misn_desc = fmt.f(_("Deliver Lieutenant Czesc to {pnt} in the {sys} system"), {pnt=targetworld, sys=targetworld_sys})
   misn.setDesc( misn_desc )
   misn.osdCreate(_("Empire Long Distance Recruitment"), {misn_desc})

   -- Set up the goal
   misn.markerAdd( targetworld, "low" )
   hook.land("land")
   local c = commodity.new( N_("Diplomat"), N_("An Imperial trade representative.") )
   mem.person = misn.cargoAdd( c, 0 )
end


function land()
   if spob.cur() ~= targetworld then
      return
   end

   vn.clear()
   vn.scene()
   local czesc = vn.newCharacter( emp.vn_czesc() )
   vn.transition( emp.czesc.transition )

   -- More flavour text
   czesc(_([[Lieutenant Czesc exits your ship and takes a deep breath of air. "I love the smell of bureaucracy in the morning."]]))
   czesc(_([[He shakes your hand. "Thanks for all your help, Captain! Follow me to headquarters and we can do some paperwork to get you all set up. After that you should start to receive long-distance shipping missions. They pay better than our regular shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions. Again, I can't thank you enough! The Empire does not quickly forget such dedication."]]) )

   vn.sfxVictory()
   vn.func( function ()
      player.pay( emp.rewards.ldc6 )
      faction.modPlayerSingle( "Empire",3 )
   end )
   vn.na(fmt.reward( emp.rewards.ldc6 ))

   vn.done( emp.czesc.transition )
   vn.run()

   emp.addShippingLog( fmt.f( _([[You transported Lieutenant Czesc to {pnt} for some paperwork. You can now do long-distance cargo missions for the Empire. They pay better than regular Empire shipping missions, but often require traveling longer distances and into territory controlled by other factions. You'll probably be more likely to see them on the edges of Empire space where cargo is ready to head out to other factions.]]), {pnt=targetworld} ) )
   misn.finish(true)
end
