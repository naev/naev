--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Deliver Cake">
 <unique />
 <priority>4</priority>
 <chance>50</chance>
 <done>Deliver Love</done>
 <location>Bar</location>
 <spob>Zhiru</spob>
</mission>
--]]
--[[
Mission Name: Deliver Cake
Author: iwaschosen

Plot: on Zhiru you meet the same girl who received the love letters,her name is Paddy. Asks if you'd be willing to run another delivery and work with Michal on food biz. she says the man is an entrepreneur, and is trying to build a Food business
 on {michalspob} where he will sell her baked goodies etc. asks if you can take recipes and plans to him on {michalspob}. Fills you cargo hold with cake which you don’t like. You can sell cake or bring to Michal who will pay a lot of $ for the cake, player doesn’t know that he will get payed for cake he brings.
--]]
local fmt = require "format"
local neu = require "common.neutral"
local vn = require "vn"
local portrait = require "portrait"
local foodbiz = require "common.foodbiz"

local cargoname = N_("Love Cake")
local cargodesc = N_("A cargo of feelings baked into a flour confection.")

local targetworld, targetworld_sys = foodbiz.michalspob()

mem.reward = 100e3
local reward_per_tonne = 5e3

local npc_name = _("Paddy")
local npc_portrait = "neutral/unique/paddy.webp"
local npc_image = portrait.getFullPath( npc_portrait )

local npc2_name = _("Michal")
local npc2_portrait = "neutral/unique/michal.webp"
local npc2_image = portrait.getFullPath( npc2_portrait )

function create () --No system shall be claimed by mission
   misn.setNPC( _("Familiar Face"), npc_portrait, _("A familiar looking young woman is looking at you") )
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local paddy = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   paddy(fmt.f(_([[The woman smiles. "Aren't you the pilot that delivered those sweet love letters to me? I think you are! My name is Paddy. Sorry I didn't introduce myself before. I was caught up in the moment; Michal's letters are always very exciting." She blushes. "Anyway, Michal is trying to start a restaurant on {pnt}. Would you be interested in giving him a hand?"]]),
      {pnt=targetworld} ) )

   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   paddy(_([["Oh, that's too bad. I thought it was such a good idea, too…"]]))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      accepted = true
      mem.amount = player.pilot():cargoFree()
      if mem.amount <= 0 then
         vn.jump("nospace")
      else
         local c = commodity.new( cargoname, cargodesc )
         misn.cargoAdd( c, mem.amount )
         mem.reward = mem.reward + reward_per_tonne * mem.amount
      end
   end )
   paddy( function () -- mem.reward is calculated dynamically
      return fmt.f(_([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. Oh, and some of my homemade cake! I've packed the cake into your ship. Feel free to give it a taste! It's delicious! Anyway, Michal will pay you {credits} when you get there. Thank you so much!"]]),
         {credits=fmt.credits(mem.reward)})
   end )
   vn.na(_([[When you arrive at your ship, you find your cargo hold packed to the brim with cake. You decide to try some, but the second it enters your mouth, you can't help but to spit it out in disgust. This is easily the most disgusting cake you've ever tasted. Well, as long as you get paid…]]))
   vn.done()

   vn.label("nospace")
   paddy(fmt.f(_([["Great!" Paddy says with a smile. She hands you what appear to be recipes. "I just need you to deliver these recipes to him. I was hoping to deliver some cake to him too, but it seems your ship doesn't have enough space for it, so that's unfortunate. In any case, Michal will pay you {credits} when you arrive. Thank you so much!"]]),
      {credits=fmt.credits(mem.reward)}))

   vn.run()

   if not accepted then return end

   misn.accept()

   -- set up mission computer
   misn.setTitle( _("A Tasty Job") )
   misn.setReward( mem.reward )
   misn.setDesc( fmt.f(_([[Deliver the recipes to Michal on {pnt} in the {sys} system.]]), {pnt=targetworld, sys=targetworld_sys} ) )

   misn.osdCreate( _("A Tasty Job"), {
      fmt.f(_("Fly to {pnt} in the {sys} system."), {pnt=targetworld, sys=targetworld_sys} ),
   } )
   misn.markerAdd( targetworld, "low" )

   --set up hooks
   mem.landhook = hook.land( "land" )
end

function land()
   if spob.cur() ~= targetworld then
      return
   end

   vn.clear()
   vn.scene()
   local michal = vn.newCharacter( npc2_name, {image=npc2_image} )
   vn.transition()

   if mem.amount > 0 then
      michal(_([[As Michal takes the recipes and cake off your hands, you can't help but wonder how quickly his business will fail with food as bad as the cake you tried. When he remarks how delicious he apparently thinks the cake is, that confirms your suspicion that he doesn't have a clue what he's doing. You bite your tongue, however, wishing him good luck as you take your pay.]]))
      vn.func( function ()
         neu.addMiscLog( _([[You delivered a whole lot of the most disgusting cake you've ever tasted in your life as well as recipes for making said cake to Michal, the man who had you deliver a literal tonne of love letters before. Supposedly this is in an attempt to start a restaurant, but with food as disgusting as that cake, you're sure the business will fail.]]) )
      end )
   else
      michal(_([[Michal takes the recipes from you gleefully and tells you how unfortunate it is that you weren't able to taste Paddy's cake, which he says is delicious. You shrug it off and collect your pay.]]) )
      vn.func( function ()
         neu.addMiscLog( _([[You delivered recipes for making some kind of cake to Michal, the man who had you deliver a literal tonne of love letters before.]]) )
      end )
   end

   vn.sfxVictory()
   vn.na(fmt.reward(mem.reward))
   vn.run()

   player.pay( mem.reward )
   misn.finish(true)
end
