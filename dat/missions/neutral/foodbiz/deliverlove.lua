--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Deliver Love">
 <unique />
 <priority>4</priority>
 <chance>50</chance>
 <location>Bar</location>
 <cond>
   local t = spob.cur():tags()
   if t.refugee or t.poor or t.restricted then
      return false
   end
   local _spb, sys = spob.getS("Zhiru")
   local d = sys:jumpDist()
   if d &lt; 3 and d &gt; 7 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]

--[[
Mission Name: Deliver Love
Author: iwaschosen
Plot: Talk to man on Zeo, bargain, load some cargo, deliver it to Zhiru in Goddard, get $
--]]
local fmt = require "format"
local neu = require "common.neutral"
local vn = require "vn"
local portrait = require "portrait"

local cargoname = N_("Love Letters")
local cargodesc = N_("A cargo of feelings inked onto pulped, dried cellulose fibres.")
local targetworld, targetworld_sys = spob.getS( "Zhiru" )

local reward = 50e3 -- Can get doubled, must be global!
local reward_outfit = outfit.get("Love Letter")

local npc_name = _("Old-Fashioned Man")
local npc_portrait = "neutral/unique/michal.webp"
local npc_image = portrait.getFullPath( npc_portrait )

local npc2_name = _("Young Woman")
local npc2_portrait = "neutral/unique/paddy.webp"
local npc2_image = portrait.getFullPath( npc2_portrait )

function create () --No system shall be claimed by mission
   mem.started = false
   misn.setNPC( npc_name, npc_portrait, _("A man sits in the corner of the bar, writing a letter.") )
end

function accept ()
   mem.reward = reward -- reset reward so it doesn't become infinite
   local accepted = false

   vn.clear()
   vn.scene()
   local michal = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   -- Introductions and a bit of bargaining
   if not mem.started then
      michal(fmt.f(_([[You can't help but wonder why the man in the corner is writing on paper instead of a datapad. As you approach the table he motions you to sit. "You must be wondering why I am using such an old fashioned way of recording information," he remarks with a grin. You take a sip of your drink as he continues. "I am writing a poem to my beloved. She lives on {pnt}." You glance at the flowing hand writing, back at the man, and back at the paper. "You wouldn't happen to be heading to {pnt} would you?" he asks.]]), {pnt=targetworld} ) )
      vn.menu{
         {_([[Yes]]), "1yes"},
         {_([[No]]), "decline"},
      }

      vn.label("1yes")
      vn.func( function ()
         mem.started = true
      end )

      michal(fmt.f(_([["It is a nice place I hear!" he exclaims visibly excited. "Say, I have written a ton of these letters at this point. You wouldn't be able to drop them off, would you?" You raise your eyebrow. "There would be a few credits in it for you… say, {credits}?" The man adds quickly with a hopeful expression. It seems like a low reward for a long journey…]]), {credits=fmt.credits(mem.reward)} ) )
      vn.menu{
         {_([[Yes]]), "2yes"},
         {_([[No]]), "2no"},
      }

      vn.label("2no")
      vn.func( function ()
         mem.reward = mem.reward * 2 --look at you go, double the reward
      end )
      michal(fmt.f(_([[The man grabs your arm as you begin to get up. "Alright, how about {credits}? Look, I wouldn't want The Empire reading these. The Emperor himself would blush." You sigh and give the man a long pause before answering.]]), {credits=fmt.credits(mem.reward)} ) )
      vn.menu{
         {_([[Yes]]), "2yes"},
         {_([[No]]), "decline"},
      }

      vn.label("nospace")
      michal(_([[You run a check of your cargo hold and notice it is packed to the brim. "Did I not mention I wrote a tonne of these letters? You don't have enough space for all of these," the man says. "I will be in the bar if you free up some space." You didn't expect him to have a LITERAL tonne of letters…]]))
      vn.done()

      vn.label("2yes")
      vn.func( function ()
         if player.pilot():cargoFree() <  1 then
            vn.jump("nospace")
         end
         accepted = true
      end )

   else
      michal(_([["Ah, are you able to deliver my ton of letters for me now?"]]) )
      vn.menu{
         {_([[Accept]]), "accept"},
         {_([[Decline]]), "decline"},
      }
   end

   vn.label("decline")
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   vn.na(fmt.f(_([[You're not sure what you expected, but you end up having a LITERAL tonne of letters headed for {spob} in the {sys} system.]]),
      {spob=targetworld, sys=targetworld_sys}))
   vn.run()

   if not accepted then return end

   -- Add Mission Cargo and set up the computer
   var.push("foodbiz_spb",spob.cur():nameRaw()) -- Be flexible with the location
   misn.accept()
   local c = commodity.new( cargoname, cargodesc )
   misn.cargoAdd( c, 1 )

   misn.setTitle( _([[Deliver Love]]) )
   misn.setReward( mem.reward )
   misn.setDesc( fmt.f(_([[Absence makes the heart grow fonder. Deliver love letters to {pnt} in the {sys} system.]]), {pnt=targetworld, sys=targetworld_sys} ) )

   misn.osdCreate( _([[Deliver Love]]), {
      fmt.f(_("Fly to {pnt} in the {sys} system."), {pnt=targetworld, sys=targetworld_sys} ),
   } )
   misn.markerAdd( targetworld, "low" )

   -- set up hooks
   hook.land( "land" )
end

function land()
   if spob.cur() ~= targetworld then
      return
   end

   vn.clear()
   vn.scene()
   local paddy = vn.newCharacter( npc2_name, {image=npc2_image} )
   vn.transition()

   paddy(fmt.f(_([[You deliver the letters to a young woman who excitedly takes them and thanks you profusely. It seems you really made her day. When you check your balance, you see that {credits} have been transferred into your account. It also seems like you forgot a letter in the ship, but there were enough that you don't think it will be missed.]]),
      {credits=fmt.credits(mem.reward)}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( mem.reward )
      player.outfitAdd( reward_outfit )
   end )
   vn.na(fmt.reward(mem.reward).."\n\n"..fmt.reward(reward_outfit) )

   vn.run()

   neu.addMiscLog( _([[You delivered a literal tonne of letters for a love-struck, old-fashioned man.]]) )
   misn.finish( true )
end
