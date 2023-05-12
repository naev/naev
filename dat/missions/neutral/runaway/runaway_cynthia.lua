--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Runaway">
 <unique />
 <priority>4</priority>
 <chance>11</chance>
 <location>Bar</location>
  <system>Gamma Polaris</system>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
This is the "The Runaway" mission as described on the wiki.
There will be more missions to detail how you are perceived as the kidnapper of "Cynthia"
--]]
local fmt = require "format"
local neu = require "common.neutral"
local portrait = require "portrait"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

local reward = 200e3

-- Mission constants
local cargoname = N_("Cynthia")
local cargodesc = N_("A young teenager.")
local targetworld, targetworld_sys = spob.getS("Zhiru")

local npc_name = _("Cynthia")
local npc_portrait = "neutral/unique/cynthia.webp"
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   misn.setNPC( _("Young Teenager"), npc_portrait, _("A teenager sits alone at a table.") )
end

function accept ()
   --This mission does not make any system claims
   local accepted = false

   vn.clear()
   vn.scene()
   local cynthia = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   cynthia(fmt.f( _([[She looks out of place in the bar. As you approach, she seems to stiffen.
"H..H..Hi", she stutters. "My name is Cynthia. Could you give me a lift? I really need to get out of here.
I can't pay you much, just what I have on me, {credits}." You wonder who she must be to have this many credits on her person. "I need you to take me to {pnt}."
You wonder who she is, but you dare not ask. Do you accept?]]),
      {credits=fmt.credits(reward), pnt=targetworld} ) )
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("accept")
   vn.func( function () accepted = true end )
   cynthia(_([["Thank you. But we must leave now, before anyone sees me."]]))

   vn.run()

   if not accepted then return end

   misn.accept()

   misn.osdCreate(_("The Runaway"), {_("Deliver Cynthia to Zhiru in the Goddard system")})
   misn.osdActive(1)

   local c = commodity.new( cargoname, cargodesc )
   mem.cargoID = misn.cargoAdd( c, 0 )

   misn.setTitle(_("The Runaway"))
   misn.setReward( fmt.f( _("{credits} on delivery."), {credits=fmt.credits(reward)} ) )
   misn.setDesc( fmt.f( _("Deliver Cynthia safely to {pnt} in the {sys} system."), {pnt=targetworld, sys=targetworld_sys} ) )
   misn.markerAdd( targetworld )

   hook.land("land")
end

function land ()
  --If we land, check if we're at our destination
   if spob.cur() ~= targetworld then
      return
   end

   misn.cargoRm( mem.cargoID )
   player.pay( reward )

   lmisn.sfxVictory()
   vntk.msg( _("The Runaway"), _([[As you walk into the docking bay, she warns you to look out behind you.
When you look back to where she was, nothing remains but a tidy pile of credit chips and a worthless pendant.]]).."\n\n"..fmt.reward(reward) )

   neu.addMiscLog( _([[You gave a teenage girl named Cynthia a lift to Zhiru. When you got there, she suddenly disappeared, leaving behind a tidy pile of credit chips and a worthless pendant.]]) )

   misn.finish(true)
end
