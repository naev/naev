--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Kidnapped">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>None</location>
 </avail>
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

sysname1 = "Arcturus"
sysname2 = "Goddard"
sysname3 = "Ogat"
sysname4 = "Delta Pavonis"
bar1 = "Praxis"
bar2 = "Seanich"
home = "Brooks"

--NPCs
local pir1_disc = _([[The two pirates seem to be talking rather quietly, but loud enough for you to overhear if you are careful.]])
local pir2_disc = _([[The pirates have both drank their wallet's worth today, so overhearing shouldn't be too much of an issue.]])

function create()
  claimsystem = {system.get("Goddard")}
  if not misn.claim(claimsystem) then
    abort()
  end

  chosen = tk.choice(_("Help!"), fmt.f(_([["Hello {player}, thank you so much for answering our hail! We really could use your help," says a haggard sounding man over the comm. "It's about our children, my wife's and mine. We were out on a family vacation to Antica, you see, and we were attacked by a gang of pirates..."
    "And he thought he could out-fly them," a woman's voice pipes up. "My husband used to be a bit of a pilot, but that was back when we were still dating. He would fly all the way to the Apez system to see me! Before we had children..." The woman trails off.
    The man quickly speaks up again. "The pirates disabled our ship and we thought we were goners, but when they boarded us, they took our three children and left us! I tried to fight them, but they had my children with knives to their necks... what was I supposed to do? So we got a tow back to Brooks, but now we need to find someone who will rescue our children. We've heard of your skills; will you please help us?"]]), {player=player.name()}), _("Rescue those children!"), _("Politely refuse"))

  if chosen == 1 then
    accept()
  else
    tk.msg(_("Another Time"), _([[You can hear that the the man is quite disappointed. "I'm sure you have a good reason to not want to help us. Perhaps you have something else more pressing..." Before the comm is cut you can hear the woman beginning to sob and the man consoling her.]]))
    abort()
  end
end

