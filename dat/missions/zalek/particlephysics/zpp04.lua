--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 4">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Katar I</planet>
  <location>Bar</location>
  <done>Za'lek Particle Physics 3</done>
 </avail>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 04

   Player has to go get some stuff from artefact hunters and gets attacked on way back
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local portrait = require "portrait"
local fleet = require "fleet"

-- luacheck: globals land approach_guy enter heartbeat (Hook functions passed by name)

local reward = zpp.rewards.zpp04
local cargo_name = _("nebula artefact")
local cargo_amount = 5 -- Amount of cargo to take

local destpnt, destsys = planet.getS( "Thaddius Station" )
local retpnt, retsys = planet.getS( "Katar I" )

-- TODO redo the portrait
mem.shady_dealer = portrait.get("Pirate")

function create ()
   misn.finish(false)
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
   vn.na(_([[]]))
   n(fmt.f(_([[]]),
      {pnt=destpnt, sys=destsys, amount=fmt.tonnes(cargo_amount), credits=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(fmt.f(_([["OK. I'll try to figure something out…"
She furrows her brow.]]),{pnt=destpnt}))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([[""]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc( fmt.f(_("Pick up some {cargo} from {pnt} in the {sys} system and deliver them to {retpnt}."),
      {cargo=cargo_name, pnt=destpnt, sys=destsys, retpnt=retpnt} ))

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
   local pcur = planet.cur()
   if mem.state==1 and pcur==destpnt then
      npcguy = misn.npcAdd( "approach_guy", _("Shady Dealer"), mem.shady_dealer, _("A fairly shady dealer seems to be staring at you and beckoning for you to come over. Could this be the individual Noona told you to meet…?") )

   elseif mem.state==3 and pcur==retpnt then
      vn.clear()
      vn.scene()
      local n = vn.newCharacter( zpp.vn_noona() )
      vn.transition( zpp.noona.transition )
      vn.na(_([[]]))
      n(_([[""]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.done( zpp.noona.transition )
      vn.run()

      player.pay( reward )
      zpp.log(_(""))
      misn.finish(true)
   end
end

local talked_once = false
function approach_guy ()
   local cargo_space = false
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( _("Shady Dealer"), {image=portrait.getFullPath(mem.shady_dealer)} )
   vn.transition()
   vn.na(_([[]]))
   if talked_once then
      d(_([[""]]))

   else
      d(_([[""]]))
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

   vn.na(fmt.f(_(""),{amount=fmt.tonnes(cargo_amount), cargo=cargo_name}))
   vn.run()

   if not cargo_space then
      return
   end

   local crg = misn.cargoNew( N_(""), N_("") )
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

      local badships = { "Pirate Admonisher", "Hyena", "Hyena" }
      badguys = fleet.add( 1, badships, fbad, pos, _("Thugs"), {ai="baddiepos"} )
      for k,p in ipairs(badguys) do
         p:setHostile()
      end

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
   if det and scan then
      bg:broadcast( _("That's the one! Get 'em!") )
      player.autonavReset( 5 )
      return
   end

   hook.time( 2, "heartbeat" )
end
