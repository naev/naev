--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Coming of Age">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <done>Coming Out</done>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Durea</planet>
  <cond>var.peek("comingout_time") == nil or time.get() &gt;= time.fromnumber(var.peek("comingout_time")) + time.create(0, 20, 0)</cond>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
 </notes>
</mission>
--]]
--[[

   Coming of Age

--]]
local fmt = require "format"
local srm = require "common.soromid"


function create ()
   misplanet, missys = planet.getS( "Crow" )
   -- Note: This mission does not make system claims

   credits = 50e3
   started = false

   misn.setNPC( _("Chelsea"), "soromid/unique/chelsea.webp", _("She seems to just be sitting by idly. It's been a while; maybe you should say hi?") )
end


function accept ()
   local txt
   if started then
      txt = fmt.f( _([["Oh, {player}! Are you able to help me out now?"]]), {player=player.name()} )
   else
      txt = fmt.f( _([[Chelsea smiles and waves as she sees you approaching. "Hi, {player}! It's been a while!" You sit down and start a friendly conversation with her. She mentions that her parents seem to be supportive of her decision to transition and her mother in particular apparently has been very helpful.
    Chelsea perks up a little. "So, remember I said I had ambitions of a pilot? Well, I have my piloting license already, but I'm kind of poor so I couldn't afford my first ship. So I've been asking around and I've managed to find a great deal for a used ship at {pnt} in the {sys} system! I just need someone to take me there. Again, no rush. Would you be able to do that for me?"]]), {player=player.name(), pnt=misplanet, sys=missys} )
   end
   started = true

   if tk.yesno( _("Getting My Feet Wet"), txt ) then
      tk.msg( _("Getting My Feet Wet"), fmt.f(_([["Thank you so much! I really appreciate it, {player}. I can't pay you much, but I can give you {credits} when we get there. I can't wait to start!"]]), {player=player.name(), credits=fmt.credits(credits)} ) )

      misn.accept()

      misn.setTitle( _("Coming of Age") )
      misn.setDesc( fmt.f( _("Chelsea needs you to take her to {pnt} so she can buy her first ship and kick off her piloting career."), {pnt=misplanet} ) )
      misn.setReward( fmt.credits( credits ) )
      marker = misn.markerAdd( missys, "low" )

      misn.osdCreate( _("Coming of Age"), {
         fmt.f( _("Go to the {sys} system and land on the planet {pnt}."), {sys=missys, pnt=misplanet} ),
      } )

      hook.land( "land" )
   else
      tk.msg( _("Getting My Feet Wet"), _([["Oh, okay. Let me know later on if you're able to."]]) )
      misn.finish()
   end
end


function land ()
   if planet.cur() == misplanet then
      tk.msg( "", _([[As you dock you can barely stop Chelsea from jumping out of your ship and hurting herself. She seems to know exactly where to go and before you even know what's going on, she's purchased an old Llama, possibly the most rusty and worn-down Llama you've ever seen, but in working order nonetheless. You express concern about the condition of the ship, but she assures you that she will fix it up as she gets enough money to do so. She hugs you in a friendly embrace, thanks you, and hands you a credit chip. "Catch up with me again sometime, okay? I'll be hanging out in Soromid space doing my first missions as a pilot!" As you walk away, you see her getting her first close-up look at the mission computer with a look of excitement in her eyes.]]) )
      player.pay(credits)

      local t = time.get():tonumber()
      var.push( "comingout_time", t )

      srm.addComingOutLog( _([[You helped transport Chelsea to Crow, where she was able to buy her first ship, a Llama which is in very bad condition, but working. As she went on to start her career as a freelance pilot, she asked you to catch up with her again sometime. She expects that she'll be sticking to Soromid space for the time being.]]) )

      misn.finish(true)
   end
end
