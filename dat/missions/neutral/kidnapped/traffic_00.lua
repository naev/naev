--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kidnapped">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Kidnapped">Triggers</done_evt>
  <campaign>Kidnapping</campaign>
 </notes>
</mission>
--]]
--[[
      MISSION: Kidnapped
      AUTHOR: Superkoop - John Koopman

      The first mission in a series of missions surrounding human trafficking. This mission consists of overhearing pirate a couple pirate conversations, disabling a trader ship, and returning the children home to their parents. It essentially sets up everything for the following 4 missions.
--]]
local fmt = require "format"
local portrait = require "portrait"
local neu = require "common.neutral"
local vn = require "vn"
local vntk = require "vntk"

-- TODO add some sort of reward I guess

local kidnappers -- Non-persistent state

local sys1 = system.get("Arcturus")
local sys2 = system.get("Goddard")
local pnt1, sys3 = spob.getS("Praxis")
local pnt2, sys4 = spob.getS("Seanich")
local pnthome = spob.get("Brooks")

-- NPCs
local pir1_desc = _([[The two pirates seem to be talking rather quietly, but loud enough for you to overhear if you are careful.]])
local pir2_desc = _([[The pirates have both drank their wallet's worth today, so eavesdropping shouldn't be too much of an issue.]])

function create()
   if not misn.claim( {system.get("Goddard")}, true ) then
      warn(_("Something has gone wrong! Claim that should work has failed!"))
      return
   end

   misn.accept()

   misn.setTitle(_("Kidnapped"))
   misn.setReward(_("A Reunited Family"))
   misn.setDesc(_([[Search for the kidnapped children, then rescue the children and return them to their parents.]]))

   misn.osdCreate(_("Kidnapped"), {fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sys3, pnt=pnt1})})

   mem.misn_mark = misn.markerAdd(pnt1, "low")

   mem.eavesdropped1 = false
   mem.eavesdropped2 = false
   mem.rescued = false

   mem.lhook =  hook.land("land1", "land")
   hook.enter("enter")

end

local portrait1, portrait2
function land1()
   if spob.cur() == pnt1 and not mem.eavesdropped1 and not mem.eavesdropped2 then
      portrait1 = portrait.getMale("Pirate")
      portrait2 = portrait.get("Pirate")
      mem.bar1pir1 = misn.npcAdd("firstpirates", _("Pirate"), portrait1, pir1_desc)
      mem.bar1pir2 = misn.npcAdd("firstpirates", _("Pirate"), portrait2, pir1_desc)
   end
end

function land2()
   if spob.cur() == pnt2 and mem.eavesdropped1 and not mem.eavesdropped2 then
      portrait1 = portrait.get("Pirate")
      portrait2 = portrait.get("Pirate")
      mem.bar2pir1 = misn.npcAdd("secondpirates", _("Pirate"), portrait1, pir2_desc)
      mem.bar2pir2 = misn.npcAdd("secondpirates", _("Pirate"), portrait2, pir2_desc)
   end
end

function firstpirates()
   vn.clear()
   vn.scene()
   local p1 = vn.newCharacter( _("Pirate A"), {image=portrait.getFullPath(portrait1), pos="left"} )
   local p2 = vn.newCharacter( _("Pirate B"), {image=portrait.getFullPath(portrait2), pos="right"} )
   vn.transition()

   p1(_([[You sit down at a table adjacent to these two pirates, ordering a drink and trying to act as inconspicuous as you can. You catch the pirates in mid-conversation. "…And he says to me 'I will give you everything, please just leave me alone!' So I take his credits, and all I get is 2K! He's clearly holding back on me, trust me, I know! So I trash his ship, and what do you know, he really didn't have any more. It's tough making any money these days, sometimes I think I gotta get into a different line o' work."]]))
   p2(fmt.f(_([[The other pirate sitting there replies with a glint in his eye, "Actually, I heard in the bar over on {pnt} that you can make fat stacks doing a little more risky work. You just gotta nab some brats, and you can sell 'em for 15 big ones a pop!"]]),
      {pnt=pnt2}))
   p1(_([["Human trafficking? No way man, that stuff gives me the heebie jeebies!"]]))
   p2(_([["Whatever, man."]]))
   p1(_([[After a brief pause the first pirate starts talking, "So, I was seeing the doctor the other day and he said that rash on my back is probably an allergic reaction."]]))
   vn.na(_([[From that point you figure the conversation will not be picking up again, and having a lead you decide to take it.]]))

   vn.run()

   misn.npcRm(mem.bar1pir1)
   misn.npcRm(mem.bar1pir2)

   misn.osdCreate(_("Kidnapped"), {fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sys4, pnt=pnt2})})

   misn.markerMove(mem.misn_mark, pnt2)

   hook.rm(mem.lhook)
   mem.lhook = hook.land("land2", "land")

   mem.eavesdropped1 = true
