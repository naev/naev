--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mission Template (mission name goes here)">
 <unique />
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
</mission>
--]]
--[[

   Mission Template (mission name goes here)

   This is a Naev mission template.
   This document aims to provide a structure on which to build many
   Naev missions and teach how to make basic missions in Naev.
   For more information on Naev, please visit: http://naev.org/
   Naev missions are written in the Lua programming language: http://www.lua.org/
   There is documentation on Naev's Lua API at: http://api.naev.org/
   You can study the source code of missions in [path_to_Naev_folder]/dat/missions/

   When creating a mission with this template, please erase the explanatory
   comments (such as this one) along the way, but retain the the MISSION and
   DESCRIPTION fields below, adapted to your mission.

   MISSION: <NAME GOES HERE>
   DESCRIPTION: <DESCRIPTION GOES HERE>

--]]

-- require statements go here. Most missions should include
-- "format", which provides the useful `number()` and
-- `credits()` functions. We use these functions to format numbers
-- as text properly in Naev. dat/scripts/common/neutral.lua provides
-- the addMiscLog function, which is typically used for non-factional
-- unique missions.
local fmt = require "format"
local neu = require "common.neutral"
local vntk = require "vntk"

--[[
Multi-paragraph dialog strings *can* go here, each with an identifiable
name. You can see here that we wrap strings that are displayed to the
player with `_()`. This is a call to gettext, which enables
localization. The _() call should be used directly on the string, as
shown here, instead of on a variable, so that the script which figures
out what all the translatable text is can find it.

When writing dialog, write it like a book (in the present-tense), with
paragraphs and quotations and all that good stuff. Leave the first
paragraph unindented, and indent every subsequent paragraph by four (4)
spaces. Use quotation marks as would be standard in a book. However, do
*not* quote the player speaking; instead, paraphrase what the player
generally says, as shown below.

In most cases, you should use double-brackets for your multi-paragraph
dialog strings, as shown below.

One thing to keep in mind: the player can be any gender, so keep all
references to the player gender-neutral. If you need to use a
third-person pronoun for the player, singular "they" is the best choice.

You may notice curly-bracketed {words} sprinkled throughout the text. These
are portions that will be filled in later by the mission via the
`fmt.f()` function.
--]]

