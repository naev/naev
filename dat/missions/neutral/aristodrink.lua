--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Drinking Aristocrat">
 <unique />
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

  Drinking Aristocrat
  Author: geekt
  Idea from todo list.

  An aristocrat wants a specific drink which he recalls from a certain planet and will pay handsomely if you bring it. When you get to said planet it turns out the drink isn't served there but the bartender gives you a hint. You are then hinted until you end up getting the drink and bringing it back.

Thank you to Bobbens, Deiz, BTAxis, and others that have helped me with learning to use Lua and debugging my scripts in the IRC channel. Thanks as well to all those that have contributed to Naev and made it as great as it is today, and continue to make it better every day.

]]--
local fmt = require "format"
local neu = require "common.neutral"
local lmisn = require "lmisn"
local vn = require "vn"
local vntk = require "vntk"
local portrait = require "portrait"

local npc_name = _("Drinking Aristocrat")
local npc_portrait = "neutral/unique/aristocrat.webp"
local npc_image = portrait.getFullPath( npc_portrait )

local payment = 200e3
local getclueplanet, isPrevPlanet -- Forward-declared functions

-- defines Previous Planets table
mem.prevPlanets = {}


local moreinfotxt = {}
moreinfotxt[1] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where they work now, but they started working at the bar on {pnt} in the {sys} system after they left here. Good luck!" With high hopes, you decide to head off to there.]])
moreinfotxt[2] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where he works now, but he started working at the bar on {pnt} in the {sys} system after he left here. Good luck!" With high hopes, you decide to head off to there.]])
moreinfotxt[3] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where she works now, but she started working at the bar on {pnt} in the {sys} system after she left here. Good luck!" With high hopes, you decide to head off to there.]])

local worktxt = {}
worktxt[1] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be them. You start to describe the drink to them and they interrupt. "A Swamp Bombing. Of course, that's my specialty." You ask if they can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as they're about to start making it, though, you stop them and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully their trained hands move, flipping bottles and shaking various containers. Before you know it, they've set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])
worktxt[2] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be him. You start to describe the drink to him and he interrupts. "A Swamp Bombing. Of course, that's my specialty." You ask if he can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as he's about to start making it, though, you stop him and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully his trained hands move, flipping bottles and shaking various containers. Before you know it, he's set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])
worktxt[3] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be her. You start to describe the drink to her and she interrupts. "A Swamp Bombing. Of course, that's my specialty." You ask if she can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as she's about to start making it, though, you stop her and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully her trained hands move, flipping bottles and shaking various containers. Before you know it, she's set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])

