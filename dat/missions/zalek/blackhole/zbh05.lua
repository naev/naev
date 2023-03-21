--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 5">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Research Post Sigma-13</spob>
 <location>Bar</location>
 <done>Za'lek Black Hole 4</done>
 <notes>
  <campaign>Za'lek Black Hole</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Black Hole 05

   Have to bring back supplies while warding off some drones here and there.
]]--
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local zbh = require "common.zalek_blackhole"
local fleet = require "fleet"


local reward = zbh.rewards.zbh05
local cargo_name = _("Special Supplies")
local cargo_amount = 150 -- Amount of cargo to take

local retpnt, retsys = spob.getS("Research Post Sigma-13")
local destpnt, destsys = spob.getS( "Thaddius Terminal" )
local atksys = system.get( "NGC-23" )

function create ()
   if not misn.claim( {retsys, atksys} ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false
   var.pop( "zach_badnoona" ) -- Just in case the player abort sand redoes it

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[Zach is browsing what looks like some charts on his cyberdeck. He notices you right away when you enter, and turns to you.]]))
   z(fmt.f(_([["It looks like Icarus is in worse shape than I thought. I've tried figuring out what it eats, but haven't really had too much luck with that. It would be great to directly ask a Soromid engineer, but not only is it unlikely they would disclose any secrets, Soromid and House Za'lek are not on the best of terms. However, I was able to get in touch with someone who told me they could get us with some bioship supplies. Would you be willing to bring the supplies over from {pnt} in the {sys} system while I try to diagnose Icarus?"]]),{pnt=destpnt, sys=destsys}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(p_("Zach", [["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(fmt.f(_([["Great! {pnt} is not really one of the best places, but they do tend to have access to pretty rare stuff, and more often than not it's easier to go through the providers there than official channels. I wouldn't be surprised if the Za'lek Chairwoman herself would want to avoid doing all the ridiculous bureaucracy!"
He chuckles.]]),{pnt=destpnt}))
   vn.menu{
      {_([["Well actually…"]]), "cont01a"},
      {_([[Say nothing about Noona.]]), "cont01b"},
   }

   vn.label("cont01a")
   z(_([["You sure about that? Isn't the Hawking radiation getting to your head? There's no way Chairwoman Sanderaite would ever get involved with such dirty dealings."]]))
   vn.func( function () var.push("zach_badnoona",true) end )

   vn.label("cont01b")
   z(_([["I'll be taking care of Icarus here. There isn't too much of a hurry, as I don't fear for Icarus's life, but it's probably better to try to get them back to shape sooner than later. Best of luck!"
You can see the glee on his face when he goes back to looking at the charts on his cyberdeck. Nothing is more addictive to a Za'lek researcher than the potential of exploring and learning new things.]]))

   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Saving Icarus") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Pick up the necessary supplies at {pnt} ({sys} system) and bring them back to Zach at {retpnt} ({retsys} system)."),
      {pnt=destpnt, sys=destsys, retpnt=retpnt, retsys=retsys} ))

   mem.mrk = misn.markerAdd( destpnt )
   mem.state = 1

   misn.osdCreate(_("Saving Icarus"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
      fmt.f(_("Return to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 and spob.cur() == destpnt then
      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end

      vntk.msg(_("Cargo Loaded"), fmt.f(_("You check into the automated cargo booth with the ID code that Zach gave you. Quickly some automated loading drones start bringing the {cargo} over and loading your ship. The entire process finishes very quickly."),{cargo=cargo_name}))

      local c = commodity.new( N_("Special Supplies"), N_("An assortment of cryofrozen bioship materials. There are many stickers indicating which side is up and to handle with care. Wait, isn't it upside down?") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2
      misn.markerMove( mem.mrk, retpnt )

   elseif mem.state==2 and spob.cur() == retpnt then

      vn.clear()
      vn.scene()
      local z = vn.newCharacter( zbh.vn_zach() )
      vn.transition( zbh.zach.transition )
      vn.na(_([[You land amidst a curious Icarus looping and dancing around your ship. Loading drones quickly begin to make work of unloading all the cargo of the ship under the watchful eyes of Icarus.]]))
      z(_([["How'd it go? … Damn, you met some hostile drones nearby? That can't be a good omen, and I wanted to think we were in the clear. At least you managed to bring all the cargo safe in one piece. Now I have to see how Icarus enjoys them."]]))
      z(_([[He goes to a nearby container and opens it up, causing a cloud of frozen gas to come rolling out.
"Very interesting. Let's give this a try."
He picks up a thick glass cylindrical cannister with metallic stoppers on both end. There seems to be a thick viscous liquid in it, reminding you of some sort of primordial soup. Without much grace, he musters all his strength and flings it towards space.]]))
      vn.sfx( zbh.sfx.bite )
      vn.na(_([[The cannister slowly rotates as it flies out the space dock. Icarus is still doing weird loops and chasing nearby drones, when suddenly it stops moving, and begins to look at the cannister. In the blink of an eye, Icarus lunges at it, opening what can only be described as a gaping maw, and crushes the cannister in a single impetuous chomp, with you and Zach staring dumbfounded.]]))
      z(_([["Holy shit! Did you just see what I did!?"
Zach looks pale and goes stiff.]]))
      z(_([["That… was…   …"]]))
      z(_([["AWESOME!"
He points to Icarus excitedly.
"Did you see that? Damn, I wish my drones could do that. Wait if…"
He seems to be seriously considering how to modify his drones.]]))
      z(_([["It looks like hopefully there will be no problem with them enjoying their treats. However, I still think we'll have to do some basic surgery to repair some of Icarus's internal damage. I think I have an idea of how to pull it off, but it's going to be tricky. Meet me at the bar when you're ready to help."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh05)
      player.pay( reward )
      zbh.log(_("You helped Zach get some supplies to help nourish the feral bioship Icarus and prepare for surgery."))
      misn.finish(true)
   end
end

function enter ()
   if mem.state==2 and system.cur() == atksys then
      -- Player will almost certainly go through these ships as shortcut isn't known yet
      local ships = {
         "Za'lek Heavy Drone",
         "Za'lek Heavy Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
         "Za'lek Light Drone",
      }
      local drones = fleet.add( 1, ships, zbh.evilpi(), jump.get(atksys, retsys):pos()*0.9, nil, {ai="baddiepos"} )
      for k,p in ipairs(drones) do
         p:memory().comm_no = _("ACCESS DENIED.")
         p:setHostile(true)
      end
   end

   if system.cur() == retsys then
      local feral = zbh.plt_icarus( retpnt:pos() + vec2.newP(300,rnd.angle()) )
      feral:rename( _("Feral Bioship") )
      feral:setFriendly(true)
      feral:setInvincible(true)
      feral:control(true)
      feral:follow( player.pilot() )
      hook.pilot( feral, "hail", "feral_hail" )
   end
end

local sfx_spacewhale = {
   zbh.sfx.spacewhale1,
   zbh.sfx.spacewhale2
}
function feral_hail ()
   local sfx = sfx_spacewhale[ rnd.rnd(1,#sfx_spacewhale) ]
   sfx:play()
   player.commClose()
end
