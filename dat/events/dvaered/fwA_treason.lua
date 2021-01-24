--[[
<?xml version='1.0' encoding='utf8'?>
 <event name="Betray General Klank">
  <trigger>enter</trigger>
  <chance>10</chance>
  <cond>system.cur():faction() == faction.get("Dvaered") and player.misnDone("Dvaered Sabotage") == true</cond>
  <flags>
   <unique />
  </flags>
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

-- TODO: at some point before, it should have been suggested that it's a bad idea to try to betray Klank
-- TODO: see chance for this event

require "missions/dvaered/frontier_war/fw_common"
require "numstring"

yesno_title = _("You are needed for a special job")
yesno_text = _([[The pilot of the fighter says, over an encrypted channel: "I have finally found you, %s. Better late than never. My employers want to congratulate you about how effective you have been with Lord Battleaddict. I am afraid there won't be many people to mourn him." You answer that you don't know what this is about, and that as far as you know, Lord Battleaddict has been killed in an honest duel by the General Klank. The interlocutor laughs "You're playing your part, eh? I can understand you, after all, they pay you well... Wait, no, they don't pay well. Not at all! How much was it for risking your life twice with this EMP bomb trick? %s? Haw haw haw! You can make better money with a cargo mission!
   "I've even heard that once, they paid you with gauss guns! Those guys are so pitiful, aren't they?
   "Now, let's talk seriously: you want money and I want a pilot. We're made to get along, you and me! I need you for a special task. I won't deny it implies going against the interests of General Klank and Major Tam and co, but if you do it well, they won't ever know that you are implicated, and you'll receive %s in the process. Oh yes, that's different from what you're used to! What do you say?"]])

yes_answer = _("Accept the offer")
no_answer =  _("Refuse and miss a unique opportunity")

accept_title = _("Your task")
accept_text = _([["Very good choice, colleague!" The pilot answers. "Go to %s, and you will be hailed by an other Vendetta for your briefing."]])

refuse_title = _("Too bad")
refuse_text = _([["I see. Stay tuned, then, maybe we will see each other again!"]])

loyal_title = _("You are a loyal citizen of House Dvaered")
loyal_text1 = _([[As you land, you see the Captain Leblanc at the dock and she congratulates you: "I've heard good things about you, citizen %s. It seems that you have passed the test. You remained loyal to our general, in spite of the absurdly high reward they had proposed to you for betraying us."]])
loyal_text2 = _([["Money matters are secondary matters, pilot. One day you are rich, and the next, you are poor. Valor, on the other hand, is the central matter of life for valor contains all the other qualities a Dvaered must have:
   "Righteousness to understand what has to be done,
   "Loyalty to know who you can trust to help you in your duty,
   "Strength to be able to do what Righteousness and Loyalty require you to do.
   "This demanding morale may require the Dvaered to risk their own lives or to kill for the community, because the philosophy of House Dvaered is a philosophy of life. And life does not come without its counterpart, death. Being a Dvaered means to accept the ultimate rule of the universe, the finitude of all things, unlike all the other factions, who hopelessly pursue eternity. Eternity in the succession of Emperors, eternity in faith, eternity in the progress of biological enhanced humanity. Their quest is doomed to fail, creating weird and ugly monsters like the Empire, House Sirius or the Soromid."]])
loyal_text3 = _([["House Dvaered has been built in respect of this ethic of life and death, that is taught to all children around all worlds in Dvaered space. You did not receive such an education and still, you passed the loyalty test. This means that Dvaered High Command can trust you. As a proof of this trust, I can reveal you what some among the army know, but was never revealed to non-Dvaered.
   "For one cycle now, the warlords have been greedily watching the Frontier planets. Since the FLF has been destroyed, Dvaered High Command does everything in its power to hold them back because we know that many other factions are waiting for us to get entangled in a war in the frontier in order to hit us. But the purpose of warlords is to invade worlds, and the role of the DHC is not to hinder that, so ultimately, we will have to allow this invasion.
   "A few decaperiods ago, General Klank had been promoted as second class general, with the task to organize this invasion. The first invasion plan that had been proposed to him was giving free rein to each warlord, allowing them to choose which planet to invade and how to proceed. However, contrary to the other generals, Klank soon realized that such a disorganized invasion, with inevitable battles between warlords, would take several periods, with a huge risk of ending up bogged down and vulnerable to attacks on other fronts."]])
loyal_text4 = _([["This is why the General Klank proposed an effective invasion plan, that requires coordinating the efforts of all the warlords, as well as supporting them with a reserve fleet directly commanded by DHC. The problem is that many warlords don't want the DHC to decide how they use their troops. The most reckless of them was Lord Battleaddict, but there are also Lady Bitterfight and Lord Jim. Now that the internal opposition to the plan has been weakened with Battleaddict's death, we will move to the diplomatic step. That is why you should expect to be summoned again by us."]])

money_answer = _("The amount of your rewards is pathetic")
loyal_answer = _("Money is not important, captain")

traitor_title = _("You tried to betray us")
traitor_text = _([[As you land, you see the Captain Leblanc at the dock and she reprimands you: "It was a trap, %s. The fact that you fell into it proves that you are disloyal. Disloyal and stupid by the way, as the trap was quite obvious if I may add. And the High Command can not afford to work with disloyal and stupid people. With regards to how well you have helped us in the past, we will exceptionally let you live, but don't expect us to trust you anymore!"]])

log_text_loyal   = _("Major Tam and Captain Leblanc, from the DHC, have tested your loyalty to the General Klank. The test has proven to be conclusive, and Leblanc revealed to you that they will soon need your services in the framework of the preparation of the invasion of the Frontier.")
log_text_traitor = _("Major Tam and Captain Leblanc, from the DHC, have tested your loyalty to the General Klank... and you miserably failed. By chance, they did not kill you, but you'll have to find other employers.")

credits = 2000000


-- Start at previous system
function create ()
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   landhook = hook.land("leave")
end

function begin ()
   hook.rm(jumphook)
   hook.rm(landhook)

   thissystem = system.cur()
   targetsys = system.get("Doranthex") -- TODO: not sure it's needed.

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if (not areAdj) then
      evt.finish(false)
   end

   vedetta = pilot.add("Dvaered Vendetta", source_system)[1]
   finish1 = hook.pilot(vedetta, "jump", "finish")
   finish2 = hook.pilot(vedetta, "death", "finish")
   finish3 = hook.land("finish")
   finish4 = hook.jumpout("finish")

   yohail = hook.timer( 4000, "hailme" );
end

function hailme()
   vedetta:hailPlayer()
   hook.pilot(vedetta, "hail", "hail")
end

-- Player answers to hail
function hail()
   player.commClose()
   local c = tk.choice(yesno_title, yesno_text:format(player.name(), creditstring(credits_01), creditstring(credits)), yes_answer, no_answer)
   if c == 1 then
      tk.msg(accept_title, accept_text:format(targetsys:name()))
      stage = 1
   elseif c == 2 then
      tk.msg(refuse_title, refuse_text)
      stage = 0
   end
   source_system = system.cur()
   landhook = hook.land("reaction")

   hook.rm(finish1)
   hook.rm(finish2)
   hook.rm(finish3)
   hook.rm(finish4)
end

-- Reaction to player's choice at first landing
function reaction()
   if stage == 1 then -- Traitor
      tk.msg(traitor_title, traitor_text:format(player.name()), portrait_leblanc)
      var.push( "loyal2klank", false )
      shiplog.createLog( "dvaered_military", _("Dvaered Military Coordination"), _("Dvaered") )
      shiplog.appendLog( "dvaered_military", log_text_traitor )
      evt.finish(true)
   else -- Loyal
      tk.choice(loyal_title, loyal_text1:format(player.name()), money_answer, loyal_answer) -- Actually we don't care of the answer
      tk.msg(loyal_title, loyal_text2, ("portraits/"..portrait_leblanc))
      tk.msg(loyal_title, loyal_text3)
      tk.msg(loyal_title, loyal_text4)
      var.push( "loyal2klank", true )
      shiplog.createLog( "frontier_war", _("Frontier War"), _("Dvaered") )
      shiplog.appendLog( "frontier_war", log_text_loyal )
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
