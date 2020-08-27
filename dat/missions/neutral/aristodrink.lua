--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Drinking Aristocrat">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>5</chance>
   <location>Bar</location>
  </avail>
 </mission>
 --]]
--[[

  Drinking Aristocrat
  Author: geekt
  Idea from todo list.

  An aristocrat wants a specific drink which he recalls from a certain planet and will pay handsomely if you bring it. When you get to said planet it turns out the drink isn't served there but the bartender gives you a hint. You are then hinted until you end up getting the drink and bringing it back.

Thank you to Bobbens, Deiz, BTAxis, and others that have helped me with learning to use Lua and debugging my scripts in the IRC channel. Thanks as well to all those that have contributed to Naev and made it as great as it is today, and continue to make it better every day.

]]--

require "jumpdist.lua"
require "numstring.lua"
require "dat/missions/neutral/common.lua"


bar_desc = _("You see an aristocrat sitting at a table in the middle of the bar, drinking a swirling concoction in a martini glass with a disappointed look on his face every time he takes a sip.")

-- Mission Details
misn_title = _("Drinking Aristocrat")
misn_reward = _("He will pay handsomely.")
misn_desc = _("Go find a specific drink for an aristocrat.")

-- OSD
OSDtitle = _("Find the Drink")
OSDdesc = _("Go to %s in the %s system and look for the special drink that the Aristocrat wants")
OSDtable = {}

-- defines Previous Planets table
prevPlanets = {}
prevPlanets.__save = true

payment = 200000

title = {}  --stage titles
text = {}   --mission text

