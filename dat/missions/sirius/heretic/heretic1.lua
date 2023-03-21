--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Return">
 <unique />
 <priority>3</priority>
 <done>The Gauntlet</done>
 <cond>faction.playerStanding("Nasin") &gt;= 0</cond>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Margot</spob>
 <notes>
   <campaign>Heretic</campaign>
 </notes>
</mission>
 --]]
--[[misn title - the return]]
--[[after smuggling a small arms shipment to the an'ku system,
   the player is asked to deliver a message to a "shady character"
   on the wringer in the Suna system.]]

local fmt = require "format"
local srs = require "common.sirius"


function create()
   --this mission makes no system claims
   --create some mission variables
   mem.nasin_rep = faction.playerStanding("Nasin")
   mem.misn_tracker = var.peek("heretic_misn_tracker") --we use this at the end.
   mem.reward = math.floor((100e3+(math.random(5,8)*2e3)*(mem.nasin_rep^1.315))*.01+.5)/.01 --using the actual reward algorithm now.
   mem.targetasset, mem.targetsystem = spob.getS("The Wringer")
   --set the mission stuff
   misn.setTitle(_("The Return"))
   misn.setReward(mem.reward)
   misn.setNPC(_("A Tall Man"), "sirius/unique/shaman.webp", _("A tall man sitting at a table littered with papers."))
   misn.setDesc(_("A tall man sitting at a table littered with papers."))
end

function accept()
   tk.msg(_("The Return"),_([[You approach the tall man and he glances up at you. You notice that some of the papers on the table are maps and as you eye the maps curiously, and the man grins and motions for you to sit.
    "Hello there," he says. "I'm sure you don't recognize me, but I certainly recognize you!. My name is Shaman, proud commander of the Nasin. You've worked for us before." The strange man and his delivery. Of course.
    "Let me tell you a bit about our organization. The Sirii describe us simply as 'heretics'. But we are so much more than that! We are the true followers of Sirichana!"]]))
   tk.msg(_("The Return"),_([["We Nasin were at one point all part of House Sirius and believed solely in the teachings of Sirichana. We loved him, and our hearts were his. As all religions do at some point, however, the teachings of Sirichana became weighed down by the ideologies and agendas of man. Most people still accepted these teachings as straight from the mouth of Sirichana himself, but we, the Nasin, knew better.
    "We started a splinter religion, still trying to cooperate with the Sirii, but when they felt threatened by our presence, they branded us as heretics and forced us out of Sirius space. We didn't know what to do at first, but then Jan Jusi pi Lawa came to lead us. He was the one who named us, the Nasin, which means "The Way" in an old earth language."
    Shaman seems to get caught up in the moment. "It was he! He who led us to join our hands! He who led us to work together! He who led us to fight back against the oppressors! It was he! The very, the only, the True Voice of Sirichana!"
    Shaman seems to realize just exactly where he is and what he is doing. All the patrons in the bar turn their heads to your table. A group of young fellows start clapping and then degrade into laughter.]]))

   local msg = fmt.f(_([[Shaman coughs out an "excuse me" and looks at you, embarrassed. "It is wrong for me to get so caught up in such things. I suppose you'll want to know about the mission now.
    "The mission is simple. Our main base operates on {pnt} in the {sys} system. I need a message delivered there. Of course, we will pay you for this service. How does {credits} sound? Will you do it?"]]), {pnt=mem.targetasset, sys=mem.targetsystem, credits=fmt.credits(mem.reward)} )
   if not tk.yesno(_("The Return"), msg) then
      return
   end

   tk.msg(_("The Return"), _([["Fantastic!" He hands you the message. "They will take care of your payment there. Thank you for aiding the true followers of Sirichana."]]))
   misn.accept()
   misn.setDesc(fmt.f(_("Shaman of Nasin has hired you to deliver the message to {pnt} in the {sys} system."), {pnt=mem.targetasset, sys=mem.targetsystem}))
   misn.markerAdd(mem.targetsystem, "high")
   misn.osdCreate(_("The Return"), {
      fmt.f(_("Fly to {pnt} in the {sys} system and deliver the message"), {pnt=mem.targetasset, sys=mem.targetsystem}),
   })
   misn.osdActive(1)
   local c = commodity.new( N_("Message"), N_("A message of seemingly high importance.") )
   mem.message = misn.cargoAdd(c,0)
   hook.land("landing")
end

function landing()
   if spob.cur() == mem.targetasset then
      tk.msg(_("The Return"), fmt.f(_([[As you land, you are once again surprised to not be greeted by anyone. After searching for a bit, you return to your ship to find that the message has been taken and a small envelope has replaced it. Inside the envelope is a note. "Our sincere apologies for missing you," it says. "As you can see, we have obtained the message, and you will also notice that a payment of {credits} has been deposited into your account. You have done great work for us and we appreciate your services. Please feel free to meet us at the bar sometime."]]), {credits=fmt.credits(mem.reward)} ))
      player.pay(mem.reward)
      misn.cargoRm(mem.message)
      mem.misn_tracker = mem.misn_tracker + 1
      faction.modPlayer("Nasin",5) --once again, the Nasin like the fact that we are helping the Nasin.
      var.push("heretic_misn_tracker", mem.misn_tracker)
      srs.addHereticLog( _([[You found out that the organization you delivered an illegal package for is Nasin, a group of followers of Sirichana who the Sirii brand as "heretics". You then delivered a message for Nasin to their base in The Wringer. You were then invited to meet the Nasin officials at the bar on The Wringer.]]) )
      misn.finish(true)
   end
end
