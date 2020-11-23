--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Return">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>3</priority>
   <done>The Gauntlet</done>
   <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Margot</planet>
  </avail>
  <notes>
   <campaign>Heretic</campaign>
  </notes>
 </mission>
 --]]
--[[misn title - the return]]
--[[after smuggling a small arms shipment to the an'ku system,
   the player is asked to deliver a message to a "shady character"
   on the wringer in the Suna system.]]
   
require "numstring.lua"
require "missions/sirius/common.lua"


--all the messages before the mission starts
bmsg = {}
bmsg[1] = _([[You approach the tall man and he glances up at you. You notice that some of the papers on the table are maps and as you eye the maps curiously, and the man grins and motions for you to sit.
    "Hello there," he says. "I'm sure you don't recognize me, but I certainly recognize you!. My name is Shaman, proud commander of the Nasin. You've worked for us before." The strange man and his delivery. Of course.
    "Let me tell you a bit about our organization. The Sirii describe us simply as 'heretics'. But we are so much more than that! We are the true followers of Sirichana!"]])
bmsg[2] = _([["We Nasin were at one point all part of House Sirius and believed solely in the teachings of Sirichana. We loved him, and our hearts were his. As all religions do at some point, however, the teachings of Sirichana became weighed down by the ideologies and agendas of man. Most people still accepted these teachings as straight from the mouth of Sirichana himself, but we, the Nasin, knew better.
    "We started a splinter religion, still trying to cooperate with the Sirii, but when they felt threatened by our presence, they branded us as heretics and forced us out of Sirius space. We didn't know what to do at first, but then Jan Jusi pi Lawa came to lead us. He was the one who named us, the Nasin, which means "The Way" in an old earth language."
    Shaman seems to get caught up in the moment. "It was he! He who led us to join our hands! He who led us to work together! He who led us to fight back against the oppressors! It was he! The very, the only, the True Voice of Sirichana!"
    Shaman seems to realize just exactly where he is and what he is doing. All the patrons in the bar turn their heads to your table. A group of young fellows start clapping and then degrade into laughter.]])
bmsg[3] = _([[Shaman coughs out an "excuse me" and looks at you, embarrassed. "It is wrong for me to get so caught up in such things. I suppose you'll want to know about the mission now.
    "The mission is simple. Our main base operates on %s in the %s system. I need a message delivered there. Of course, we will pay you for this service. How does %s sound? Will you do it?"]])
bmsg[4] = _([["Fantastic!" He hands you the message. "They will take care of your payment there. Thank you for aiding the true followers of Sirichana."]])

--all the messages after the player lands on the target asset
emsg = {}
emsg[1] = _([[As you land, you are once again surprised to not be greeted by anyone. After searching for a bit, you return to your ship to find that the message has been taken and a small envelope has replaced it. Inside the envelope is a note. "Our sincere apologies for missing you," it says. "As you can see, we have obtained the message, and you will also notice that a payment of %s has been deposited into your account. You have done great work for us and we appreciate your services. Please feel free to meet us at the bar sometime."]])

--random odds and ends
misn_title = _("The Return")
npc_name = _("A Tall Man")
bar_desc = _("A tall man sitting at a table littered with papers.")
misn_desc = _("Shaman of Nasin has hired you to deliver the message to %s in the %s system.")
osd = {}
osd[1] = _("Fly to %s in the %s system and deliver the message")

log_text = _([[You found out that the organization you delivered an illegal package for is Nasin, a group of followers of Sirichana who the Sirii brand as "heretics". You then delivered a message for Nasin to their base in The Wringer. You were then invited to meet the Nasin officials at the bar on The Wringer.]])


function create()
   --this mission makes no system claims
   --create some mission variables
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker") --we use this at the end.
   reward = math.floor((100000+(math.random(5,8)*2000)*(nasin_rep^1.315))*.01+.5)/.01 --using the actual reward algorithm now.
   targetasset, targetsystem = planet.get("The Wringer")
   --set the mission stuff
   misn.setTitle(misn_title)
   misn.setReward(creditstring(reward))
   misn.setNPC(npc_name, "sirius/unique/shaman")
   misn.setDesc(bar_desc)

   osd[1] = osd[1]:format(_(targetasset:name()),_(targetsystem:name()))
   misn_desc = misn_desc:format(_(targetasset:name()),_(targetsystem:name()))
end

function accept()
   tk.msg(misn_title,bmsg[1])
   tk.msg(misn_title,bmsg[2])

   local msg = bmsg[3]:format( _(targetasset:name()), _(targetsystem:name()), creditstring(reward) )
   if not tk.yesno(misn_title, msg) then
      misn.finish()
   end

   tk.msg(misn_title, bmsg[4])
   misn.accept()
   misn.setDesc(misn_desc)
   misn.markerAdd(targetsystem,"high")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   message = misn.cargoAdd("Message",0)
   hook.land("landing")
end

function landing()
   if planet.cur() == targetasset then
      tk.msg(misn_title, emsg[1]:format( creditstring(reward) ))
      player.pay(reward)
      misn.cargoRm(message)
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",5) --once again, the Nasin like the fact that we are helping the Nasin.
      var.push("heretic_misn_tracker",misn_tracker)
      srs_addHereticLog( log_text )
      misn.finish(true)
   end
end