-- Set some mission parameters.
-- For credit values in the thousands or millions, we use scientific notation (less error-prone than counting zeros).
-- There are two ways to set values usable from outside the create() function:
-- - Define them at file scope in a statement like "local credits = 250e3" (good for constants)
-- - Define them as fields of a special "mem" table: "mem.credits = 250e3" (will persist across games in the player's save file)
local misplanet, missys = spob.getS("Ulios")
local credits = 250e3

-- Here we use the `fmt.credits()` function to convert our credits
-- from a number to a string. This function both applies gettext
-- correctly for variable amounts (by using the ngettext function),
-- and formats the number in a way that is appropriate for Naev (by
-- using the numstring function). You should always use this when
-- displaying a number of credits.
local reward_text = fmt.credits( credits )

--[[
First you need to *create* the mission.  This is *obligatory*.

You have to set the NPC and the description. These will show up at the
bar with the character that gives the mission and the character's
description.
--]]
function create ()
   -- Naev will keep the contents of "mem" across games if the player saves and quits.
   -- Track mission state there. Warning: pilot variables cannot be saved.
   mem.talked = false

   -- If we needed to claim a system, we would do that here with
   -- something like the following commented out statement. However,
   -- this mission won't be doing anything fancy with the system, so we
   -- won't make a system claim for it.
   -- Only one mission or event can claim a system at a time. Using claims
   -- helps avoid mission and event collisions. Use claims for all systems
   -- you intend to significantly mess with the behaviour of.
   --if not misn.claim(missys) then misn.finish(false) end

   -- Give the name of the NPC and the portrait used. You can see all
   -- available portraits in dat/gfx/portraits.
   misn.setNPC( _("A well-dressed man"), "neutral/unique/youngbusinessman.webp", _("This guy is wearing a nice suit.") )
end


--[[
This is an *obligatory* part which is run when the player approaches the
character.

Run misn.accept() here to internally "accept" the mission. This is
required; if you don't call misn.accept(), the mission is scrapped.
This is also where mission details are set.
--]]
function accept ()
   -- Use different text if we've already talked to him before than if
   -- this is our first time.
   local text
   if mem.talked then
      -- We use `fmt.f()` here to fill in the destination and
      -- reward text. (You may also see Lua's standard library used for similar purposes:
      -- `s1:format(arg1, ...)` or equivalently string.format(s1, arg1, ...)`.)
      -- You can tell `fmt.f()` to put a planet/system/commodity object into the text, and
      -- (via the `tostring` built-in) know to write its name in the player's native language.
      text = fmt.f(_([["Ah, it's you again! Have you changed your mind? Like I said, I just need transport to {pnt} in the {sys} system, and I'll pay you {rwd} when we get there. How's that sound?"]]), {pnt=misplanet, sys=missys, rwd=reward_text})
   else
      text = fmt.f(_([[As you approach the guy, he looks up in curiosity. You sit down and ask him how his day is. "Why, fine," he answers. "How are you?" You answer that you are fine as well and compliment him on his suit, which seems to make his eyes light up. "Why, thanks! It's my favourite suit! I had it custom tailored, you know.
    "Actually, that reminds me! There was a special suit on {pnt} in the {sys} system, the last one I need to complete my collection, but I don't have a ship. You do have a ship, don't you? So I'll tell you what, give me a ride and I'll pay you {rwd} for it! What do you say?"]]),
         {pnt=misplanet, sys=missys, rwd=reward_text})
      mem.talked = true
   end

   -- This will create the typical "Yes/No" dialogue. It returns true if
   -- yes was selected.
   -- For more full-fledged visual novel API please see the vn module. The
   -- vntk module wraps around that and provides a more simple and easy to use
   -- interface, although it is much more limited.
   if vntk.yesno( _("My Suit Collection"), text ) then
      -- Followup text.
      vntk.msg( _("My Suit Collection"), _([["Fantastic! I knew you would do it! Like I said, I'll pay you as soon as we get there. No rush! Just bring me there when you're ready.]]) )

      -- Accept the mission
      misn.accept()

      -- Mission details:
      -- You should always set mission details right after accepting the
      -- mission.
      misn.setTitle( _("Suits Me Fine") )
      misn.setReward( reward_text )
      misn.setDesc( fmt.f(_("A well-dressed man wants you to take him to {pnt} in the {sys} system so he can get some sort of special suit."), {pnt=misplanet, sys=missys}) )

      -- Markers indicate a target planet (or system) on the map, it may not be
      -- needed depending on the type of mission you're writing.
      misn.markerAdd( misplanet, "low" )

      -- The OSD shows your objectives.
      local osd_desc = {}
      osd_desc[1] = fmt.f(_("Fly to {pnt} in the {sys} system"), {pnt=misplanet, sys=missys} )
      misn.osdCreate( _("Suits Me Fine"), osd_desc )

      -- This is where we would define any other variables we need, but
      -- we won't need any for this example.

      -- Hooks go here. We use hooks to cause something to happen in
      -- response to an event. In this case, we use a hook for when the
      -- player lands on a planet.
      hook.land( "land" )
   end
   -- If misn.accept() isn't run, the mission doesn't change and the player can
   -- interact with the NPC and try to start it again.
end

-- luacheck: globals land (Hook functions passed by name)
-- ^^ That is a directive to Luacheck, telling it we're about to use a global variable for a legitimate reason.
-- (More info here: https://github.com/naev/naev/issues/1566) Typically we put these at the top of the file.

-- This is our land hook function. Once `hook.land( "land" )` is called,
-- this function will be called any time the player lands.
function land ()
   -- First check to see if we're on our target planet.
   if spob.cur() == misplanet then
      -- Mission accomplished! Now we do an outro dialog and reward the
      -- player. Rewards are usually credits, as shown here, but
      -- other rewards can also be given depending on the circumstances.
      vntk.msg( fmt.f(_([[As you arrive at {pnt}, your passenger reacts with glee. "I must sincerely thank you, kind stranger! Now I can finally complete my suit collection, and it's all thanks to you. Here is {reward}, as we agreed. I hope you have safe travels!"]]), {pnt=misplanet, reward=reward_text}) )

      -- Reward the player. Rewards are usually credits, as shown here,
      -- but other rewards can also be given depending on the
      -- circumstances.
      player.pay( credits )

      -- Add a log entry. This should only be done for unique missions.
      neu.addMiscLog( fmt.f(_([[You helped transport a well-dressed man to {pnt} so that he could buy some kind of special suit to complete his collection.]]), {pnt=misplanet} ) )

      -- Finish the mission. Passing the `true` argument marks the
      -- mission as complete.
      misn.finish( true )
   end
end
