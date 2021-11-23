--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Search for Cynthia">
 <flags>
   <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>The Runaway</done>
  <chance>11</chance>
  <location>Bar</location>
  <system>Goddard</system>
 </avail>
</mission>
--]]
--[[
   This is the second half of "The Runaway"
   Here, Cynthia's father pays you to track down his daughter.
   It is alluded to that Cynthia ran away due to her abusive mother.
   The father has been named after me, Old T. Man.
   I'm joking about the last line a little. If you want to name him, feel free.
--]]

local fmt = require "format"
local neu = require "common.neutral"

local releasereward = 25e3
local reward = 300e3

-- Mission constants
local cargoname = N_("Cynthia")
local cargodesc = N_("A young teenager.")
local targetworld = planet.get("Niflheim")

-- luacheck: globals land (Hook functions passed by name)

-- Here are stored the fake texts for the OSD
mem.osd_text = {}
mem.osd_text[1] = _("Search for Cynthia on Niflheim in Dohriabi")
mem.osd_text[2] = _("Search for Cynthia on Nova Shakar in Shakar")
mem.osd_text[3] = _("Search for Cynthia on Selphod in Eridani")
mem.osd_text[4] = _("Search for Cynthia on Emperor's Fist in Gamma Polaris")

-- Can't let them see what's coming up, can I?

function create ()
   misn.setNPC( _("Old Man"), "neutral/unique/cynthia_father.webp", _("An old man sits at a table with some missing person papers.") )
end

function accept ()
   --This mission does not make any system claims
   if not tk.yesno( fmt.f(_("The Search for Cynthia"), _([[Approaching him, he hands you a paper. It offers a {credits} reward for the finding of a "Cynthia" person.
    "That's my girl. She disappeared quite a few decaperiods ago. We managed to track her down to here, but where she went afterwards remains a mystery. We know she was kidnapped, but if you know anything..." The man begins to cry. "Have you seen any trace of her?"]]),{credits=fmt.credits(reward)})) then
      misn.finish()
   end

   --Set up the OSD
   if misn.accept() then
      misn.osdCreate(_("The Search for Cynthia"), mem.osd_text)
      misn.osdActive(1)
   end

   misn.setTitle( _("The Search for Cynthia") )
   misn.setReward( fmt.f( _("{credits} on delivery."), {credits=fmt.credits(reward)} ) )

   misn.setDesc( _("Search for Cynthia.") )
   mem.runawayMarker = misn.markerAdd( targetworld, "low" )

   tk.msg( _("The Search for Cynthia"), _([[Looking at the picture, you see that the locket matches the one that Cynthia wore, so you hand it to her father. "I believe that this was hers." Stunned, the man hands you a list of planets that they wanted to look for her on.]]) )

   hook.land("land")
end

function land ()
   -- Only proceed if at the target.
   if planet.cur() ~= targetworld then
      return
   end

   --If we land on Niflheim, display message, reset target and carry on.
   if planet.cur() == planet.get("Niflheim") then
      targetworld = planet.get("Nova Shakar")
      tk.msg(_("The Search for Cynthia"), _("After thoroughly searching the spaceport, you decide that she wasn't there."))
      misn.osdActive(2)
      misn.markerMove(mem.runawayMarker, targetworld)

   --If we land on Nova Shakar, display message, reset target and carry on.
   elseif planet.cur() == planet.get("Nova Shakar") then
      targetworld = planet.get("Torloth")
      tk.msg(_("The Search for Cynthia"), _("At last! You find her, but she ducks into a tour bus when she sees you. The schedule says it's destined for Torloth. You begin to wonder if she'll want to be found."))

      --Add in the *secret* OSD text
      mem.osd_text[3] = _("Catch Cynthia on Torloth in Cygnus")
      mem.osd_text[4] = _("Return Cynthia to her father on Zhiru in the Goddard system")

      --Update the OSD
      misn.osdDestroy()
      misn.osdCreate(_("The Search for Cynthia"), mem.osd_text)
      misn.osdActive(3)

      misn.markerMove(mem.runawayMarker, targetworld)

   --If we land on Torloth, change OSD, display message, reset target and carry on.
   elseif planet.cur() == planet.get("Torloth") then
      targetworld = planet.get("Zhiru")

      --If you decide to release her, speak appropriately, otherwise carry on
      if not tk.yesno(_("The Search for Cynthia"), _([[After chasing Cynthia through most of the station, you find her curled up at the end of a hall, crying. As you approach, she screams, "Why can't you leave me alone? I don't want to go back to my terrible parents!" Will you take her anyway?]])) then
         mem.osd_text[4] = _("Go to Zhiru in Goddard to lie to Cynthia's father")
         tk.msg(_("The Search for Cynthia"), _([["Please, please, please don't ever come looking for me again, I beg of you!"]]))
      else
         tk.msg(_("The Search for Cynthia"), _([[Cynthia stops crying and proceeds to hide in the farthest corner of your ship. Attemps to talk to her turn up fruitless.]]))
         local c = misn.cargoNew( cargoname, cargodesc )
         mem.cargoID = misn.cargoAdd( c, 0 )
      end

      --Update the OSD
      misn.osdDestroy()
      misn.osdCreate(_("The Search for Cynthia"), mem.osd_text)
      misn.osdActive(4)

      misn.markerMove(mem.runawayMarker, targetworld)

   --If we land on Zhiru to finish the mission, clean up, reward, and leave.
   elseif planet.cur() == planet.get("Zhiru") then

      --Talk to the father and get the reward
      if misn.osdGetActive() == _("Return Cynthia to her father on Zhiru in the Goddard system") then
         tk.msg(_("The Search for Cynthia"), _("As Cynthia sees her father, she begins her crying anew. You overhear the father talking about how her abusive mother died. Cynthia becomes visibly happier, so you pick up your payment and depart."))
         player.pay(reward)
         misn.cargoRm(mem.cargoID)
         neu.addMiscLog( _([[The father of Cynthia, who you had given a lift before, asked you to find her and bring her back to him, thinking that she was kidnapped. Cynthia protested, telling you that she did not want to go back to her parents, but you took her anyway. When she saw her father, she started crying, but seemed to become visibly happier when her father told her that her abusive mother had died.]]) )
      else
         tk.msg(_("The Search for Cynthia"), _([[You tell the father that you checked every place on the list, and then some, but his daughter was nowhere to be found. You buy the old man a drink, then go back to the spaceport. Before you leave, he hands you a few credits. "For your troubles."]]))
         player.pay(releasereward)
         neu.addMiscLog( _([[The father of Cynthia, who you had given a lift before, asked you to find her and bring her back to him, thinking that she was kidnapped. Cynthia protested, telling you that she did not want to go back to her parents. Respecting her wishes, you let her be and lied to her father, saying that you couldn't find her no matter how hard you tried.]]) )
      end

      misn.finish(true)
   end
end