function create ()
   -- Note: this mission does not make any system claims.

   -- creates the NPC at the bar to create the mission
   misn.setNPC( npc_name, npc_portrait, _("You see an aristocrat sitting at a table in the middle of the bar, drinking a swirling concoction in a martini glass with a disappointed look on his face every time he takes a sip.") )

   mem.startplanet, mem.startsys = spob.cur()
   mem.prevPlanets[1] = mem.startplanet
   mem.numjumps = 0

   -- chooses the planet
   mem.clueplanet, mem.cluesys = getclueplanet(1, 3)
   mem.prevPlanets[#mem.prevPlanets+1] = mem.clueplanet
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local ari = vn.newCharacter( npc_name, { image=npc_image } )
   vn.transition()

   ari(fmt.f( _([[You begin to approach the aristocrat. Next to him stands a well dressed and muscular man, perhaps his assistant, or maybe his bodyguard, you're not sure. When you get close to his table, he begins talking to you as if you work for him. "This simply will not do. When I ordered this 'drink', if you can call it that, it seemed interesting. It certainly doesn't taste interesting. It's just bland. The only parts of it that are in any way interesting are not at all pleasing. It just tastes so… common.
    You know what I would really like? There was this drink at a bar on, what planet was that? Damien, do you remember? The green drink with the red fruit shavings." Damien looks down at him and seems to think for a second before shaking his head. "I believe it might have been {pnt} in the {sys} system. The drink was something like an Atmospheric Re-Entry or Gaian Bombing or something. It's the bar's specialty. They'll know what you're talking about. You should go get me one. Can you leave right away?"]]),
      {pnt=mem.clueplanet, sys=mem.cluesys}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   ari(_([["What do you mean, you can't leave right away? Then why even bother? Remove yourself from my sight." The aristocrat makes a horrible face, and sips his drink, only to look even more disgusted. Ignoring you, he puts his drink back on the table and motions to the bartender.]]))
   vn.done()

   vn.label("accept")
   ari(_([["Oh good! Of course you will be paid handsomely for your efforts. I trust you can figure out how to get it here intact on your own." Ignoring you, the aristocrat goes back to sipping his drink, making an awful face every time he tastes it. You walk away, a bit confused.]]))
   vn.func(function () accepted = true end )
   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   mem.landmarker = misn.markerAdd( mem.clueplanet, "low" )

   -- mission details
   misn.setTitle( _("Drinking Aristocrat") )
   misn.setReward( _("He will pay handsomely.") )
   misn.setDesc( _("Go find a specific drink for an aristocrat.") )

   -- how many systems you'll have to run through
   mem.numclues = rnd.rnd(1,5)
   mem.numexwork = rnd.rnd(1,3)

   -- final bartender data
   mem.fintendergen = rnd.rnd(1,3)

   -- hooks
   mem.landhook = hook.land ("land", "bar")
   mem.takeoffhook = hook.takeoff ("takeoff")
end

function land ()
   if spob.cur() == mem.clueplanet then
      if mem.numclues > 0 then   -- first clue
         mem.numclues = mem.numclues - 1
         mem.numjumps = mem.numjumps + 1

         -- next planet
         mem.clueplanet, mem.cluesys = getclueplanet(1, 3)
         misn.markerMove( mem.landmarker, mem.clueplanet )
         mem.prevPlanets[#mem.prevPlanets+1] = mem.clueplanet

         vntk.msg( _("Clue"), fmt.f( _([[You walk into the bar and approach the bartender. You describe the drink, but the bartender doesn't seem to know what you're talking about. There is another bartender that they think may be able to help you though, at {pnt} in the {sys} system.]]),
            {pnt=mem.clueplanet, sys=mem.cluesys} ) )

      else
         if not mem.foundexwork then   -- find out that it's a bartender's specialty
            mem.foundexwork = true
            mem.numexwork = mem.numexwork - 1
            mem.numjumps = mem.numjumps + 1

            -- next planet
            mem.clueplanet, mem.cluesys = getclueplanet(1, 5)
            misn.markerMove( mem.landmarker, mem.clueplanet )
            mem.prevPlanets[#mem.prevPlanets+1] = mem.clueplanet

            vntk.msg( _("A bit more info…"), fmt.f( moreinfotxt[mem.fintendergen],
               {pnt=mem.clueplanet, sys=mem.cluesys} ) )

         else   -- find another bar that the bartender used to work at
            if mem.numexwork > 0 then
               mem.numexwork = mem.numexwork - 1

               -- next planet
               mem.clueplanet, mem.cluesys = getclueplanet(1, 5)
               misn.markerMove( mem.landmarker, mem.clueplanet )
               mem.prevPlanets[#mem.prevPlanets+1] = mem.clueplanet

               vntk.msg( _("Is this it?"), fmt.f( _([[You walk into the bar fully confident that this is the bar. You walk up to the bartender and ask for a Swamp Bombing. "A wha???" Guess this isn't the right bar. You get another possible clue, {pnt} in the {sys} system, and head on your way.]]),
                  {pnt=mem.clueplanet, sys=mem.cluesys} ) )

            elseif not mem.hasDrink then  -- get the drink
               mem.hasDrink = true

               vntk.msg( _("This is it!"), worktxt[mem.fintendergen] )

               misn.markerMove(mem.landmarker, mem.startplanet)
            end
         end
      end
   elseif mem.hasDrink and spob.cur() == mem.startplanet then
      lmisn.sfxVictory()

      local reward = outfit.get("Swamp Bombing")

      vn.clear()
      vn.scene()
      local ari = vn.newCharacter( npc_name, { image=npc_image } )
      vn.transition()

      ari(fmt.f(_([["Ahh! I was just thinking how much I wanted one of those drinks! I'm so glad that you managed to find it. You sure seemed to take your time though." You give him his drink and tell him that it wasn't easy, and how many systems you had to go through. "Hmm. That is quite a few systems. No reason for you to be this late though." He takes a sip from his drink. "Ahh! That is good though. I suppose you'll be wanting to get paid for your efforts. You did go through a lot of trouble. Then again, you did take quite a long time. I suppose {credits} should be appropriate."]]),
         {credits=fmt.credits(payment)}))
      vn.na(_([[Considering the amount of effort that you went through, you feel almost cheated. You don't feel like arguing with the snobby aristocrat though, so you just leave him to his drink without another word. It's probably the most that anyone's ever paid for a drink like that anyway.
    When you get back to your ship you realize you have a drink left over. It might look good like an ornament?]]))
      vn.sfxVictory()
      vn.func( function ()
         player.outfitAdd(reward)
         player.pay( payment )
      end )
      vn.na(fmt.reward(payment).."\n"..fmt.reward(reward))
      vn.run()

      hook.rm(mem.landhook)
      hook.rm(mem.takeoffhook)
      neu.addMiscLog( _([[You delivered a special drink called a Swamp Bombing to an aristocrat.]]) )
      misn.finish( true )
   end
end

function getclueplanet ( mini, maxi )
   local planets = {}

   lmisn.getSysAtDistance( system.cur(), mini, maxi,
      function(s)
         for i, v in ipairs(s:spobs()) do
            if not isPrevPlanet(v) and v:services()["bar"] and v:canLand() then
               planets[#planets + 1] = {v, s}
            end
         end
      return false
   end )
   if #planets == 0 then abort() end
   local index = rnd.rnd(1, #planets)

   return planets[index][1], planets[index][2]
end

function isPrevPlanet ( passedPlanet )
   for i = 1, #mem.prevPlanets, 1 do
      if mem.prevPlanets[i] == passedPlanet then
         return true
      end
   end
end

function takeoff ()
   if mem.hasDrink then
      misn.osdCreate( _("Find the Drink"), {
         fmt.f( _("Return the drink to the Aristocrat at {pnt} in the {sys} system."), {pnt=mem.startplanet, sys=mem.startsys} )
      } )
   else
      misn.osdCreate( _("Find the Drink"), {
         fmt.f( _("Go to {pnt} in the {sys} system and look for the special drink that the Aristocrat wants"), {pnt=mem.clueplanet, sys=mem.cluesys} )
      } )
   end
end

function abort()
   misn.finish()
end
