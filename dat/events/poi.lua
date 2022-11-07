--[[
<?xml version='1.0' encoding='utf8'?>
<event name="POI Generator">
 <location>load</location>
 <chance>100</chance>
 <priority>11</priority>
</event>
--]]
local vn = require "vn"
local vni = require "vnimage"
local poi = require "common.poi"
local fmt = require "format"

local BOARD_CHANCE = 0.05
local NPC_CHANCE = 0.05

function create ()
   hook.land( "land" )
   hook.board( "board" )
end

local npc_image, npc_prt, npc_poidata, npc_name, npc_desc
function land ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Independent"] or 0
   local fct = cur:faction()

   -- Need a generic faction
   if not fct or not fct:tags().generic then
      return
   end

   -- Need independent presence in the system
   if presence <= 0 then
      return
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      return
   end

   if poi.done() < 1 then
      return
   end

   npc_poidata = poi.generate()
   if not npc_poidata then -- failed to generate
      return
   end

   if rnd.rnd() > NPC_CHANCE then
      return
   end

   npc_image, npc_prt = vni.generic()
   npc_name = _("Tipsy Patron")
   npc_desc = _("You see a tipsy individual who seems like they have something to say.")
   evt.npcAdd( "approach_npc", npc_name, npc_prt, npc_desc, 9 )
end

function approach_npc( npcid )
   local accept = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   n(fmt.f(_([["The other day I was arguing with a pilot about weird sensor readings in the {sys} system. They claim that there has to be something there, but I call bullocks!"]]),
      {sys="#b"..npc_poidata.sys:name().."#0"}))

   vn.menu{
      { _("Inquire about the coordinates."), "accept" },
      { _("Be off."), "reject" },
   }

   vn.label("accept")
   vn.func( function () accept = true end )
   vn.na(_([[After some persistence, you manage to get the coordinates of the weird readings.]]))
   vn.done()

   vn.label("reject")
   vn.na(_("You take your leave."))
   vn.run()

   if accept then
      poi.setup( npc_poidata )
      naev.missionStart("Point of Interest")
      evt.npcRm( npcid )
   end
end

function board( p )
   local m = p:memory()
   if not m.natural then
      return
   end

   if rnd.rnd() > BOARD_CHANCE then
      return
   end

   if poi.done() < 1 then
      return
   end

   local poidata = poi.generate()
   if not poidata then -- failed to generate
      return
   end

   local accept = false

   vn.clear()
   vn.scene()
   vn.transition()
   if poidata.sys:known() then
      vn.na(fmt.f(_([[While boarding the ship, you manage to find some interesting data in the navigation log. It looks like you may be able to follow the lead to something of interest in the {sys} system. Do you wish to download the data?]]),
         {sys="#b"..poidata.sys:name().."#0"}))
   else
      vn.na(_([[While boarding the ship, you manage to find some interesting data in the navigation log. It looks like you may be able to follow the lead to something of interest in what appears to be a nearby system. Do you wish to download the data?]]))
   end

   vn.menu{
      {_("Download the data"), "accept"},
      {_("Ignore."), "leave"},
   }

   vn.label("accept")
   vn.func( function () accept = true end )
   vn.na(_([[You download the data and mark the target system on your navigation console. With nothing else to do on the derelict, you leave it behind, and return to your ship.]]))
   vn.jump("done")

   vn.label("leave")
   vn.na(_([[You ignore the information.]]))

   vn.label("done")
   vn.done()

   if accept then
      poi.setup( poidata )
      naev.missionStart("Point of Interest")
   end
end
