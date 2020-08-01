--[[

   Coming of Age

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

require "numstring.lua"
require "dat/missions/soromid/common.lua"


title = {}
text = {}

title[1] = _("Getting My Feet Wet")
text[1] = _([[Chelsea smiles and waves as she sees you approaching. "Hi, %s! It's been a while!" You sit down and start a friendly conversation with her. She mentions that her parents seem to be supportive of her decision to transition and her mother in particular apparently has been very helpful.
    Chelsea perks up a little. "So, remember I said I had ambitions of a pilot? Well, I have my piloting license already, but I'm kind of poor so I couldn't afford my first ship. So I've been asking around and I've managed to find a great deal for a used ship at %s in the %s system! I just need someone to take me there. Again, no rush. Would you be able to do that for me?"]])

text[2] = _([["Thank you so much! I really appreciate it, %s. I can't pay you much, but I can give you %s credits when we get there. I can't wait to start!"]])

text[3] = _([["Oh, okay. Let me know later on if you're able to."]])

text[4] = _([["Oh, %s! Are you able to help me out now?"]])

landtext = _([[As you dock you can barely stop Chelsea from jumping out of your ship and hurting herself. She seems to know exactly where to go and before you even know what's going on, she's purchased an old Llama, possibly the most rusty and worn-down Llama you've ever seen, but in working order nonetheless. You express concern about the condition of the ship, but she assures you that she will fix it up as she gets enough money to do so. She hugs you in a friendly embrace, thanks you, and hands you a credit chip. "Catch up with me again sometime, okay? I'll be hanging out in Soromid space doing my first missions as a pilot!" As you walk away, you see her getting her first close-up look at the mission computer with a look of excitement in her eyes.]])

misn_title = _("Coming of Age")
misn_desc = _("Chelsea needs you to take her to %s so she can buy her first ship and kick off her piloting career.")
misn_reward = _("%s credits")

npc_name = _("Chelsea")
npc_desc = _("She seems to just be sitting by idly. It's been a while; maybe you should say hi?")

osd_desc    = {}
osd_desc[1] = _("Go to the %s system and land on the planet %s.")

log_text = _([[You helped transport Chelsea to Crow, where she was able to buy her first ship, a Llama which is in very bad condition, but working. As she went on to start her career as a freelance pilot, she asked you to catch up with her again sometime. She expects that she'll be sticking to Soromid space for the time being.]])


function create ()
   misplanet, missys = planet.get( "Crow" )
   -- Note: This mission does not make system claims

   credits = 50000
   started = false

   misn.setNPC( npc_name, "soromid/unique/chelsea" )
   misn.setDesc( npc_desc )
end


function accept ()
   local txt
   if started then
      txt = text[4]:format( player.name() )
   else
      txt = text[1]:format( player.name(), misplanet:name(), missys:name() )
   end
   started = true

   if tk.yesno( title[1], txt ) then
      tk.msg( title[1], text[2]:format( player.name(), numstring( credits ) ) )

      misn.accept()

      misn.setTitle( misn_title )
      misn.setDesc( misn_desc:format( misplanet:name() ) )
      misn.setReward( misn_reward:format( numstring( credits ) ) )
      marker = misn.markerAdd( missys, "low" )

      osd_desc[1] = osd_desc[1]:format( missys:name(), misplanet:name() )
      misn.osdCreate( misn_title, osd_desc )

      hook.land( "land" )
   else
      tk.msg( title[1], text[3] )
      misn.finish()
   end
end


function land ()
   if planet.cur() == misplanet then
      tk.msg( "", landtext )
      player.pay(credits)

      local t = time.get():tonumber()
      var.push( "comingout_time", t )

      srm_addComingOutLog( log_text )

      misn.finish(true)
   end
end
