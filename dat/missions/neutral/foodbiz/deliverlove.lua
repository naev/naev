--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Deliver Love">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>50</chance>
   <location>Bar</location>
   <planet>Zeo</planet>
  </avail>
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

cargoname = N_("Love Letters")
cargodesc = N_("A cargo of feelings inked onto pulped, dried cellulose fibres.")


function create () --No system shall be claimed by mission
   startworld, startworld_sys = planet.cur()
   targetworld, targetworld_sys = planet.getS( "Zhiru" )

   reward = 20e3
   started = false

   misn.setNPC( _("Old-Fashioned Man"), "neutral/unique/michal.webp", _("A man sits in the corner of the bar, writing a letter.") )
end


function accept ()
   -- Introductions and a bit of bargaining
   if not started then
      if not tk.yesno( _("Absence Makes The Heart Grow Fonder"), fmt.f(_([[You can't help but wonder why the man in the corner is writing on paper instead of a datapad. As you approach the table he motions you to sit. "You must be wondering why I am using such an old fashioned way of recording information," he remarks with a grin. You take a sip of your drink as he continues. "I am writing a poem to my beloved. She lives on {pnt}." You glance at the flowing hand writing, back at the man, and back at the paper. "You wouldn't happen to be heading to {pnt} would you?" he asks.]]), {pnt=targetworld} ) ) then
         misn.finish()
      end
      started = true
      if not tk.yesno( _("Absence Makes The Heart Grow Fonder"), fmt.f(_([["It is a nice place I hear!" he exclaims visibly excited. "Say, I have written a ton of these letters at this point. You wouldn't be able to drop them off, would you?" You raise your eyebrow. "There would be a few credits in it for you... say, {credits}?" The man adds quickly with a hopeful expression. It seems like a low reward for a long journey...]]), {credits=fmt.credits(reward)} ) ) then
         reward = reward * 2 --look at you go, double the reward
         if not tk.yesno(_("Absence Makes The Heart Grow Fonder"), fmt.f(_([[The man grabs your arm as you begin to get up. "Alright, how about {credits}? Look, I wouldn't want The Empire reading these. The Emperor himself would blush." You sigh and give the man a long pause before answering.]]), {credits=fmt.credits(reward)} ) ) then
            misn.finish()
         end
      end
      if player.pilot():cargoFree() <  1 then
         tk.msg( _("Absence Makes The Heart Grow Fonder"), _([[You run a check of your cargo hold and notice it is packed to the brim. "Did I not mention I wrote a tonne of these letters? You don't have enough space for all of these," the man says. "I will be in the bar if you free some space up." You didn't expect him to have a LITERAL tonne of letters...]]) )
         misn.finish()
      end
   else
      if not tk.yesno( _("Absence Makes The Heart Grow Fonder"), _([["Ah, are you able to deliver my ton of letters for me now?"]]) ) then
         misn.finish()
      end
   end

   -- Add Mission Cargo and set up the computer

   misn.accept()
   local c = misn.cargoNew( cargoname, cargodesc )
   misn.cargoAdd( c, 1 )

   misn.setTitle( _([[Deliver Love]]) )
   misn.setReward( fmt.credits( reward ) )
   misn.setDesc( fmt.f(_([[Deliver the love letters to {pnt} in the {sys} system.]]), {pnt=targetworld, sys=targetworld_sys} ) )

   misn.osdCreate( _([[Deliver Love]]), {
      fmt.f(_("Fly to {pnt} in the {sys} system."), {pnt=targetworld, sys=targetworld_sys} ),
   } )
   misn.markerAdd( targetworld, "low" )

   -- set up hooks

   hook.land( "land" )
end

function land()
   if planet.cur() == targetworld then
      player.pay( reward )
      tk.msg( "", fmt.f(_([[You deliver the letters to a young woman who excitedly takes them and thanks you profusely. It seems you really made her day. When you check your balance, you see that {credits} have been transferred into your account. It also seems like you forgot a letter in the ship, but there were enough that you don't think it will be missed.]]), {credits=fmt.credits(reward)} ) )
      player.outfitAdd("Love Letter")
      neu.addMiscLog( _([[You delivered a literal tonne of letters for a love-struck, old-fashioned man.]]) )
      misn.finish( true )
   end
end