function accept()
  misn.accept()

  tk.msg(_("Thank you!"), fmt.f(_([[The two parents immediately begin thanking you quite profusely, spending a few hectoseconds simply telling you how much they truly appreciate your assistance. After a while, you realize that if these children are going to be rescued this cycle you are going to need to get started sooner rather than later. "Yes, quite right," the father replies. "No need to delay any longer than absolutely necessary. I don't know a whole lot, but you should be able to eavesdrop on some pirates at a bar. The bar in {sys} on {pnt} has been known to serve pirates occasionally, so stopping there would be a good course of action. We will anticipate your return. Again, this means so much to us." Before you know it, the two parents are at it again, thanking you like it's all they know how to do. Before it gets really bad, you bid farewell, break communication, and get on your way.]]), {sys=sysname3, pnt=bar1}))
  misn.setTitle(_("Kidnapped"))
  misn.setReward(_("A Reunited Family"))
  misn.setDesc(_([[Search for the kidnapped children, then rescue the children and return them to their parents.]]))

  misn.osdCreate(_("Kidnapped"), {fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sysname3, pnt=bar1})})

  misn_mark = misn.markerAdd(system.get(sysname3), "low")

  eavesdropped1 = false
  eavesdropped2 = false
  rescued = false

  lhook =  hook.land("land1", "land")
  hook.enter("enter")

end

function land1()
  if planet.cur() == planet.get(bar1) and not eavesdropped1 and not eavesdropped2 then
    bar1pir1 = misn.npcAdd("firstpirates", _("Pirate"), portrait.getMale("Pirate"), pir1_disc)
    bar1pir2 = misn.npcAdd("firstpirates", _("Pirate"), portrait.get("Pirate"), pir1_disc)
  end
end

function land2()
  if planet.cur() == planet.get(bar2) and eavesdropped1 and not eavesdropped2 then
    bar2pir1 = misn.npcAdd("secondpirates", _("Pirate"), portrait.get("Pirate"), pir2_disc)
    bar2pir2 = misn.npcAdd("secondpirates", _("Pirate"), portrait.get("Pirate"), pir2_disc)
  end
end

function firstpirates()
  tk.msg(_("Rumors"), fmt.f(_([[You sit down at a table adjacent to these two pirates, ordering a drink and trying to act as inconspicuous as you know how. You catch the pirates in mid-conversation. "...And he says to me 'I will give you everything, please just leave me alone!' So I take his credits, and all I get is 2K! He's clearly holding back on me, trust me, I know! So I trash his ship, and what do you know, he really didn't have any more. It's tough making any money these days, sometimes I think I gotta get into a different line o' work."
    The other pirate sitting there replies with a glint in his eye, "Actually, I heard in the bar over on {pnt} that you can make fat stacks doing a little more risky work. You just gotta nab some brats, and you can sell em for 15 big ones a pop!"
    "Human trafficking? No way man, that stuff gives me the heebie jeebies!" The other pirate replies.
    "Whatever, man."
    After a brief pause the first pirate starts talking, "So I was seeing the doctor the other day and he said that rash on my back is probably an allergic reaction."
    From that point you figure the conversation will not be picking up again, and having a lead you decide to take it.]]), {pnt=bar2}))

  misn.npcRm(bar1pir1)
  misn.npcRm(bar1pir2)

  misn.osdCreate(_("Kidnapped"), {fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sysname4, pnt=bar2})})

  misn.markerMove(misn_mark, system.get(sysname4))

  hook.rm(lhook)
  lhook = hook.land("land2", "land")

  eavesdropped1 = true
end

function secondpirates()
  tk.msg(_("The Dirt"), fmt.f(_([[You don't even bother sitting down too close to these two pirates considering how loudly they're talking. It doesn't take too much listening before you get exactly what you need as one of the pirates is telling his recent tales to the other.
    "So this dummy thought he could out-fly me in his pathetic Llama! So I took him offline in like 2 seconds, got on that ship, and took the kids. The guy tried to fight back, but I stopped that quick enough. Then the woman says they was on a vacation, like I care! Ha! Fools think they can even bother to mess with me when I have work to do.
    "So I took the kids to the {sys} system where they were loaded into this Trader Koala named the Progeny. Clever name if you ask me! No one will ever even wonder what it's carrying. It looks like the most innocent little guy flying around there. Little does everyone know it's waiting to fill up its load of brats!"
    Having listened to this dirt-bag, you feel like going over there and giving that pirate a good beating. But if you get yourself killed now you will never be able to save those children, and you don't even want to think what will happen to those children if you don't rescue them.]]), {sys=sysname2}))

  misn.npcRm(bar2pir1)
  misn.npcRm(bar2pir2)

  misn.osdCreate(_("Kidnapped"), {fmt.f(_("Fly to the {sys} system and disable (do not destroy) that Koala"),{sys=sysname2})})

  misn.markerMove(misn_mark, system.get(sysname2))

  hook.rm(lhook)

  eavesdropped2 = true
end

function enter()
   local fkidnappers = faction.dynAdd( "Pirate", "Kidnappers", _("Kidnappers"), { clear_allies=true, clear_enemies=true, ai="trader" } )

  if eavesdropped1 and eavesdropped2 and system.cur() == system.get(sysname2) and (not rescued) then
    kidnappers = pilot.add( "Koala", fkidnappers, planet.get("Zhiru"):pos() + vec2.new(-800,-800), _("Trader Koala") )
    kidnappers:rename(_("Progeny"))
    kidnappers:setHilight(true)
    kidnappers:setVisible(true)
    kidnappers:memory().aggressive = true
    kidnappers:control()
    idlehook = hook.pilot(kidnappers, "idle", "idle")
    attackhook = hook.pilot(kidnappers, "attacked", "attackedkidnappers")
    hook.pilot(kidnappers, "exploded", "explodedkidnappers")
    hook.pilot(kidnappers, "board", "boardkidnappers")
    idle()
  end
  needpirates = true
end

function idle()
  kidnappers:moveto(planet.get("Zhiru"):pos() + vec2.new( 800,  800), false)
  kidnappers:moveto(planet.get("Zhiru"):pos() + vec2.new(-800,  800), false)
  kidnappers:moveto(planet.get("Zhiru"):pos() + vec2.new(-800, -800), false)
  kidnappers:moveto(planet.get("Zhiru"):pos() + vec2.new( 800, -800), false)
end

function attackedkidnappers()

  if kidnappers:exists() then
    kidnappers:runaway(player.pilot(), true)
  end

  if needpirates then
    bodyguard1 = pilot.add( "Hyena", "Pirate", vec2.new(800, 700), _("Pirate Hyena") )
    bodyguard2 = pilot.add( "Hyena", "Pirate", vec2.new(-900, 600), _("Pirate Hyena") )
    bodyguard3 = pilot.add( "Hyena", "Pirate", vec2.new(700, -500), _("Pirate Hyena") )
    bodyguard1:control()
    bodyguard2:control()
    bodyguard3:control()
    bodyguard1:attack(player.pilot())
    bodyguard2:attack(player.pilot())
    bodyguard3:attack(player.pilot())
    bodyguard1:broadcast(_([[You are damaging the goods! You are dead!]]), true)
    needpirates = false
  end

end

function explodedkidnappers()
  if (not rescued) then
     hook.timer(1.5, "kidskilled")
  end
end

function kidskilled()
  tk.msg(_("You killed the children"), _([[Having just destroyed the kidnappers, you also just killed the children. As you sit there in space, with your head against the dash, a tear rolls down your cheek as you think of the parents and how their children are forever stolen from them. If only you could rewind and try again; you know the next time you would be more cautious. If only it were so easy...]]))
  misn.finish(false)
end

function boardkidnappers()
  tk.msg(_("You did it!"), fmt.f(_([[After disabling the ship, you take your small crew along with you and go in ready for a fight! But when you get on the small Koala you find only two men guarding it, and it turns out they are not prepared for fighting at all. They can drive a ship, but fighting is not their forte. After you tie them up, you go to the cargo hold to rescue the children. When you get there, you find a few more than three; there are probably a couple dozen! This is all probably just the tip of the iceberg, too. Either way, it's time to head back to {pnt} and reunite the parents with their children.]]), {pnt=home}))
  misn.osdCreate(_("Kidnapped"), {fmt.f(_("Return the children to the {sys} system on planet {pnt}"), {sys=sysname1, pnt=home})})
  misn.markerMove(misn_mark, system.get(sysname1))
  kidnappers:setHilight(false)
  kidnappers:hookClear()
  local c = misn.cargoNew( N_("Children"), N_("The rescued children.") )
  thekids = misn.cargoAdd( c, 0 )
  player.unboard()
  lhook = hook.land("land3", "land")
  rescued = true
  kidnappers:setVisible(false)
end

function land3()
  if planet.cur() == planet.get(home) and rescued then
    tk.msg(_("Reunited"), fmt.f(_([[As you step off the landing deck with a couple dozen children in tow, the two parents you spoke with over the comm run up to you. From behind you hear a few children yelling, "Mom! Dad!" Three children shove their way out to the front and the parents and children meet in a big group of hugs, kisses, tears, and smiles from ear to ear.
    You and the children stand off to the side and watch one of the most beautiful reunions you have ever seen. After a little while the father approaches you, wiping a tear from his cheek, and takes you in an embrace. Releasing you and drying his eyes, he says, "Thank you so much, {player}. You have no idea what this means to us. I would love to be able to repay you somehow, but I just have no idea how I can do so right now. You have rescued my children and brought them back to me. Thank you isn't enough, but I'm afraid for now it's the best I can do. If there is anything I can ever do for you, feel free to ask me, my friend."]]), {player=player.name()}))
    tk.msg(_("Reunited"), fmt.f(_([[You assure him that it is alright, and you will not hesitate to take him up on his offer. After a while the smile fades from his face. "You see how happy my family is? Well, look at all these other children here who are still separated from their parents. I want to return them all home. I'd also like to fight against all this human trafficking, but that will take a lot of planning. For now, I want to be with my family. Come back soon though if you're willing. I would like to get something organized."
    The father goes back to his children, and as you start walking back to your ship you notice the father and mother lavishing their children in love. You look over to the other children now sitting around, gazing at the loving family with envy. Sighing, you begin climbing into your ship as the mother runs up to you, "{player}, wait a second! We know your name, but you don't know ours. I'm Janice." As she looks to her husband, who's talking animatedly with the children, she smiles. "My husband's name is Andrew. Thank you for everything." A tear rolls down her face as she looks at you with her bright hazel eyes and she kisses you on the cheek.
    You watch her return to her family. A child jumps into her arms, and you climb up into your ship.]]), {player=player.name()}))
    neu.addMiscLog( _([[You successfully rescued a couple dozen children who were kidnapped by a child trafficking ring after two parents asked for your help rescuing their own children. Said parents said that they would like to make an effort to fight the human trafficking problem directly and invited you to meet them again in the future on Brooks if you're willing to aid them in this quest.]]) )
    misn.finish(true)
  end
end

function abort()
  misn.finish(false)
end