title[1] = _("Drinking Aristocrat")
text[1] = _([[You begin to approach the aristocrat. Next to him stands a well dressed and muscular man, perhaps his assistant, or maybe his bodyguard, you're not sure. When you get close to his table, he begins talking to you as if you work for him. "This simply will not do. When I ordered this 'drink', if you can call it that, it seemed interesting. It certainly doesn't taste interesting. It's just bland. The only parts of it that are in any way interesting are not at all pleasing. It just tastes so, common.
    You know what I would really like? There was this drink at a bar on, what planet was that? Damien, do you remember? The green drink with the red fruit shavings." Damien looks down at him and seems to think for a second before shaking his head. "I believe it might have been %s in the %s system. The drink was something like an Atmospheric Re-Entry or Gaian Bombing or something. It's the bar's specialty. They'll know what you're talking about. You should go get me one. Can you leave right away?"]])

title[2] = _("Leave Immediately")
text[2] = _([["Oh good! Of course you will be paid handsomely for your efforts. I trust you can figure out how to get it here intact on your own." The aristocrat goes back to sipping his drink, making an awful face every time he tastes it, ignoring you. You walk away, a bit confused.]])

title[3] = _("Refuse")
text[3] = _([["What do you mean, you can't leave right away? Then why even bother? Remove yourself from my sight." The aristocrat makes a horrible face, and sips his drink, only to look even more disgusted. He puts his drink back on the table and motions to the bartender, ignoring you beyond now.]])

cluetitle = _("Clue")
cluetxt = _([[You walk into the bar and approach the bartender. You describe the drink, but the bartender doesn't seem to know what you're talking about. There is another bartender that they think may be able to help you though, at %s in the %s system.]])

moreinfotitle = _("A bit more info...")
moreinfotxt = {}
moreinfotxt[1] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where they work now, but they started working at the bar on %s in the %s system after they left here. Good luck!" With high hopes, you decide to head off to there.]])
moreinfotxt[2] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where he works now, but he started working at the bar on %s in the %s system after he left here. Good luck!" With high hopes, you decide to head off to there.]])
moreinfotxt[3] = _([[You walk in and see someone behind the bar. When you approach and describe the drink, they tell you that the drink isn't the specialty of any one bar, but actually the specialty of a bartender who used to work here. "It's called a Swamp Bombing. I don't know where she works now, but she started working at the bar on %s in the %s system after she left here. Good luck!" With high hopes, you decide to head off to there.]])

exworktitle = _("Is this it?")
exworktxt = _([[You walk into the bar fully confident that this is the bar. You walk up to the bartender and ask for a Swamp Bombing. "A wha???" Guess this isn't the right bar. You get another possible clue, %s in the %s system, and head on your way.]])

worktitle = _("This is it!")
worktxt = {}
worktxt[1] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be them. You start to describe the drink to them and they interrupt. "A Swamp Bombing. Of course, that's my specialty." You ask if they can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as they're about to start making it, though, you stop them and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully their trained hands move, flipping bottles and shaking various containers. Before you know it, they've set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])
worktxt[2] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be him. You start to describe the drink to them and he interrupts. "A Swamp Bombing. Of course, that's my specialty." You ask if he can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as he's about to start making it, though, you stop him and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully his trained hands move, flipping bottles and shaking various containers. Before you know it, he's set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])
worktxt[3] = _([[You walk into the bar and know instantly that you are finally here! This is the place! You walk up to the bartender, who smiles. This has to be her. You start to describe the drink to her and she interrupts. "A Swamp Bombing. Of course, that's my specialty." You ask if she can make it to go, prompting a bit of a chuckle. "Sure, why not?"
    Just as she's about to start making it, though, you stop her and say you'll have one here after all. As long as you've come all this way, you might as well try it. You're amazed at how quickly and gracefully her trained hands move, flipping bottles and shaking various containers. Before you know it, she's set a drink before you and closed another container to take with you. You taste it expecting something incredible. It's alright, but you doubt it was worth all this trouble.]])

finishedtitle = _("Delivery")
finishedtxt = _([["Ahh! I was just thinking how much I wanted one of those drinks! I'm so glad that you managed to find it. You sure seemed to take your time though." You give him his drink and tell him that it wasn't easy, and how many systems you had to go through. "Hmm. That is quite a few systems. No reason for you to be this late though." He takes a sip from his drink. "Ahh! That is good though. I suppose you'll be wanting to get paid for your troubles. You did go through a lot of trouble. Then again, you did take quite a long time. I suppose %s credits should be appropriate."
    Considering the amount of effort that you went through, you feel almost cheated. You don't feel like arguing with the snobby aristocrat though, so you just leave him to his drink without another word. It's probably the most that anyone's ever paid for a drink like that anyway.]])

log_text = _([[You delivered a special drink called a Swamp Bombing to an aristocrat.]])


function create ()
   -- Note: this mission does not make any system claims.

   -- creates the NPC at the bar to create the mission
   misn.setNPC( _("Drinking Aristocrat"), "neutral/unique/aristocrat" )
   misn.setDesc( bar_desc )

   startplanet, startsys = planet.cur()

   prevPlanets[1] = startplanet
   prevPlanets.__save = true

   numjumps = 0

   -- chooses the planet
   clueplanet, cluesys = getclueplanet(1, 3)
   prevPlanets[#prevPlanets+1] = clueplanet
end

function accept ()
   if not tk.yesno( title[1], text[1]:format( clueplanet:name(), cluesys:name() ) ) then
      tk.msg( title[3], text[3] )
      misn.finish()

   else
      misn.accept()

      landmarker = misn.markerAdd( cluesys, "low" )

      -- mission details
      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc )

      tk.msg( title[2], text[2] )

      -- how many systems you'll have to run through
      numclues = rnd.rnd(1,5)
      numexwork = rnd.rnd(1,3)

      -- final bartender data
      fintendergen = rnd.rnd(1,3)

      -- hooks
      landhook = hook.land ("land", "bar")
      takeoffhook = hook.takeoff ("takeoff")
   end
end

function land ()
   if planet.cur() == clueplanet then
      if numclues > 0 then   -- first clue
         numclues = numclues - 1
         numjumps = numjumps + 1

         -- next planet
         clueplanet, cluesys = getclueplanet(1, 3)
         misn.markerMove( landmarker, cluesys )
         prevPlanets[#prevPlanets+1] = clueplanet

         tk.msg( cluetitle, cluetxt:format(
            clueplanet:name(), cluesys:name() ) )

      else
         if not foundexwork then   -- find out that it's a bartender's specialty
            foundexwork = true
            numexwork = numexwork - 1
            numjumps = numjumps + 1

            -- next planet
            clueplanet, cluesys = getclueplanet(1, 5)
            misn.markerMove( landmarker, cluesys )
            prevPlanets[#prevPlanets+1] = clueplanet

            tk.msg( moreinfotitle, moreinfotxt[fintendergen]:format(
               clueplanet:name(), cluesys:name() ) )

         else   -- find another bar that the bartender used to work at
            if numexwork > 0 then
               numexwork = numexwork - 1

               -- next planet
               clueplanet, cluesys = getclueplanet(1, 5)
               misn.markerMove( landmarker, cluesys )
               prevPlanets[#prevPlanets+1] = clueplanet

               tk.msg( exworktitle, exworktxt:format(
                  clueplanet:name(), cluesys:name() ) )

            elseif not hasDrink then  -- get the drink
               hasDrink = true

               tk.msg( worktitle, worktxt[fintendergen] )

               misn.markerMove(landmarker, startsys)
            end
         end
      end
   elseif hasDrink and planet.cur() == startplanet then
      tk.msg( finishedtitle, finishedtxt:format( numstring(payment) ) )
      player.pay( payment )

      hook.rm(landhook)
      hook.rm(takeoffhook)
      addMiscLog( log_text )
      misn.finish( true )
   end
end

function getclueplanet ( mini, maxi )
   local planets = {}

   getsysatdistance( system.cur(), mini, maxi,
      function(s)
         for i, v in ipairs(s:planets()) do
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
   for i = 1, #prevPlanets, 1 do
      if prevPlanets[i]:name() == passedPlanet:name() then
         return true
      end
   end
end

function takeoff ()
   if hasDrink then
      OSDdesc = _("Return the drink to the Aristocrat at %s in the %s system.")
      OSDtable[1] = OSDdesc:format( startplanet:name(), startsys:name() )
      misn.osdCreate( OSDtitle, OSDtable )
   else
      OSDtable[1] = OSDdesc:format( clueplanet:name(), cluesys:name() )
      misn.osdCreate( OSDtitle, OSDtable )
   end
end

function abort()
   misn.finish()
end
