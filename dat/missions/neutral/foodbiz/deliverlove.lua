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

require "numstring.lua"
require "dat/missions/neutral/common.lua"


misn_title = _([[Deliver Love]])
npc_name = _("Old-Fashioned Man")
bar_desc = _("A man sits in the corner of the bar, writing a letter.")

title = _("Absence Makes The Heart Grow Fonder")

firstcontact = _([[You can't help but wonder why the man in the corner is writing on paper instead of a datapad. As you approach the table he motions you to sit. "You must be wondering why I am using such an old fashioned way of recording information," he remarks with a grin. You take a sip of your drink as he continues. "I am writing a poem to my beloved. She lives on %s." You glance at the flowing hand writing, back at the man, and back at the paper. "You wouldn't happen to be heading to %s would you?" he asks.]])

acceptornot = _([["It is a nice place I hear!" he exclaims visibly excited. "Say, I have written a ton of these letters at this point. You wouldn't be able to drop them off, would you?" You raise your eyebrow. "There would be a few credits in it for you... say, %s credits?" The man adds quickly with a hopeful expression. It seems like a low reward for a long journey...]])

bargain = _([[The man grabs your arm as you begin to get up. "Alright, how about %s credits? Look, I wouldn't want The Empire reading these. The Emperor himself would blush." You sigh and give the man a long pause before answering.]])

not_enough_cargospace = _([[You run a check of your cargo hold and notice it is packed to the brim. "Did I not mention I wrote a tonne of these letters? You don't have enough space for all of these," the man says. "I will be in the bar if you free some space up." You didn't expect him to have a LITERAL tonne of letters...]])

ask_again = _([["Ah, are you able to deliver my ton of letters for me now?"]])

reward_desc = _([[%s credits]])

misn_desc = _([[Deliver the love letters to %s in the %s system.]])

misn_accomplished = _([[You deliver the letters to a young woman who excitedly takes them and thanks you profusely. It seems you really made her day. When you check your balance, you see that %s credits have been transferred into your account.]])

osd_desc = {}
osd_desc[1] = _("Fly to %s in the %s system.")
osd_desc["__save"] = true

log_text = _([[You delivered a literal tonne of letters for a love-struck, old-fashioned man.]])

cargoname = "Love Letters"


function create () --No system shall be claimed by mission
   startworld, startworld_sys = planet.cur()
   targetworld, targetworld_sys = planet.get( "Zhiru" )

   reward = 20000
   started = false

   misn.setNPC( npc_name, "neutral/unique/michal" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Introductions and a bit of bargaining
   if not started then
      if not tk.yesno( title, firstcontact:format( targetworld:name(), targetworld:name() ) ) then
         misn.finish()
      end
      started = true
      if not tk.yesno( title, acceptornot:format( numstring( reward ) ) ) then
         reward = reward * 2 --look at you go, double the reward
         if not tk.yesno(title, bargain:format( numstring( reward ) ) ) then
            misn.finish()
         end
      end
      if player.pilot():cargoFree() <  1 then
         tk.msg( title, not_enough_cargospace )
         misn.finish()
      end
   else
      if not tk.yesno( title, ask_again ) then
         misn.finish()
      end
   end

   -- Add Mission Cargo and set up the computer

   misn.accept()
   misn.cargoAdd( cargoname, 1 )

   misn.setTitle( misn_title )
   misn.setReward( reward_desc:format( numstring( reward ) ) )
   misn.setDesc( misn_desc:format( targetworld:name(), targetworld_sys:name() ) )

   osd_desc[1] = osd_desc[1]:format( targetworld:name(), targetworld_sys:name() )
   misn.osdCreate( misn_title, osd_desc )
   misn.markerAdd( targetworld_sys, "low" )

   -- set up hooks

   hook.land( "land" )
end

function land()
   if planet.cur() == targetworld then
      player.pay( reward )
      tk.msg( "", misn_accomplished:format( numstring( reward ) ) )
      addMiscLog( log_text )
      misn.finish( true )
   end
end
