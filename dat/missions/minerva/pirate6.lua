--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 6">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 5</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
   Assassinate a Dvaered Warlord and Za'lek General.

--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local fmt = require "format"
--local pilotai = require "pilotai"

local reward_amount = minerva.rewards.pirate6
local title = _("Limbo Mayhem")
local zlk_name = "TODO"
local dv_name = "TODO"

local mainsys = system.get("Limbo")
-- Mission states:
--  nil: mission not accepted yet
--    1. fly to position
--    2. mayhem starts
--    3. both targets down
mem.misn_state = nil

function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   misn.setDesc(_("Zuri wants you do help a Za'lek General and Dvaered Warlord finish each other off."))
   misn.setReward(_("Cold hard credits"))
   misn.setTitle( title )
end

function accept ()
   approach_zuri()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      _("Get to the position"),
      _("Eliminate the targets"),
      _("Return to Minerva Station"),
   } )
   mem.mrk_mainsys = misn.markerAdd( mainsys )

   minerva.log.pirate( fmt.f(_("You accepted a job from Zuri to assassinate both a Za'lek General and Dvaered Warlord at the same time in the {sys} system."), {sys=mainsys}) )

   hook.load("generate_npc")
   hook.land("generate_npc")
   hook.enter("enter")
   generate_npc()
end

function generate_npc ()
   if spob.cur()==spob.get("Minerva Station") then
      misn.npcAdd( "approach_zuri", minerva.zuri.name, minerva.zuri.portrait, minerva.zuri.description )
   end
end

function approach_zuri ()
   vn.clear()
   vn.scene()
   local zuri = vn.newCharacter( minerva.vn_zuri() )
   vn.music( minerva.loops.pirate )
   vn.transition()

   if mem.misn_state==3 then
      vn.na(_([[]]))
      zuri(_([[""]]))
      vn.sfxVictory()
      vn.func( function () -- Rewards
         player.pay( reward_amount )
         minerva.log.pirate(_("You took down an advanced Za'lek hacking centre and got rewarded for your efforts.") )
         faction.modPlayerSingle("Wild Ones", 5)
      end )
      vn.na(fmt.reward(reward_amount))
      vn.run()
      misn.finish(true)
   elseif  mem.misn_state==nil then
      -- Not accepted
      vn.na(_([[You approach a strangely giddy Zuri calling you to her table.]]))
      zuri(_([["It looks like we finally have a nice window of opportunity! My sources indicate that a Za'lek General and Dvaered Warlord are going to be passing through the system at roughly the same time. You know what this means."
She winks at you.]]))
      vn.menu{
         {_([["Get them to fight each other?"]]), "cont01_fight"},
         {_([["Invite them for tea and settle this peacefully?"]]), "cont01_tea"},
         {_([["No idea."]]), "cont01_noidea"},
      }

      vn.label("cont01_fight")
      zuri(_([["Exactly! Two birds with... zero stones I guess? It's perfect!"]]))
      vn.jump("cont01")

      vn.label("cont01_tea")
      zuri(_([["Exactly! Wait, no!"
She shakes her head.
"We get them to fight each other! I thought we were on the same wavelength!"]]))
      vn.jump("cont01")

      vn.label("cont01_fight")
      zuri(_([["We get them to fight each other! I thought we were on the same wavelength!"]]))
      vn.jump("cont01")

      vn.label("cont01")
      zuri(_([["Tensions at Minerva Station are at an all high, if not only a Za'lek General goes down, but a Dvaered Warlord does too, it's going to really ruffle a lot of feathers! Pretty sure the Empire will have to get involved, and if we play our cards right, the Great Houses get kicked out unceremoniously!"]]))
      zuri(_([["You game for pulling the plug on the Dvaered and Za'lek oppressors? Should be a cinch!"]]))

      vn.menu( {
         {_("Accept the job"), "accept"},
         {_("Kindly decline"), "decline"},
      } )

      vn.label("decline")
      vn.na(_("You decline their offer and take your leave."))
      vn.done()

      vn.label("accept")
      vn.func( function () mem.misn_state=0 end )
      zuri(fmt.f(_([["Great! Let me get you up to date with the plan. The Za'lek General {zlk} is set to do a rendezvous of the system soon as part of an inspection of the Za'lek border systems. Coincidently, the Dvaered Warlord {dv} is also going to be in the system after some training in a nearby system. We've pulled a few strings to make sure their timing in the {sys} system matches. Setting us up for a perfect operation."]]),
         {zlk=zlk_name, dv=dv_name, sys=mainsys}))
      zuri(_([["For this mission, I've enlisted the aid of a few helping hands. They should make sure that the mayhem gets started by launching a Za'lek drone to attack the Warlord, while simultaneously using mace rockets to draw the attention of the General. If all goes well we'll have a great nice firework display visible from Minerva Station!"]]))
      zuri(_([["However, the thing is, we have to make sure both the General and the Warlord go down, and that is where you come in! You have to make sure both the General and Warlord ships are destroyed. Do not let them get away, in particular, I would guess the Za'lek General will try to make a break for it, so make sure to break them before they can jump out of the system."]]))
      zuri(_([["With both the targets down, this will force the Houses to react, and the Empire will be forced to intervene. The ineptitude of both House Za'lek and House Dvaered, should make it unlikely that they will get a good resolution in their favour, and if we play our cards right, we can kick them out of the system."]]))
      zuri(_([["Remember, our helper will wait for you at the rendezvous location. Once you get in position, make sure to take out the targets! If all goes well, you should be able to make use of the chaos to achieve our goals! Return here so we can discuss what to do next."]]))
   else
      -- Accepted.
      vn.na(_("You approach Zuri."))
   end

   vn.label("menu_msg")
   zuri(_([["Is there anything you would like to know?"]]))
   vn.menu( function ()
      local opts = {
         {_("Ask about the job"), "job"},
         {_("Leave"), "leave"},
      }
      return opts
   end )

   vn.label("job")
   zuri(fmt.f(_([["You have to take out the Za'lek General and Dvaered Warlord that will be visiting the {sys} system. We've made sure that they should be in the system at roughly the same time. A helping hand will be there to start the mayhem, making them fight each other and you have to take advantage of the chaos to take out the priority targets."]]),
      {sys=mainsys}))
   zuri(_([["Remember, our helper will wait for you at the rendezvous location. Once you get in position, make sure to take out the targets! If all goes well, you should be able to make use of the chaos to achieve our goals! Return here so we can discuss what to do next."]]))
   vn.jump("menu_msg")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.run()
end

function enter ()
   -- Must be goal system
   if system.cur() ~= mainsys then
      return
   end
end
