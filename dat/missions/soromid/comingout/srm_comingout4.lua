--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Visiting Family">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <done>A Friend's Aid</done>
   <chance>10</chance>
   <location>Bar</location>
   <faction>Soromid</faction>
  </avail>
  <notes>
   <campaign>Coming Out</campaign>
  </notes>
 </mission>
 --]]
 --[[

   Visiting Family

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--]]

require "numstring"
require "missions/soromid/common"


title = {}
text = {}

title[1] = _("Visiting Family")
text[1] = _([[Chelsea greets you as before. "Hi, %s! It's so nice to see you again!" The two of you talk about your travels once again. "So, I've got a bit of a request. Could you use they/them pronouns with me again?" You agree to the request. "Thank you," they say. "I've done some more soul-searching lately and I've found that I identify more as nonbinary than as a woman. I really appreciate it!
    "Actually, come to think of it, this would be a great excuse to go see my parents again! It's been so long since I've seen them. Say, could you take me to them in %s?"]])

text[2] = _([["Awesome! Thank you so much! So just take me to %s in the %s system, and then take me to %s in the %s system. As usual, no rush. Just as long as I get to see my parents, I'm happy!"]])

text[3] = _([["Ah, busy, eh? That's OK. Let me know later if you can do it!"]])

text[4] = _([["Oh, %s! Have you changed your mind? Can you take me to %s?"]])

