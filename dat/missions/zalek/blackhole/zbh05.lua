--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Black Hole 5">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <spob>Research Post Sigma-13</spob>
  <location>Bar</location>
  <done>Za'lek Black Hole 4</done>
 </avail>
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

-- luacheck: globals land enter heartbeat feral_hail (Hook functions passed by name)

local reward = zbh.rewards.zbh05
local cargo_name = _("Special Supplies")
local cargo_amount = 150 -- Amount of cargo to take

local retpnt, retsys = spob.getS("Research Post Sigma-13")
local destpnt, destsys = spob.getS( "Thaddius Station" )
local atksys = spob.get( "NGC-23" )

function create ()
   misn.finish()
   if not misn.claim( {retsys, atksys} ) then
      misn.finish()
   end

   misn.setNPC( _("Zach"), zbh.zach.portrait, zbh.zach.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local z = vn.newCharacter( zbh.vn_zach() )
   vn.transition( zbh.zach.transition )
   vn.na(_([[]]))
   z(fmt.f(_([[""]]),{}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   z(_([["OK. I'll be here if you change your mind."]]))
   vn.done( zbh.zach.transition )

   vn.label("accept")
   z(_([[""]]))
   vn.func( function () accepted = true end )
   vn.done( zbh.zach.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Saving Icarus") )
   misn.setReward( fmt.reward(reward) )
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
   if mem.state==1 and spob.cur() == mem.destpnt then
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
      vn.na(_([[]]))
      z(_([[""]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zbh.zach.transition )
      vn.run()

      faction.modPlayer("Za'lek", zbh.fctmod.zbh05)
      player.pay( reward )
      zbh.log(fmt.f(_(""),{}))
      misn.finish(true)
   end
end

function enter ()
   --if mem.state==2 and system.cur() == atksys then
   --end

   if system.cur() == retsys then
      local feral = zbh.plt_icarus( retpnt:pos() + vec2.newP(300,rnd.angle()) )
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
