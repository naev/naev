--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 10">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Antlejos V</spob>
 <cond>require('common.antlejos').unidiffLevel() &gt;= 9</cond>
 <done>Terraforming Antlejos 9</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Simple epilogue that gives the player a reward.
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"

local reward = outfit.get("Commemorative Stein")

function create ()
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

function accept ()
   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   if var.peek("antlejos_drank") then
      vn.na(_([[You find Verner drinking his usual foul Antlejos Brew at the bar. You can't help to wince and gag a bit upon the smell pervading your nostrils.]]))
   else
      vn.na(_([[You find Verner drinking his usual foul Antlejos Brew at the bar. He seems glad to see you.]]))
   end
   v(_([["Hey, glad to see you around again. You feel that? We finally breached the planet core and were able to install the final gravitational wave generators. It feels much better now!"
He does a little jump to make a point. It does look like there is less buoyancy than before.]]))
   v(_([["We also finally got the paperwork done to be accepted as an independent trading world by the Empire. With this, most of the PUAAA should be out of the way, with only maybe a few confused stragglers still coming to harass us. All in all a great improvement! I never want to go through that paperwork again, if I have to submit another triple signed verified EW-104-3920 I'll jump out an airlock."]]))
   v(_([["We're going to have a small event to celebrate all the good happenings. As the hero of Antlejos V, it would be my honour to invite you."]]))
   vn.na(_([[Seeing you aren't really given a choice, you accept Verner's offer.]]))
   vn.na(_([[He leads you to the largest room at the base, which is the not very elegant cafeteria. He opens the door and your are greeted by a myriad of faces, some which you recognize as colonists you helped bring over to Antlejos V from different parts of the galaxy. A cheer erupt from the crowd as you go to the center of the room.]]))
   vn.na(_([[Verner almost slips when he jumps onto a table, likely not entirely used to the new gravity, and then helps you up. People start pouring and sharing foul smelling drinks, but Verner hands you a glass with something that smells much better and winks at you. After a bit it seems like everyone has gotten their drinks, and Verner directs himself to the crowd.]]))
   v(_([["Thank you everyone for coming. It's been a long journey that has led us up to this moment. I used to have a place that I could call home, but that was lost, like a great many other things in that fateful day when everything went to shit. The Incident took a great many things from me, and I thought I would never be complete again: just a husk of a human doomed to venture space until their demise. For many periods, that is all I knew. I know I am not alone. Many of you were just like me. Lost souls."]]))
   v(fmt.f(_([["However, thanks to {playername}, we have finally been able to find that which we had thought was lost forever. We have finally been able to find a place that we can call home."
He raises his stein into the air, with you and everyone else following his lead.
"To {playername}, the Hero of Antlejos, and the prosperity of Antlejos V. Our future begins now!"]]),
      {playername=player.name()}))
   vn.na(_([[The crowd begins cheering and the festivities commence…]]))
   vn.na(_([[The party is a blur, and you don't recall much other than Verner insists that you take a commemorative stein as the Hero of Antlejos. While wondering what the future holds for Antlejos V, you return to your ship, ready to take once again to the skies…]]))
   vn.na( fmt.reward(reward) )
   vn.run()

   player.outfitAdd( reward )
   ant.log(_([[You were given a Commemorative Stein from Verner for helping terraform Antlejos V.]]))

   news.add{
      {
         faction = "Generic",
         head = _("New Independent Spaceport Antlejos V"),
         body = _("Through a long process of terraforming, the once barren moon Antlejos V has become officially been recognized as a new trading independent spaceport by the Great Houses."),
         date_to_rm = time.get()+time.new(0,20,0),
      },
   }

   misn.finish(true)
end