text[5] = _([[You land and dock on %s, then meet up with both of Chelsea's parents. They welcome Chelsea and their mother gives them a warm hug, then releases them. Chelsea's father slightly waves, and the three of them start chatting.
    Eventually, the topic of Chelsea's gender comes up. Chelsea explains that they are nonbinary and prefer they/them pronouns similarly to when they explained it to you. Their mother says that she is proud of them and hugs them.]])

text[6] = _([[Chelsea's father shakes his head. "Look, bud, you will always be my son no matter what." You and Chelsea's mother both frown slightly. Chelsea's dad continues. "But don't you think enough is enough? You are a man. It's about time you stop pretending and start-"
    Chelsea's mother cuts him off. "That's enough of that garbage from you! Chelsea is our child, and I love them as they are, not as some fantasy of what you think they should be!"
    "He's our son!" Chelsea's father snaps back. "You're feeding into his wild imagination! He's already admitted that he's not a woman. But now he's just moving on to another fantasy! What's next? Identifying as a Soromid? Good lord!"]])

text[7] = _([[Everything goes silent for what must be mere seconds, but seems to last a period. Finally, Chelsea speaks up. "Dad... I've made good friends with some Soromid in my travels." Their father's face seems to turn red with fury, but Chelsea continues. "The Soromid are not these horrible people you've made them out to be all my life. And dad, I am not your son. I never really was your son. I will never be your son. I am a transfemme enby. End of discussion.
    "%s here has really taught me a lot about asserting myself and not letting others dictate who I am, probably without realizing it. So I'm no longer going to allow your bigoted ideas about gender define me." Chelsea pauses. "And with the help of the Soromid, I'll be getting some procedures done to help affirm who I am."]])

text[8] = _([[Chelsea's mother smiles. "I'm proud of you, sweetie," she says. "No matter what happens, always be true to yourself. You are my child, and I will always love you."
    Chelsea's father frowns. "Is this really how you want it to be?" Chelsea nods. Their father continues. "Very well, then. You're right. You are not my son." He reaches into his pocket as Chelsea's mother looks in his direction.
    Suddenly, Chelsea's mother yells out. "NO!" she shouts as she tackles her husband. That's when you see what he was reaching for: a laser gun. The two start to wrestle for control as Chelsea's mother shouts. "Run! Both of you! Get out of here!" Not needing to be told twice, you grab Chelsea's arm and run as fast as you can. Just as you make it out of view, you hear the laser gun fire.]])

text[9] = _([[With no time to lose, you dash into your ship and immediately start launch procedures just in time to see Chelsea's father appear and attempt to fire his weapon at you. As Chelsea sits next to you, shaking uncontrollably from the the stress, you make a mental note to do whatever you can to comfort them once you make it out of this situation.]])

text[10] = _([[Having spent a large portion of the trip trying to console Chelsea, you honestly feel bad about dropping them off now. Nonetheless, Chelsea insists. "Thank you for your help," they say. "I never expected it to come to this, you know? I just... I hope my mom is OK. I just hope, you know?" They start to cry and you give them a friendly hug. As you release them, they wipe the tears away from their eyes. "Well, then, I've got some work to do I take it... it looks like I'm going to have a major fight on my hands, whatever form that takes. For now I'll keep doing missions as before. You know, save up money, build up my ship... that sort of thing. I'll find you if I need you, eh?" Chelsea forces a smile, as do you, and the two of you part ways for the time being. You hope they'll be OK.]])

misn_title = _("Visiting Family")
misn_desc = _("Chelsea wants to revisit their family in %s.")
misn_reward = _("None")

npc_name = _("Chelsea")
npc_desc = _("You see Chelsea in the bar and feel an urge to say hello.")

osd_desc = {}
osd_desc[1] = _("Fly to %s in the %s system.")
osd_desc[2] = _("Fly to %s in the %s system.")
osd_desc["__save"] = true

ambush_msg = _("Don't think you'll get away that easily! Get them!")
noland_msg = _("It's too dangerous to land here right now.")

log_text = _([[You transported Chelsea, who requests they/them pronouns now, to Durea so that they could see their parents. However, Chelsea's father turned on them because of their gender identity and acceptance of the Soromid, aiming a laser gun at Chelsea before he was tackled and held back by Chelsea's mother. You didn't see what happened, but as you and Chelsea ran away, you heard a gunshot. Chelsea's father then caught up with you as you began launch procedures, attempted to fire his laser gun at your ship, and then sent a group of thugs after you as you transported Chelsea to safety.
    Traumatized, Chelsea has set off to continue doing what they were doing, but this time, they are partly doing so to prepare for the worst. They said that they will find you if they need your help again.]])


function create ()
   misplanet, missys = planet.get( "Durea" )
   misplanet2, missys2 = planet.get( "Crow" )
   if not misn.claim( missys ) then misn.finish( false ) end

   started = false

   misn.setNPC( npc_name, "soromid/unique/chelsea" )
   misn.setDesc( npc_desc )
end


function accept ()
   local txt
   if started then
      txt = text[4]:format( player.name(), misplanet:name() )
   else
      txt = text[1]:format( player.name(), misplanet:name() )
   end
   started = true

   if tk.yesno( title[1], txt ) then
      tk.msg( title[1], text[2]:format( misplanet:name(), missys:name(), misplanet2:name(), missys2:name() ) )

      misn.accept()

      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( misplanet:name() ) )
      misn.setReward( misn_reward )
      marker = misn.markerAdd( missys, "low" )

      osd_desc[1] = osd_desc[1]:format( misplanet:name(), missys:name() )
      osd_desc[2] = osd_desc[2]:format( misplanet2:name(), missys2:name() )
      misn.osdCreate( misn_title, osd_desc )

      stage = 1

      hook.enter( "enter" )
      hook.land( "land" )
   else
      tk.msg( title[1], text[3] )
      misn.finish()
   end
end


function enter ()
   player.allowSave( true )
   if stage >= 2 and system.cur() == missys then
      player.allowLand( false, noland_msg )
      hook.timer( 1000, "ambush_timer" )
   end
end


function land ()
   if stage == 1 and planet.cur() == misplanet then
      player.allowSave( false )

      tk.msg( "", text[5]:format( misplanet:name() ) )
      tk.msg( "", text[6] )
      tk.msg( "", text[7]:format( player.name() ) )
      tk.msg( "", text[8] )
      tk.msg( "", text[9] )

      stage = 2
      misn.osdActive( 2 )
      if marker ~= nil then misn.markerRm( marker ) end
      marker = misn.markerAdd( missys2, "low" )

      player.takeoff()
   elseif stage >= 2 and planet.cur() == misplanet2 then
      tk.msg( "", text[10] )
      srm_addComingOutLog( log_text )
      misn.finish( true )
   end
end


function ambush_timer ()
   local thugships = {
      "Admonisher", "Lancelot", "Lancelot", "Shark", "Shark", "Shark", "Shark",
      "Hyena", "Hyena"
   }
   local leaderthug
   for i, j in ipairs( thugships ) do
      local p = pilot.addRaw( j, "baddie", misplanet, "Comingout_thugs" )
      p:rename( "Thug " .. j )
      p:setHostile()
      p:setLeader( leaderthug )

      if i == 1 then
         leaderthug = p
      end
   end

   if stage == 2 then
      leaderthug:msg( player.pilot(), ambush_msg )
      stage = 3
   end
end
