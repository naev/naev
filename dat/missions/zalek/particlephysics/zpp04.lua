--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 4">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Katar I</spob>
 <location>Bar</location>
 <done>Za'lek Particle Physics 3</done>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 04

   Player has to go get some stuff from artefact hunters and gets attacked on way back
]]--
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local fleet = require "fleet"

local reward = zpp.rewards.zpp04
local cargo_name = _("strange container")
local cargo_amount = 5 -- Amount of cargo to take

local destpnt, destsys = spob.getS( "Thaddius Terminal" )
local retpnt, retsys = spob.getS( "Katar I" )

-- TODO redo the portrait
mem.shady_dealer_image, mem.shady_dealer_portrait = vni.pirate()

function create ()
   if not misn.claim( destsys ) then
      misn.finish(false)
   end
   misn.setNPC( _("Noona"), zpp.noona.portrait, zpp.noona.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You once again meet up with Noona.]]))
   n(fmt.f(_([["It's a worse setback than expected. I'm going to have to get new materials to be able to do the experiments. Since they're usually pretty hard to get, and I don't have time to fill in all the usual academic bureaucracy, I decided to go the black market route. Would you be willing to go pick up the materials at {pnt} in the {sys} system? The materials should only be {amount}, so you should not have a trouble fitting them on your ship. I would be able to pay you {credits} for your troubles this time."]]),
      {pnt=destpnt, sys=destsys, amount=fmt.tonnes(cargo_amount), credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([["OK. I'll try to figure something out…"
She furrows her brow.]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(fmt.f(_([["Thanks again! I know {pnt} is not really a place you want to go, but there really was no option this time around. By the time you get back I should have finished all the other preparations and this time I'll finally be able to complete the experiment!… I hope."]]),{pnt=destpnt}))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward(reward)
   misn.setDesc( fmt.f(_("Pick up some cargo from {pnt} in the {sys} system and deliver them to {retpnt}."),
      {pnt=destpnt, sys=destsys, retpnt=retpnt} ))

   mem.mrk = misn.markerAdd( destpnt )

   misn.osdCreate( _("Particle Physics"), {
      fmt.f(_("Pick up cargo at {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
      fmt.f(_("Deliver to {pnt} ({sys} system)"), {pnt=retpnt, sys=retsys}),
   } )

   mem.state = 1

   hook.land( "land" )
   hook.load( "land" )
   hook.enter( "enter" )
end

local npcguy
function land ()
   local pcur = spob.cur()
   if mem.state==1 and pcur==destpnt then
      npcguy = misn.npcAdd( "approach_guy", _("Shady Dealer"), mem.shady_dealer_portrait, _("A fairly shady dealer seems to be staring at you and beckoning for you to come over. Could this be the individual Noona told you to meet…?") )

   elseif mem.state==3 and pcur==retpnt then
      vn.clear()
      vn.scene()
      local n = vn.newCharacter( zpp.vn_noona() )
      vn.transition( zpp.noona.transition )
      vn.na(_([[You land and the lone cargo drone begins to unload the container. It seems to have trouble balancing it and you amuse yourself by looking at its antics until you are startled by Noona.]]))
      n(_([["The drones sure are cute. I like to call that one Laboriosi."
She points at the lone cargo drone.
"Thanks a lot for bringing me my materials. I don't know what I would do without them. I was able to go over the drones, and I think it might be best to not rely on them for the final experiment. I think the electromagnetic radiation from Katar doesn't agree too well with them. If you could help me do the experiments, I would be very grateful. Meet me up at the bar, I have to do some small preparations."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zpp.noona.transition )
      vn.run()

      faction.modPlayer("Za'lek", zpp.fctmod.zpp04)
      player.pay( reward )
      zpp.log(_("You helped Noona get some materials from a shady dealer in order for her to pursue her research and perform experiments."))
      misn.finish(true)
   end
end

local talked_once = false
function approach_guy ()
   local cargo_space = false
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( _("Shady Dealer"), {image=mem.shady_dealer_image} )
   vn.transition()
   if talked_once then
      vn(_([[You once again approach the shady dealer.]]))
      d(_([["Have you made preparations for the cargo?"
Their grin makes your feel uneasy.]]))

   else
      vn.na(_([[As you approach the shady dealer, you can barely make out some movement in the background.]]))
      d(_([[They start to grin and begin to speak with an almost serpent-like accent.
"Ah, pleasure to meet you. You must be the one in charge of the delivery. Your friend has quite a refined taste, too. It's not often we get a sample as good as this one."
They lick their lips.]]))
      d(_([["The arrangements have all been made, and you'll soon have it aboard your ship. If I were you, I would put it as far away from any personnel as possible."
They then lean it to whisper to you.
"You might want to watch your back, your friend is not the only one that wants it."]]))
      talked_once = true

   end

   local fs = player.pilot():cargoFree()
   if fs < cargo_amount then
      vn.na(fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
         {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
      vn.done()
   else
      cargo_space = true
   end

   vn.na(_("When you get back to your ship, the cargo has already been taken care of and is properly secured on your ship. As you get close to it, you hear a weird running river sound that seems to come from the cargo container. What have you gotten into?"))
   vn.run()

   if not cargo_space then
      return
   end

   local crg = commodity.new( N_("Strange Container"), N_("A large strange container that seems oddly warm to the touch. You can swear you hear weird sounds coming from inside it, almost like some sort of running river.") )
   misn.cargoAdd( crg, cargo_amount )

   misn.osdActive(2)
   mem.state = 2
   misn.markerMove( mem.mrk, retpnt )
   misn.npcRm( npcguy )
end

local badguys
function enter ()
   -- Time for !!FUN!!
   if mem.state==2 and system.cur()==destsys then
      mem.state = 3

      local pos = jump.get( destsys, "Tomas" ):pos()
      local fbad = faction.dynAdd( "Pirate", "physics_badguys", _("Thugs"), {clear_allies=true, clear_enemies=true} )

      local badships = { "Pirate Admonisher", "Pirate Hyena", "Pirate Hyena" }
      badguys = fleet.add( 1, badships, fbad, pos, _("Thugs"), {ai="baddiepos"} )

      hook.timer( 2, "heartbeat" )
   end
end

function heartbeat ()
   local bg = badguys[1]
   if not bg:exists() then
      return
   end
   local pp = player.pilot ()
   local det, scan = bg:inrange( pp )
   if det and scan and bg:pos():dist( pp:pos() ) < 5000 then
      for k,p in ipairs(badguys) do
         p:setHostile()
      end
      bg:broadcast( _("That's the one! Get 'em!") )
      player.autonavReset( 5 )
      return
   end

   hook.timer( 2, "heartbeat" )
end
