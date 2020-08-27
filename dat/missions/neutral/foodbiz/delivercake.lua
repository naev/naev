--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Deliver Cake">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>50</chance>
   <done>Deliver Love</done>
   <location>Bar</location>
   <planet>Zhiru</planet>
  </avail>
 </mission>
 --]]
--[[
Mission Name: Deliver Cake
Author: iwaschosen

Plot: on Zhiru you meet the same girl who received the love letters,her name is Paddy. Asks if you'd be willing to run another delivery and work with Michal on food biz. she says the man is an entrepreneur, and is trying to build a Food business
 on Zeo where he will sell her baked goodies etc. asks if you can take recipes and plans to him on Zeo. Fills you cargo hold with cake which you don’t like. You can sell cake or bring to Michal who will pay a lot of $ for the cake, player doesn’t know that he will get payed for cake he brings.
--]]

require "numstring.lua"
require "dat/missions/neutral/common.lua"


-- Dialogue
title = _("A Tasty Job")
npc_name = _("Familiar Face")
bar_desc = _("A familiar looking young woman is looking at you")

firstcontact = _([[The woman smiles. "Aren't you the pilot that delivered those sweet love letters to me? I think you are! My name is Paddy, sorry I didn't introduce myself before. I was caught up in the moment; Michal's letters are always very exciting." She blushes. "Anyway, Michal is trying to start a restaurant on %s. Would you be interested in giving him a hand?"]])

toobad = _([["Oh, that's too bad. I thought it was such a good idea, too...."]])

objectives = _([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. Oh, and some of my homemade cake! I've packed that cake into your ship. Feel free to give it a taste! It's delicious! Anyway, Michal will pay you %s credits when you get there. Thank you so much!"
    When you arrive at your ship, you find your cargo hold packed to the brim with cake. You decide to try some, but the second it enters your mouth, you can't help but to spit it out in disgust. This is easily the most disgusting cake you've ever tasted. Well, as long as you get paid....]])

objectives_nocake = _([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. I was hoping to deliver some cake to him too, but it seems your ship doesn't have enough space for it, so that's unfortunate. In any case, Michal will pay you %s credits when you arrive. Thank you so much!"]])

-- Mission Computer text
misn_desc = _([[Deliver the recipes to Michal on %s in the %s system.]])
reward_desc = _([[%s credits]])

-- Mission Ending
finish = _([[As Michal takes the recipes and cake off your hands, you can't help but wonder how quickly his business will fail with food as bad as the cake you tried. When he remarks how delicious he apparently thinks the cake is, that confirms in your mind that he doesn't have a clue what he's doing. You bite your tongue, however, wishing him good luck as you take your pay.]])

finish_nocake = _([[Michal takes the recipes from you gleefully and tells you how unfortunate it is that you weren't able to taste Paddy's cake, which he says is delicious. You shrug it off and collect your pay.]])

osd_desc = {}
osd_desc[1] = _("Fly to %s in the %s system.")
osd_desc["__save"] = true

log_text = _([[You delivered a whole lot of the most disgusting cake you've ever tasted in your life as well as recipes for making said cake to Michal, the man who had you deliver a literal tonne of love letters before. Supposedly this is for an attempt to start a restaurant, but with food as disgusting as that cake, you're sure the business will fail.]])
log_text_nocake = _([[You delivered recipes for making some kind of cake to Michal, the man who had you deliver a literal tonne of love letters before.]])

cakes = "Food"
  

function create () --No system shall be claimed by mission
   startworld, startworld_sys = planet.cur()
   targetworld, targetworld_sys = planet.get( "Zeo" )

   reward = 10000

   misn.setNPC( npc_name, "neutral/unique/paddy" )
   misn.setDesc( bar_desc )
end

function accept()
   if not tk.yesno( title, firstcontact:format( targetworld:name() ) ) then
      tk.msg( title, toobad )
      misn.finish()
   end

   misn.accept()

   amount = player.pilot():cargoFree()
   if amount > 0 then
      misn.cargoAdd( cakes, amount )
      reward = reward + ( 1000 * amount )
      tk.msg( title, objectives:format( numstring( reward ) ) )
   else
      tk.msg( title, objectives_nocake:format( numstring( reward ) ) )
   end

   -- set up mission computer
   misn.setTitle( title )
   misn.setReward( reward_desc:format( numstring( reward ) ) )
   misn.setDesc( misn_desc:format( targetworld:name(), targetworld_sys:name() ) )

   osd_desc[1] = osd_desc[1]:format( targetworld:name(), targetworld_sys:name() )
   misn.osdCreate( title, osd_desc )
   misn.markerAdd( targetworld_sys, "low" )

   --set up hooks
   landhook = hook.land( "land" )
end

function land()
   if planet.cur() == targetworld then
      if amount > 0 then
         tk.msg( "", finish )
         addMiscLog( log_text )
      else
         tk.msg( "", finish_nocake )
         addMiscLog( log_text_nocake )
      end

      player.pay( reward )
      misn.finish(true)
   end
end
