--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Epilogue 2">
 <unique/>
 <location>land</location>
 <chance>100</chance>
 <spob>New Haven</spob>
 <cond>player.misnDone("Minerva Finale 2")</cond>
 <notes>
  <campaign>Minerva</campaign>
  <done_misn name="Minerva Finale 2" />
 </notes>
</event>
--]]
--[[
   Event handling the gambling stuff going on at Minerva station
--]]
local fmt = require "format"
local minerva = require 'common.minerva'
local vn = require 'vn'

local reward = outfit.get("Energy Harpoon Prototype")

function create ()
   evt.npcAdd("approach", _("Maikki and Kex"), minerva.maikkiP.portrait, _("You see Maikki and Kex chilling at the bar.") )
   hook.takeoff("leave")
end

function approach ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikkiP{pos="left"} )
   local kex = vn.newCharacter( minerva.vn_kexP{pos="right"} )
   vn.music( minerva.loops.maikki )
   vn.transition("hexagon")

   vn.na(_([[You find Kex and Maikki sharing a table. Looks like they are getting along.]]))
   maikki(fmt.f(_([["Hey {playername}! Long time no see! How have you been?"
She seems a tad tipsy.]]),
      {playername=player.name()}))
   kex(_([["Hey, you're back!"]]))
   vn.menu{
      {_([["Great, and you?"]]),"01_great"},
      {_([["Could be better."]]),"01_bad"},
      {_([["Arrrr!"]]),"01_arrr"},
   }

   vn.label("01_great")
   kex(_([["Glad to hear that!"]]))
   vn.jump("01_cont")

   vn.label("01_bad")
   kex(_([["We need the bad days to appreciate the good ones."]]))
   vn.jump("01_cont")

   vn.label("01_arrr")
   local arr01 = false
   vn.func( function () arr01 = true end )
   maikki(_([["That's the spirit!"]]))
   kex(_([[Kex tries his most ferocious appearance.
"AarrrrrrrrrrR!"]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   maikki(_([["I have to thank you again, it's been great catching up with my father. We've been sharing anecdotes about our adventures."]]))
   kex(_([["I was quite adventurous in my day, but damn, pirate life is something else. Maikki has pulled of some serious shit!"]]))
   maikki(_([["Oh stop, I've never piloted a ship using a modified hacked mace rocket engine with no life support in a collapsing nebula pocket with a bunch of cannibal raiders trying to break through the hull with power drills!"]]))
   kex(_([["Ah, I can never forget that one! I never thought I'd make it alive!"]]))
   vn.menu{
      {_([["Wait, for real?"]]),"02_real"},
      {_([["There are cannibal raiders in the Nebula?"]]),"02_cannibals"},
      {_([["I see."]]),"02_noreaction"},
   }

   vn.label("02_real")
   maikki(_([["For real!"]]))
   kex(_([["Well, I may have embellished things a bit, but there really were cannibals trying to break into my ship!"]]))
   maikki(_([["Why were there cannibal raiders in the Nebula?"]]))
   vn.jump("02_cont")

   vn.label("02_cannibals")
   maikki(_([["I'm also surprised by that. I mean, who hasn't resorted to a bit of cannibalism when times get tough, but being full out cannibalism? Feels like something out of a holovid!"]]))
   kex(_([["Resorted to cannibalism? Wait, what do you mean?"]]))
   maikki(_([["Umm, er. Why were there cannibal raiders in the Nebula?"]]))
   vn.jump("02_cont")

   vn.label("02_noreaction")
   kex(_([["What, no reaction?"]]))
   maikki(_([["Poor dad, you'll make him sad! Tell me again, why were there cannibal raiders in the Nebula?"]]))
   vn.jump("02_cont")

   vn.label("02_cont")
   kex(_([["Ah. It's the Nebula itself you see? I think it can interact with your brainwaves or something. Sometimes you see things that aren't there, and that can drive some people off the edge."]]))
   maikki(_([["What do you mean off the edge?"]]))
   kex(_([["Like mad, crazy, bonkers."]]))
   maikki(_([["Wait, isn't that quite relative. I mean, I consider the Imperial bureaucracy and normal salaried life abnormal. I mean, it's such a droll! And, Suave Bill, he was called 'crazy' back in the Empire for his constant twitching, and look at him here, one of the best Hyena pilot! Those terms are offensive old man!"]]))
   kex(_([["I have no idea who Suave Bill is, but OK, so what would you call trying to bash your skull in and eat your brain with a spoon?"]]))
   maikki(_([["A bad day?"]]))
   kex(_([[Kex sighs.
"Well, some people in the deep Nebula, have bad days for the rest of their short lives!"]]))
   maikki(_([["I see, so some of them become cannibals or whatever?"]]))
   kex(_([["More than you'd expect! Got to be real careful in the Nebula. Never know when and who is going to be affected."]]))
   maikki(fmt.f(_([["Wait, before I forget, {playername}, remember the holodrives you took from Minerva Station?"]]),
      {playername=player.name()}))
   vn.menu{
      {_([["The ones with Kex's schematics on them?"]]),"03_kex"},
      {_([["What holodrives?"]]),"03_forgot"},
   }

   vn.label("03_kex")
   kex(_([["My schematics??"]]))
   maikki(_([["Yeah, those ones."]]))
   vn.jump("03_cont")

   vn.label("03_forgot")
   maikki(_([["Forgot so fast? You wouldn't have gotten a major concussion while piloting?"]]))
   vn.jump("03_cont")

   vn.label("03_cont")
   maikki(_([["Well, not only were we able to save Kex with them, but the Skull & Bones engineers were able to recover some interesting experimental weapon designs! We sold a couple to recoup costs on the entire Minerva Station incident, of course we kept a copy for ourselves."
She flashes a sly grin.]]))
   maikki(_([["It doesn't stop there though, the great gals and girls at Skull & Bones even implemented one of the designs, and solve some of the power regulation problems it had. Behold, the Energy Harpoon!"
She shows you a holographic projection of what seems to be a fancy-looking medium-sized weapon prototype.]]))
   vn.menu( function ()
      local opts = {
         {_([["What does it do?"]]),"04_cont"},
         {_([["Looks fancy!"]]),"04_cont"},
      }
      if arr01 then
         table.insert( opts, 1, {_([["Arrr!"]]),"04_arrr"} )
      end
      return opts
   end )

   vn.label("04_arrr")
   kex(_([[Kex follows your guide with another ferocious "Aaaarrraarrrr!"]]))
   maikki(_([["I take you like the sound of it? Harpoons do seem quite fitting for a pirate."]]))
   vn.jump("04_cont")

   vn.label("04_cont")
   maikki(_([["The design seems to be loosely based on a Heavy Ripper Cannon, however, they did something about reversing the fluctuations of something or other, and instead of knocking the target back, it actually pulls them in towards you, not letting them get away. A superb weapon indeed."]]))
   maikki(_([["I've tried it myself, and it really works quite well. The engineers have outdone themselves this time!"]]))
   kex(_([["She wanted to keep it for the Pink Demon, but I managed to convince her to give it to you, it's the least we could do."]]))
   maikki(_([["C'mon, you're not supposed to say that part aloud! It was a mutual agreement."]]))
   kex(_([["Quack."
He pecks at the ground.]]))
   maikki(_([["You're not going to fool anyone pretending to be a duck. Get back here asshole!"
She chases Kex around the room.]]))
   vn.na(_([[Looks like it's best for you to take your leave and let them bond or whatever a bit more.]]))

   -- Yeeeeeeah!
   vn.func( function ()
      player.outfitAdd( reward )
   end )
   vn.sfxVictory()
   vn.na(fmt.reward(reward))

   vn.done("hexagon")
   vn.run()

   evt.finish(true)
end

function leave ()
   evt.finish(false)
end
