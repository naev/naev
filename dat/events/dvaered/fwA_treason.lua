--[[
<?xml version='1.0' encoding='utf8'?>
 <event name="Betray General Klank">
  <location>enter</location>
  <chance>10</chance>
  <cond>system.cur():faction() == faction.get("Dvaered") and player.misnDone("Dvaered Sabotage") == true</cond>
  <unique />
  <notes>
   <campaign>Frontier Invasion</campaign>
   <done_misn name="Dvaered Sabotage"/>
  </notes>
 </event>
 --]]
--[[
--Event for Frontier Invasion campaign. One proposes to the player to betray Klank

   Stages:
   0) Refused
   1) Accepted
--]]

local vn = require "vn"
local fw = require "common.frontier_war"
local fmt = require "format"
local love_shaders = require "love_shaders"

-- Event constants
local credits = 2e6
local targetsys = system.get("Doranthex")

local finish, jumphook, landhook, source_system, stage, vendetta, yohail -- Non-persistent state.

-- Start at previous system
function create ()
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   landhook = hook.land("leave")
end

function begin ()
   hook.rm(jumphook)
   hook.rm(landhook)

   local thissystem = system.cur()

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   local areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if (not areAdj) then
      evt.finish(false)
   end

   vendetta = pilot.add( "Dvaered Vendetta", "Dvaered", source_system )
   finish = {}
   finish[1] = hook.pilot(vendetta, "jump", "finish")
   finish[2] = hook.pilot(vendetta, "death", "finish")
   finish[3] = hook.land("finish")
   finish[4] = hook.jumpout("finish")

   yohail = hook.timer( 4.0, "hailme" )
end

function hailme()
   vendetta:hailPlayer()
   hook.pilot(vendetta, "hail", "hail")
end