end

function secondpirates()
   vn.clear()
   vn.scene()
   local p1 = vn.newCharacter( _("Pirate A"), {image=portrait.getFullPath(portrait1), pos="left"} )
   vn.newCharacter( _("Pirate B"), {image=portrait.getFullPath(portrait2), pos="right"} )
   vn.transition()

   vn.na(_([[You don't even bother sitting down too close to these two pirates considering how loudly they're talking. It doesn't take too much listening before you get exactly what you need as one of the pirates is telling his recent tales to the other.]]))
   p1(_([["So this dummy thought he could out-fly me in his pathetic Llama! So I took him offline in, like, 2 seconds, got on that ship, and took the kids. The guy tried to fight back, but I stopped that quick enough. Then the woman says they was on a vacation, like I care! Ha! Fools think they can even bother to mess with me when I have work to do."]]))
   p1(fmt.f(_([["So I took the kids to the {sys} system where they were loaded into this Trader Koala named the Progeny. Clever name if you ask me! No one will ever even wonder what it's carrying. It looks like the most innocent little guy flying around there. Little does everyone know it's waiting to fill up its load of brats!"]]),
      {sys=sys2}))
   vn.na(_([[Having listened to this dirt-bag, you feel like going over there and giving that pirate a good beating. But if you get yourself killed now you will never be able to save those children, and you don't even want to think what will happen to those children if you don't rescue them.]]))

   vn.run()

   misn.npcRm(mem.bar2pir1)
   misn.npcRm(mem.bar2pir2)

   misn.osdCreate(_("Kidnapped"), {
      fmt.f(_("Fly to the {sys} system and disable (do not destroy) that Koala"), {sys=sys2})
   })

   misn.markerMove(mem.misn_mark, sys2)

   hook.rm(mem.lhook)

   mem.eavesdropped2 = true
end

function enter()
   local fkidnappers = faction.dynAdd( "Pirate", "Kidnappers", _("Kidnappers"), { clear_allies=true, clear_enemies=true, ai="trader" } )

   if mem.eavesdropped1 and mem.eavesdropped2 and system.cur() == sys2 and (not mem.rescued) then
      kidnappers = pilot.add( "Koala", fkidnappers, spob.get("Zhiru"):pos() + vec2.new(-800,-800), _("Progeny"), {naked=true} )
      kidnappers:setHilight(true)
      kidnappers:setVisible(true)
      kidnappers:memory().aggressive = true
      kidnappers:control()
      mem.idlehook = hook.pilot(kidnappers, "idle", "idle")
      mem.attackhook = hook.pilot(kidnappers, "attacked", "attackedkidnappers")
      hook.pilot(kidnappers, "exploded", "explodedkidnappers")
      hook.pilot(kidnappers, "board", "boardkidnappers")
      idle()
   end
   mem.needpirates = true
end

function idle()
   local homepos = spob.get("Zhiru"):pos()
   kidnappers:moveto(homepos + vec2.new( 800,  800), false)
   kidnappers:moveto(homepos + vec2.new(-800,  800), false)
   kidnappers:moveto(homepos + vec2.new(-800, -800), false)
   kidnappers:moveto(homepos + vec2.new( 800, -800), false)
end

function attackedkidnappers()
   if kidnappers:exists() then
      kidnappers:runaway(player.pilot(), true)
   end

   if mem.needpirates then
      local p = {}
      for k = 1, 3 do
         p[k] = pilot.add( "Pirate Hyena", "Pirate", kidnappers:pos() + vec2.newP( 1200, 2*math.pi*k/3 ) )
         p[k]:setHostile( true )
         p[k]:setLeader( kidnappers )
      end
      p[1]:comm(_([[You are damaging the goods! You are dead!]]), true)
      mem.needpirates = false
   end
end

function explodedkidnappers()
   if not mem.rescued then
      hook.timer(1.5, "kidskilled")
   end
end

function kidskilled()
   vntk.msg(_("You killed the children"), _([[Having destroyed the kidnappers, you also just killed the children. As you sit there in space, with your head against the dash, a tear rolls down your cheek as you think of the parents and how their children are forever stolen from them. If only you could rewind and try again; you know the next time you would be more cautious. If only it were so easy…]]))
   misn.finish(false)
end

function boardkidnappers()
   vntk.msg(_("You did it!"), fmt.f(_([[After disabling the ship, you and your small crew go in ready for a fight! But when you get on the small Koala, you find only two men guarding it, and it turns out they are not prepared for fighting at all. They can pilot a ship, but fighting is not their forte. After you tie them up, you go to the cargo hold to rescue the children. When you get there, you find a few more than three; there are probably a couple dozen! This is all probably just the tip of the iceberg, too. Either way, it's time to head back to {pnt} and reunite the parents with their children.]]), {pnt=pnthome}))
   misn.osdCreate(_("Kidnapped"), {fmt.f(_("Return the children to the {sys} system on planet {pnt}"), {sys=sys1, pnt=pnthome})})
   misn.markerMove(mem.misn_mark, pnthome)
   kidnappers:setHilight(false)
   kidnappers:hookClear()
   local c = commodity.new( N_("Children"), N_("The rescued children.") )
   mem.thekids = misn.cargoAdd( c, 0 )
   player.unboard()
   mem.lhook = hook.land("land3", "land")
   mem.rescued = true
   kidnappers:setVisible(false)
end

function land3()
   if spob.cur() == pnthome and mem.rescued then
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[As you step off the landing deck with a couple dozen children in tow, the two parents you spoke with over the comm run up to you. From behind you hear a few children yelling, "Mom! Dad!" Three children shove their way out to the front and the parents and children meet in a big group of hugs, kisses, tears, and smiles from ear to ear.]]))
      vn.na(fmt.f(_([[You and the other children stand off to the side and watch one of the most beautiful reunions you have ever seen. After a little while the father approaches you, wiping a tear from his cheek, and takes you in an embrace. Releasing you and drying his eyes, he says, "Thank you so much, {player}. You have no idea what this means to us. I would love to be able to repay you somehow, but I just have no idea how I can do so right now. You have rescued my children and brought them back to me. Thank you isn't enough, but I'm afraid for now it's the best I can do. If there is anything I can ever do for you, feel free to ask me, my friend."]]),
         {player=player.name()}))
      vn.na(_([[You assure him that it is alright, and you will not hesitate to take him up on his offer. After a while the smile fades from his face. "You see how happy my family is? Well, look at all these other children here who are still separated from their parents. I want to return them all home. I'd also like to fight against all this human trafficking, but that will take a lot of planning. For now, I want to be with my family. Come back soon though if you're willing. I would like to get something organized."]]))
      vn.na(fmt.f(_([[The father goes back to his children, and as you start walking back to your ship you notice the father and mother lavishing their children in love. You look over to the other children now sitting around, gazing at the loving family with envy. Sighing, you begin climbing into your ship as the mother runs up to you, "{player}, wait a second! We know your name, but you don't know ours. I'm Janice." As she looks to her husband, who's talking animatedly with the children, she smiles. "My husband's name is Andrew. Thank you for everything." A tear rolls down her face as she looks at you with her bright hazel eyes and she kisses you on the cheek.
You watch her return to her family. A child jumps into her arms, and you climb up into your ship.]]),
         {player=player.name()}))

      --TODO should probably give some reward

      vn.run()

      neu.addMiscLog( _([[You successfully rescued a couple dozen children who were kidnapped by a child trafficking ring after two parents asked for your help rescuing their own children. Said parents said that they would like to make an effort to fight the human trafficking problem directly and invited you to meet them again in the future on Brooks if you're willing to aid them in this quest.]]) )
      misn.finish(true)
   end
end
