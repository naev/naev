--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 3">
 <unique />
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
local tut   = require "common.tutorial"
local vn    = require 'vn'
local fmt   = require "format"
local lmisn = require "lmisn"
local mining = require "minigames.mining"

--[[
   Mission States:
   0: Accepted and going to mine
   1: Acquired something and flying back
--]]
mem.misn_state = nil

local reward_amount = tutnel.reward.nelly03
local reward_cargo = commodity.get("Nickel")

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
      local candidates = lmisn.getSysAtDistance( system.cur(), 0, 1, function( s )
         return (#s:asteroidFields() > 0)
      end )
      if #candidates <= 0 then
         misn.finish()
      end
      mem.destsys = candidates[ rnd.rnd(1,#candidates) ]
   end
   if not misn.claim{ mem.destsys, "nelly" } then
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see Nelly motioning to you at a table.") )
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   nel(_([[Nelly is covered in grease and smells a bit funny.
"Hey, how's everything going? You wouldn't happen to be free? I was trying to fix my ship following this 'Foolproof Guide to Ship Repairs' manual, but it turns out that hull plating I dug out of the trash is harder to use than I expected. However, in the appendix it mentions that mining is a free alternative to getting supplies, and I was wondering if you could help me get some supplies to repair my ship?"]]))
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
   nel(fmt.f(_([["I've recently learned about the notes functionality in the system map, it's very useful for writing down where good asteroids are! You can add notes to systems from the system map which you can access with the {mapkey}. You should try it out."]]),
      {mapkey=tut.getKey("starmap")}))

   nel(_([["One second. I have to… uh… go to the bathroom before we can be off!"
She runs off out of the spaceport bar.]]))
   vn.na(_("You wait, and after what seems to be quite a while, she comes back. Wait, is she covered in more grease than before!?"))
   nel(_([["All set, ready to go!"]]))
   vn.na(_("You go with her to the spaceport docks to board your ship, she seems a bit more giddy than usual."))
   vn.na(_("You get to your ship and the first thing you immediately notice is that someone taped an enormous drill onto your ship that has 'DRILLMASTER 5000' written on the side in permanent marker. It also has some crude flames drawn on it. You're amazed it doesn't fall off and turn to Nelly to ask for an explanation, only to find her beaming radiantly, likely very proud of her modification. You give up scolding and gesture towards the 'Drillmaster'."))
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
   mem.misn_marker = misn.markerAdd( mem.destsys )

   local title = _("Helping Nelly Out… Again")

   misn.osdCreate( title, {
      fmt.f(_("Go to an asteroid field in the {sys} system"),{sys=mem.destsys}),
      _("Get close and match speeds to an asteroid and mine it"),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=mem.retpnt,sys=mem.retsys}),
   } )

   misn.setTitle( title )
   misn.setDesc(_("Help Nelly do some ad-hoc repairs on their ship by getting some materials from asteroids. There is no way this can go wrong, is there?"))
   misn.setReward(reward_amount)

   hook.enter("enter")
   hook.land("land")
end

local hb_state
function enter ()
   local scur = system.cur()
   local ast = scur:asteroidFields()
   if #ast > 0 then
      hb_state = 1
      hook.timer( 5, "heartbeat" )
   end
end

local function nelly_say( msg )
   player.autonavReset( 3 )
   player.msg(fmt.f(_([[Nelly: "{msg}"]]),{msg=msg}),true)
end

local function drilltime ()
   local mining_success = false
   local mining_tries = 0

   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   vn.na(_("You approach the asteroid, Nelly seems a bit nervous."))
   nel(_([["Time for the big moment! I've never actually tried the 'DRILLMASTER 5000'. This is like a maiden's voyage!"]]))
   nel(_([["When I activate it, you'll see the drill interface. It will start stopped and you have to press any key for it to start. You have to press the trigger when the blue scanning thing is inside the red optimal target zone. I guess this is easier shown then done. Let me start it up and go ahead and try it!"]]))

   vn.label("mining")
   mining.vn{ difficulty=0, shots_max=2, reward_func=function( bonus )
      mining_tries = mining_tries + 1
      if bonus >= 1 then
         mining_success = true
      end
   end }
   vn.func( function ()
      if mining_success then
         vn.jump( "mining_success" )
      elseif mining_tries > 3 then
         vn.jump( "mining_tries" )
      else
         vn.jump( "mining_failure" )
      end
   end )

   vn.label("mining_failure")
   nel(_([["You have to try to time it a bit better. We won't get the best minerals otherwise! Try again!"]]))
   vn.jump("mining")

   vn.label("mining_tries")
   nel(_([["You still don't have the timing down, but I think we have enough materials for now. Do you want to try again?"]]))
   vn.menu{
      {_("Try again!"), "mining"},
      {_("Enough."), "mining_done"},
   }

   vn.label("mining_success")
   nel(_([["Great job! You really nailed it!"]]))
   vn.label("mining_done")
   nel(fmt.f(_([["With all the materials we got, I should be able to repair the ship no problem. Take us back to {pnt}!"]]),
      {pnt=mem.retpnt}))

   vn.func( function ()
      local pp = player.pilot()
      mem.cargo_added = pp:cargoFree()
      if mem.cargo_added > 0 then
         pp:cargoAdd( reward_cargo, pp:cargoFree() ) -- Fill cargo hold
      else
         vn.jump("noadd")
      end
   end )
   vn.na( function () return fmt.f(_("You have obtained {mass} of {cargo}!"),{mass=fmt.tonnes(mem.cargo_added),cargo=reward_cargo}) end )
   vn.label("noadd")
   vn.na(_("It seems like just in time too, because as Nelly talks, you hear a groan on the hull and see what is left of the 'Drillmaster' is left as an irreparable damaged mess that you have no choice to leave behind."))
   nel(_([["Drillmaster, we hardly knew thee."
She gives a small reverence to the debris before coming back to her happy self.
"Oh well, I found that in an Imperial garbage dump anyway. Although I was pretty proud of the flames I drew on it…"]]))
   nel(fmt.f(_([["On to {pnt}!"]]), {pnt=mem.retpnt}))

   vn.sfxBingo()
   vn.done( tutnel.nelly.transition )
   vn.run()

   mem.misn_state = 1
   misn.markerMove( mem.misn_marker, mem.retpnt )
   misn.osdActive(3)
end

function heartbeat ()
   if hb_state==1 then
      local af = system.cur():asteroidFields()
      nelly_say(fmt.f(_("I've marked an asteroid field on your overlay. Use {overlaykey} to check it!"),
         {overlaykey=tut.getKey("overlay")}))
      system.markerAdd( af[ rnd.rnd(1,#af) ].pos )
      hb_state = hb_state+1
   elseif hb_state==2 then
      local af = system.cur():asteroidFields()
      local ppos = player.pos()
      for k,v in ipairs(af) do
         if ppos:dist( v.pos ) < v.radius+1000 then
            nelly_say(_("Get close to an asteroid try to stop ontop of it. I want to try the Drillmaster!"))
            misn.osdActive(2)
            hb_state = hb_state+1
         end
      end

   elseif hb_state==3 then
      local pp = player.pilot()
      local a = asteroid.get( pp )
      if a:pos():dist( pp:pos() ) < 50 and a:vel():dist( pp:vel() ) < 15 then
         system.markerClear()
         drilltime()
         a:setTimer( -1 ) -- Get rid of the asteroid
         return -- We're done here
      end
   end

   hook.timer( 0.5, "heartbeat" )
end

function land ()
   local cpnt = spob.cur()
   if cpnt == mem.retpnt and mem.misn_state >= 1 then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      nel(_([["Great! That was awesome. Now it is my time to shine and repair my ship!"]]))
      nel(_([["Here, let me transfer a nice reward for you!"]]))
      vn.sfxVictory()
      vn.na(fmt.reward(reward_amount))
      vn.func( function () -- Rewards
         player.pay( reward_amount )
      end )
      if mem.cargo_added > 0 then
         nel(fmt.f(_([["You can also keep the extra {cargo} we got. I would recommend selling it, but I'm really bad at finding out the most expensive place to make money. Maybe you are better?"]]),
            {cargo=reward_cargo}))
      end
      vn.na(_("With some materials in hand, she rushes towards her beaten-up Llama to begin the reparations. Hopefully she'll be able to pull this off, but it's best for her to try herself."))
      vn.done( tutnel.nelly.transition )
      vn.run()

      tutnel.log(_("You helped Nelly get materials from asteroids to repair her ship."))

      misn.finish(true)
   end
end