-- Player answers to hail
function hail()
   player.commClose()
   
   vn.clear()
   vn.scene()
   local vendettaVN = vn.newCharacter( _("Vendetta pilot"), { image="neutral/male1n", shader=love_shaders.hologram() } )
   vn.transition()
   
   vendettaVN(fmt.f(_([["I have finally found you, {player}. Better late than never. My employers want to congratulate you about how effective you have been with Lord Battleaddict. I am afraid there won't be many people to mourn him."]]), {player=player.name()}))
   vn.menu{
      {_([[Stay silent]]), "silent"},
      {_([["I agree I did pretty well with Lord Battleaddict."]]), "brag"},
      {_([["I don't understand what you are talking about. Lord Battleaddict has been killed in an honest duel by the General Klank."]]), "secret"},
   }
   
   vn.label("brag")
   vendettaVN(_([["I see… You're not the sharpest knife in the drawer, are you? You were supposed to deny, remember? Not to confirm this duel was fixed. I suppose you're the best they can afford given how lame their rewards are."]]))
   vn.jump("silent")
   
   vn.label("secret")
   vendettaVN(_([["You're playing your part, eh? I can understand you, after all, they pay you well… Wait, no, they don't pay well. Not at all!"]]))
   
   vn.label("silent")
   vendettaVN(fmt.f(_([["How much was it for risking your life twice with this EMP bomb trick? {credits_01}? Haw haw haw! You can make better money with a cargo mission!
   "I've even heard that once, they paid you with gauss guns! Those guys are so pitiful, aren't they?"]]), { credits_01=fmt.credits(fw.credits_01) }))
   vn.menu{
      {_([[Stay silent]]), "proceed"},
      {_([["You nailed it: they are stingy mission givers."]]), "proceed"},
      {_([["Look, mate. I have no idea what you're talking about."]]), "proceed"},
      {_([["All I want to do is to serve House Dvaered."]]), "dvaered"},
      {_([["All I want to do is to have fun blowing stuff up."]]), "dvaered"},
   }
   
   vn.label("dvaered")
   vendettaVN(_([["Yeah. Whatever. You're not very subtle, are you?"]]))
   
   vn.label("proceed")
   vendettaVN(fmt.f(_([["Now, let's talk seriously: you want money and I want a pilot. We're made to get along, you and me! I need you for a special task. I won't deny it implies going against the interests of General Klank and Major Tam and co, but if you do it well, they won't ever know that you are implicated, and you'll receive {credits} in the process. Oh yes, that's different from what you're used to! What do you say?"]]), { credits=fmt.credits(credits) }))
   vn.menu{
      {_([[Accept the offer]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }
   
   vn.label("refuse")
   vendettaVN(_([["I see. Stay tuned, then, maybe we will see each other again!"]]))
   vn.func( function () stage = 0 end )
   vn.done()
   
   vn.label("accept")
   vendettaVN(fmt.f(_([["Very good choice, colleague! Go to {system}, and you will be hailed by another Vendetta for your briefing."]]), {system=targetsys} ) )
   vn.func( function () stage = 1 end )
   vn.done()
   vn.run()
   
   source_system = system.cur()
   landhook = hook.land("reaction")

   for i = 1,4 do
      hook.rm(finish[i])
   end
end

-- Reaction to player's choice at first landing
function reaction()
   if stage == 1 then -- Traitor
      vn.clear()
      vn.scene()
      local leblancVN = vn.newCharacter( fw.vn_char( fw.leblanc ) )
      vn.transition()
      vn.na(_([[As you land, you see the Captain Leblanc at the dock and she reprimands you.]]))
      leblancVN(fmt.f(_([["It was a trap, {player}. The fact that you fell into it proves that you are disloyal. Disloyal and stupid by the way, as the trap was quite obvious if I may add. And the High Command can not afford to work with disloyal and stupid people. With regards to how well you have helped us in the past, we will exceptionally let you live, but don't expect us to trust you anymore!"]]), {player=player.name()}))
      vn.done()
      vn.run()

      var.push( "loyal2klank", false )
      shiplog.create( "dvaered_military", _("Dvaered Military Coordination"), _("Dvaered") )
      shiplog.append( "dvaered_military", _("Major Tam and Captain Leblanc, from the DHC, have tested your loyalty to the General Klank… and you miserably failed. By chance, they did not kill you, but you'll have to find other employers.") )
      evt.finish(true)
      
   else -- Loyal
      vn.clear()
      vn.scene()
      local leblancVN = vn.newCharacter( fw.vn_char( fw.leblanc, {pos="left"} ) )
      vn.transition()
      vn.na(_([[As you land, you see the Captain Leblanc at the dock and she congratulates you.]]))
      leblancVN(fmt.f(_([["I've heard good things about you, citizen {player}. You have passed the loyality test. You remained loyal to our general, in spite of the absurdly high reward they had proposed to you for betraying us."]]), {player=player.name()}))
      vn.menu{
         {_([[Stay silent]]), "proceed"},
         {_([["Actually, this raised a relevant point: can we speak of the fact that the amount of your rewards is pathetic?"]]), "money"},
         {_([["Money is not important to me, captain."]]), "money"},
      }
      
      vn.label("money")
      leblancVN(_([["Money matters are secondary matters, pilot. One day you are rich, and the next, you are poor. And anyways, it is better to be poor and loyal, than to be rich and dishonored."]]))
      vn.menu{
         {_([[Stay silent]]), "proceed"},
         {_([["What about being rich and loyal?"]]), "rich"},
         {_([["Whatever, when do we start blowing stuff up?"]]), "dvaered"},
      }
      
      vn.label("rich")
      leblancVN(_([["Don't try to confuse me, citizen. I mean it is better to be rich and loyal, than to be poor and dishonored…"]]))
      leblancVN(_([["Wait… That's not what I meant."]]))
      leblancVN(_([["It is better to be dishonored and… No… Wait… It is worse to be rich than…"]]))
      leblancVN(_([["Wrong again… Anyways."]]))
      vn.jump("proceed")
      
      vn.label("dvaered")
      leblancVN(_([["Yes, that's the spirit. But listen:"]]))
      
      vn.label("proceed")
      leblancVN(_([["Valour is the central matter of life for valour contains all the other qualities a Dvaered must have:
   "Righteousness to understand what has to be done,
   "Loyalty to know who you can trust to help you in your duty,
   "Strength to be able to do what Righteousness and Loyalty require you to do.
   "This demanding morale may require the Dvaered to risk their own lives or to kill for the community, because the philosophy of House Dvaered is a philosophy of life. And life does not come without its counterpart, death. Being a Dvaered means to accept the ultimate rule of the universe, the finitude of all things, and to struggle to build the best present and future despite this. Unlike all the other factions, who hopelessly pursue eternity, with no regard for what makes us human. Eternity in the succession of Emperors, eternity in faith, eternity in the progress of biological enhanced humanity. Their quest is doomed to fail, creating weird and ugly monsters like the Empire, House Sirius or the Soromid."]]))
      leblancVN(_([["House Dvaered has been built in respect of this ethic of life and death, that is taught to all children around all worlds in Dvaered space. You did not receive such an education and still, you passed the loyalty test. This means that Dvaered High Command can trust you. As a proof of this trust, I can reveal you what some among the army know, but was never revealed to non-Dvaered.
   "For one cycle now, the warlords have been greedily watching the Frontier planets. Since the FLF has been destroyed, Dvaered High Command does everything in its power to hold them back because we know that many other factions are waiting for us to get entangled in a war in the Frontier in order to hit us. But the purpose of warlords is to invade worlds, and the role of the DHC is not to hinder that, so ultimately, we will have to allow this invasion.
   "A few decaperiods ago, General Klank had been promoted as Major General, with the task to organize this invasion. The first invasion plan that had been proposed to him was giving free rein to each warlord, allowing them to choose which planet to invade and how to proceed. However, contrary to the other generals, Klank soon realized that such a disorganized invasion, with inevitable battles between warlords, would take several periods, with a huge risk of ending up bogged down and vulnerable to attacks on other fronts."]]))
      leblancVN("This is why the General Klank proposed an effective invasion plan, that requires coordinating the efforts of all the warlords, as well as supporting them with a reserve fleet directly commanded by DHC. The problem is that many warlords don't want the DHC to decide how they use their troops. The most reckless of them was Lord Battleaddict, but there are also Lady Bitterfight and Lord Jim. Now that the internal opposition to the plan has been weakened with Battleaddict's death, we will move to the diplomatic step. That is why you should expect to be summoned again by us.")
      
      vn.done()
      vn.run()

      var.push( "loyal2klank", true )
      shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
      shiplog.append( "frontier_war", _("Major Tam and Captain Leblanc, from the DHC, have tested your loyalty to the General Klank. The test has proven to be conclusive, and Leblanc revealed to you that they will soon need your services in the framework of the preparation of the invasion of the Frontier.") )
      evt.finish(true)
   end
end

function finish()
   hook.rm(yohail)
   evt.finish(false)
end
function leave()
   evt.finish(false)
end
