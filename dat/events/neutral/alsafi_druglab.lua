--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Alsafi Druglab">
 <location>land</location>
 <chance>100</chance>
 <spob>Alsafi II</spob>
 <unique />
</event>
--]]
local neu = require "common.neutral"
local vn = require "vn"
local vnimage = require "vnimage"
local fmt = require "format"
local poi = require "common.poi"

local VAR = "alsafi_ii_druglab"
local KILL_MONEY = 283782
local DRUG = commodity.get("Neblaze")

function create ()
   local result

   -- TODO give a fixed image if we potentially have them appear again
   local image = vnimage.pirate()

   vn.reset()
   vn.scene()
   local voice = vn.newCharacter( _("Voice") )
   vn.transition()
   vn.na(_([[You sit in your command chair, blanking out as you look at the endless nothingness of Alsafi II. Nothing is good for the soul.]]))
   vn.na(_([[To your chagrin, you see what seems to be a dust covered hatch open and someone, or something pulls them out of the ground. They stretch a bit before they notice your ship, and, pretending as if nothing happened, backtrack back into the surface and close the hatch. Whelp, there goes your nothingness.]]))
   vn.menu{
      {_([[Go investigate.]]), "01_investigate" },
      {_([[Ignore it.]]), "01_ignore" },
   }

   vn.label("01_ignore")
   vn.na(_([[You ignore the intrusive event and go back to spacing out. Nothing else happens to your full enjoyment.]]))
   vn.done()

   vn.label("01_investigate")
   vn.na(_([[You don your space suit, grab a weapon, and head out into the vast nothingness.]]))
   vn.na(_([[You eventually reach more or less the location, and after a bit of searching, find a hatch that seems to be closed.]]))
   vn.menu{
      {_([[Stomp on it.]]), "02_stomp" },
      {_([[Shoot at it.]]), "02_shoot" },
      {_([[Open it.]]), "02_open" },
   }

   vn.label("02_stomp")
   vn.na(_([[You angrily stomp at the annoying hatch for interrupting your nothingness until you are happy.]]))
   voice(_([["Stop it." says a little voice from behind the hatch.]]))
   vn.jump("02_cont")

   local shot = false
   vn.label("02_shoot")
   vn.func( function () shot = true end )
   vn.na(_([[You shoot angrily at the annoying hatch for interrupting your nothingness, leaving large blast marks.]]))
   voice(_([["Hey! Do you know how hard it is to get a decent hatch here!?" yells a little voice from behind the damaged hatch.]]))
   vn.jump("02_cont")

   vn.label("02_cont")
   vn.menu( function ()
      local opts = {
         {_([["Step out or you're next!"]]), "02_stepout"},
      }
      if not shot then
         table.insert( opts, 1, {_([["Who are you?"]]), "02_who"} )
      end
      return opts
   end )

   vn.label("02_who")
   voice(_([["Go away! I'm not here"]]))
   vn.menu{
      {_([[Shoot at the hatch.]]), "02_shoot"},
      {_([[Open the hatch.]]), "02_open"},
   }

   vn.label("02_stepout")
   voice(_([["What are you going to do? Shoot me?"]]))
   vn.na( function ()
      if shot then
         return _([[You shoot the hatch more for dramatic effect.]])
      else
         return _([[You shoot at the hatch.]])
      end
   end )
   vn.na(_([[The hatch begrudgingly opens.]]))
   vn.jump("02_done")

   vn.label("02_open")
   vn.na(_([[The hatch easily opens, beckoning you in, away from the dull landscape.]]))
   vn.jump("02_done")

   vn.label("02_done")
   vn.na(_([[You walk through a hallway that seems to be acting as a makeshift pressure lock and make your way to what seems to be a makeshift drug lab, most likely highly illegal.]]))

   vn.scene()
   local dude = vn.newCharacter( _("???"), { image=image } )
   vn.transition()
   dude(_([["Don't shoot! I'm unarmed!"
You see a shady character with clearly bloodshot eyes.]]))
   vn.menu{
      {_([[Shoot to kill.]]), "kill"},
      {_([[Shoot at the roof.]]), "03_roof"},
      {_([["What do you have here?"]]), "04_what"},
   }

   vn.label("03_roof")
   dude( function ()
      if shot then
         return _([["Shit, soul! You sure like shooting everything! Don't shoot!"]])
      else
         return _([["Shit, soul! You're going to wreck the place!"]])
      end
   end )
   dude(_([["I'm just trying to make a livin' soul. Gotta pay them bills. Not like I'm terrorizing anyone."]]))
   vn.menu{
      {_([["What are you making?"]]), "04_what"},
      {_([["What's this operation?"]]), "04_what"},
   }

   vn.label("04_what")
   dude(fmt.f(_([["I'me just making some {drug}, mainly for meself, but sometimes some pirates come and buy some."]]), {drug="#b"..tostring(DRUG).."#0"}))
   dude(_([["I just want to get away from it all, be one with the universe, in harmony with all."]]))
   vn.menu{
      {_([["Neblaze?"]]), "05_neblaze"},
      {_([["You have got to stop making drugs."]]), "05_stop"},
   }

   vn.label("05_neblaze")
   dude(_([[Their bloodshot eyes widen a bit.
"It expands your mind, soul. It's like your mind blends into the mesmerizing nebula patterns, grey matter and space united!"]]))
   dude(_([["You've got to try this, soul!"]]))
   vn.menu{
      {_([["Just a bit."]]), "05_try"},
      {p_("Alsafi Druglab", [["No."]]), "05_notry"},
      {_([["You have got to stop making drugs."]]), "05_stop"},
   }

   --local tried = false
   vn.label("05_try")
   --vn.func( function ()
   --   tried = true
   --end )
   vn.na(_([[You take a bit with your free hand and put it under your tongue. It seems to fizzle and pop, and the world wobbles a bit. This stuff is stronger than you expected.]]))
   vn.jump("the_choice")

   vn.label("05_notry")
   vn.na(_([[You refuse to have anything to do with the suspicious psychadelics.]]))
   dude(_([["Your loss, soul. It would help you loosen up."]]))
   vn.jump("the_choice")

   vn.label("the_choice")
   vn.menu{
      {_([[Cut a deal.]]), "05_deal"},
      {_([[Stop them from making more drugs.]]), "05_stop_def"},
      {_([[Shoot them and put a stop to this.]]), "kill"},
   }

   vn.label("05_stop")
   dude(_([["I'm not hurting anyone, soul. I just want to leave in peace, be one among the stars, soul."]]))
   vn.jump("the_choice")

   vn.label("05_stop_def")
   vn.func( function ()
      result = "destroyed"
   end )
   vn.na(_([[You tell them they have to voluntarily stop making drugs, or you'll make sure they won't be able to.]]))
   vn.na(_([[You watch and make sure they leave with their meagre belongings. As you do a last pass of the lab before getting rid of it, you notice some encrypted data matrices, likely something they didn't need, you take them with you.]]))
   vn.na(_([[Finally, you torch the lab to make sure no new drugs can be made on Alsafi II, however, you have the sinking feeling they'll just do it somewhere else, but that's not your problem any more.]]))
   local poi_reward = poi.data_str(2)
   vn.na(fmt.reward(poi_reward))
   vn.func( function ()
      poi.data_give(2)
   end )
   vn.done()

   vn.label("05_deal")
   vn.func( function ()
      result = "kept"
   end )
   vn.na(fmt.f(_([[You bargain with them and strike a deal to be able to purchase {drug} at a bargain.]]),
      {drug=DRUG}))
   vn.done()

   vn.label("kill")
   vn.func( function ()
      result = "killed"
   end )
   vn.na(_([[You shoot, landing a clean shot, and they crumple to the floor. With your weapon still ready, you get close to the body and confirm a lack of pulse. Looks like they won't be flooding the market with any more illegal drugs.]]))
   vn.na(_([[You look around the lab and are able to scrounge up some credits.]]))
   vn.func( function ()
      player.pay( KILL_MONEY )
   end )
   vn.sfxMoney()
   vn.na( fmt.reward(KILL_MONEY) )
   vn.done()

   vn.run()

   -- Nothing happened, boring
   if result==nil then
      return evt.finish(false)
   end

   -- Handle cases
   var.push( VAR, result )
   if result == "destroyed" then
      neu.addMiscLog(fmt.f(_([[You found an illegal drug lab on {spob}, which you shut down without bloodshed, preventing illegal drugs from flowing out. Out of the ordeal, you were able to get {reward}.]]),
         {spob=spob.cur(), reward=poi_reward}))
   elseif result == "killed" then
      neu.addMiscLog(fmt.f(_([[You found an illegal drug lab on {spob}. You killed the criminal, putting an end to illegal drugs flowing into civilized space.]]),
         {spob=spob.cur()}))
   elseif result == "kept" then
      diff.apply( "Alsafi II Druglab" )
      neu.addMiscLog(fmt.f(_([[You found an illegal drug lab on {spob}. Not wanting to be left out in the action, you struck a bargain to be able to {drug} at a bargain price.]]),
         {spob=spob.cur(), drug=DRUG}))
   end

   evt.finish(true)
end
