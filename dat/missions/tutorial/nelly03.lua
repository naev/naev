--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>1</priority>
  <chance>100</chance>
  <cond>require("common.pirate").systemPresence() &lt;= 0</cond>
  <location>Bar</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Za'lek</faction>
  <done>Helping Nelly Out 2</done>
  <chapter>[01]</chapter>
 </avail>
 <notes>
  <campaign>Tutorial Nelly</campaign>
 </notes>
</mission>
--]]
--[[
   Nelly Tutorial Campaign

   Second mission is designed to teach about:
   1. Mining
   2. Map notes

   Mission Details:
   0. Teach about mining minigame and make sure player has weapons
   1. Go to mining place
   2. Mine something
   3. Return back
--]]
local tutnel= require "common.tut_nelly"
--local tut   = require "common.tutorial"
local vn    = require 'vn'
local fmt   = require "format"
local lmisn = require "lmisn"
--local mining= require "minigame.mining"

--[[
   Mission States:
   0: Accepted and going to mine
   1: Acquired something
   2: Flying back
--]]
mem.misn_state = nil
-- luacheck: globals enter land heartbeat (Hook functions passed by name)

local reward_amount = tutnel.reward.nelly03

function create ()
   -- Save current system to return to
   mem.retpnt, mem.retsys = spob.cur()

   -- Need commodity exchange
   local rs = mem.retpnt:services()
   if rs.commodity == nil then
      misn.finish()
   end

   -- Check for asteroid fields
   if #system.cur():asteroidFields() > 0 then
      mem.destsys = system.cur()
   else
      mem.destsys = lmisn.getSysAtDistance( system.cur(), 0, 1, function( s )
         return (#s:asteroidFields() > 0)
      end )
   end
   if not misn.claim{ mem.destsys } then
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see a Nelly motioning to you at a table.") )
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   nel(_([[Nelly is covered in grease and smells a bit funny.
"Hey, who's everything doing? You wouldn't happen to be free? I was trying to fix my ship following this 'Foolproof Guide to Ship Repairs' manual, but it turns out that hull plating I dug out of the trash is harder to use than I expected. However, in the appendix it mentions that mining is a free alternative to getting supplies, and I was wondering if you could help me get some supplies to repair my ship?"]]))
   vn.menu{
      {_("Help them out"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[They look dejected.
"I guess I'll have to see if there is any other way to solve my problem."]]))
   vn.done( tutnel.nelly.transition )

   vn.label("accept")
   vn.func( function () doaccept = true end )
   nel(fmt.f(_([["Great! I swear it was totally not my fault this time. You see, I was just minding my own business, eating gravy packs while orbiting {pnt}, but I overheated them, and accidentally squirted gravy all over the ship controls that caused it to accelerate and ram into a {ship}! I was lucky that they didn't notice, but it caused half my ship to crumple on itself! They really should put warnings on the gravy packets or have a gravy detector system on the ship controls to not have this problem!"]]),
      {pnt=mem.retpnt, ship=ship.get("Zebra")}))
   vn.menu{
      {_("Tell them how to use the ship safety lock"), "01_safety"},
      {_("Ask about how they plan to repair the ship"), "01_repairs"},
      {_("Stay silent"), "01_silent"},
   }

   vn.label("01_safety")
   vn.na(_("You explain how ships have a safety lock that can be engaged to disallow controls temporarily that should be used when eating or just idling around. They listen to you intently."))
   nel(_([["You mean the big orange lever-thing? I use it to hang vegetables I'm drying out. I have never used it for anything else. It makes some wicked dried Pastor Reaper peppers. I'll have to look into that."]]))
   vn.jump("01_silent")

   vn.label("01_repairs")
   nel(_([["Oh, right over here on this page! It goes all over the different standard Llama configurations and how to troubleshoot all these different things."]]))
   vn.na(_("Ignoring the large amounts of gravy stains on the page she shows you, you see a big warning at the top that says to leave ship repairs to professionals and not attempt what is written there without supervision. However, you don't think it will do much good to point that out to Nelly."))
   vn.jump("01_silent")

   vn.label("01_silent")
   if mem.destsys == system.cur() then
      nel(fmt.f(_([["Anyway, if my info is correct, there should be an asteroid field right here in {sys}! You just have to take us there, and get some materials from it and we should all be set!"]]),
         {sys=mem.destsys}))
   else
      nel(fmt.f(_([["Anyway, if my info is correct, there should be an asteroid field nearby at the {sys} system! You just have to take us there, and get some materials from it and we should all be set!"]]),
         {sys=mem.destsys}))
   end

   nel(_([["One second. I have to… uh… go to the bathroom before we can be off!"
She runs off out of the spaceport bar.]]))
   vn.na(_("You wait, and after what seems to be quite a while, she comes back. Wait, is she covered in more grease than before!?"))
   nel(_([["All set, ready to go!"]]))
   vn.na(_("You go with her to the spaceport docks to board your ship, she seems a bit more giddy than usual."))
   vn.na(_("You get to your ship and the first thing you immediately notice is that someone taped an enormous drill onto your ship that has 'DRILLMASTER 5000' written on the side in permanent marker. You're amazed it doesn't fall off and turn to Nelly to ask for an explanation, only to find her beaming radiantly, likely very proud of her modification. You give up scolding and gesture towards the 'Drillmaster'."))
   nel(fmt.f(_([["Isn't it awesome! I figure we would need some drilling hardware to be able to get to the materials in the asteroid! You do know that normal weapons aren't able to extract materials properly right? Usually you have to use something like a {tool1}, or even better, a {tool2}! However, I'm pretty sure this should work as good or better! I wasn't able to fit an asteroid scanner, so we will have to eyeball it, but I'm great at that! Leave it to me."]]),
      {tool1=outfit.get("Mining Lance MK1"), tool2=outfit.get("S&K Plasma Drill")}))
   nel(_([["Time is credits or so they say, so let's go drill some asteroids!"]]))
   vn.na(_("You don't really expect the 'Drillmaster 5000' to make it through takeoff, but worst comes to worst, you expect to be able to blast asteroids apart, which might work better."))

   vn.done( tutnel.nelly.transition )
   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   mem.misn_state = 0
   mem.misn_marker = misn.markerAdd( mem.retsys )

   local title = _("Helping Nelly Out… Again")

   misn.osdCreate( title, {
      fmt.f(_("Go to an asteroid field in the {sys} system"),{sys=mem.destsys}),
      _("Get close to an asteroid and mine it"),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=mem.retpnt,sys=mem.retsys}),
   } )

   misn.setTitle( title )
   misn.setDesc(_("Help Nelly do some ad-hoc repairs on their ship by getting some materials from asteroids. There is no way this can go wrong, is there?"))
   misn.setReward( fmt.credits(reward_amount) )

   hook.enter("enter")
   hook.land("land")
end

function enter ()
   local scur = system.cur()
   local ast = scur:asteroidFields()
   if #ast > 0 then
      hook.timer( 5, "heartbeat" )
   end
end

function heartbeat ()
end

function land ()
   local cpnt = spob.cur()
   if cpnt == mem.retpnt and mem.misn_state >= 3 then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      nel(_([[""]]))
      vn.sfxVictory()
      vn.na(fmt.reward(reward_amount))
      vn.func( function () -- Rewards
         player.pay( reward_amount )
      end )
      vn.done( tutnel.nelly.transition )
      vn.run()

      tutnel.log(_("You helped Nelly get materials from asteroids to repair her ship."))

      misn.finish(true)
   end
end
