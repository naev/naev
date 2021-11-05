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
local fmt = require "format"
local neu = require "common.neutral"

local targetworld, targetworld_sys = planet.getS( "Zeo" )

reward = 50e3 -- Plus 1e3 * cake, has to be global (is saved)

function create () --No system shall be claimed by mission
   startworld, startworld_sys = planet.cur()

   misn.setNPC( _("Familiar Face"), "neutral/unique/paddy.webp", _("A familiar looking young woman is looking at you") )
end

function accept()
   if not tk.yesno( _("A Tasty Job"), fmt.f(_([[The woman smiles. "Aren't you the pilot that delivered those sweet love letters to me? I think you are! My name is Paddy, sorry I didn't introduce myself before. I was caught up in the moment; Michal's letters are always very exciting." She blushes. "Anyway, Michal is trying to start a restaurant on {pnt}. Would you be interested in giving him a hand?"]]), {pnt=targetworld} ) ) then
      tk.msg( _("A Tasty Job"), _([["Oh, that's too bad. I thought it was such a good idea, too...."]]) )
      misn.finish()
   end

   misn.accept()

   amount = player.pilot():cargoFree()
   if amount > 0 then
      misn.cargoAdd( "Food", amount )
      reward = reward + ( 1e3 * amount )
      tk.msg( _("A Tasty Job"), fmt.f(_([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. Oh, and some of my homemade cake! I've packed that cake into your ship. Feel free to give it a taste! It's delicious! Anyway, Michal will pay you {credits} when you get there. Thank you so much!"
    When you arrive at your ship, you find your cargo hold packed to the brim with cake. You decide to try some, but the second it enters your mouth, you can't help but to spit it out in disgust. This is easily the most disgusting cake you've ever tasted. Well, as long as you get paid....]]), {credits=fmt.credits(reward)} ) )
   else
      tk.msg( _("A Tasty Job"), fmt.f(_([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. I was hoping to deliver some cake to him too, but it seems your ship doesn't have enough space for it, so that's unfortunate. In any case, Michal will pay you {credits} when you arrive. Thank you so much!"]]), {credits=fmt.credits(reward)} ) )
   end

   -- set up mission computer
   misn.setTitle( _("A Tasty Job") )
   misn.setReward( fmt.credits( reward ) )
   misn.setDesc( fmt.f(_([[Deliver the recipes to Michal on {pnt} in the {sys} system.]]), {pnt=targetworld, sys=targetworld_sys} ) )

   misn.osdCreate( _("A Tasty Job"), {
      fmt.f(_("Fly to {pnt} in the {sys} system."), {pnt=targetworld, sys=targetworld_sys} ),
   } )
   misn.markerAdd( targetworld, "low" )

   --set up hooks
   landhook = hook.land( "land" )
end

function land()
   if planet.cur() == targetworld then
      if amount > 0 then
         tk.msg( "", _([[As Michal takes the recipes and cake off your hands, you can't help but wonder how quickly his business will fail with food as bad as the cake you tried. When he remarks how delicious he apparently thinks the cake is, that confirms in your mind that he doesn't have a clue what he's doing. You bite your tongue, however, wishing him good luck as you take your pay.]]) )
         neu.addMiscLog( _([[You delivered a whole lot of the most disgusting cake you've ever tasted in your life as well as recipes for making said cake to Michal, the man who had you deliver a literal tonne of love letters before. Supposedly this is for an attempt to start a restaurant, but with food as disgusting as that cake, you're sure the business will fail.]]) )
      else
         tk.msg( "", _([[Michal takes the recipes from you gleefully and tells you how unfortunate it is that you weren't able to taste Paddy's cake, which he says is delicious. You shrug it off and collect your pay.]]) )
         neu.addMiscLog( _([[You delivered recipes for making some kind of cake to Michal, the man who had you deliver a literal tonne of love letters before.]]) )
      end

      player.pay( reward )
      misn.finish(true)
   end
end
