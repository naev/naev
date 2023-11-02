--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Shipping Delivery">
 <unique />
 <priority>4</priority>
 <chance>10</chance>
 <faction>Za'lek</faction>
 <cond>return require("misn_test").reweight_active()</cond>
 <location>Bar</location>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Cargo Run. adapted from Drunkard Mission

   Normal cargo delivery except for no payment after delivery and the player has to hail logan to get paid
]]--
local fmt = require "format"
local zlk = require "common.zalek"
local vn = require "vn"
local vntk = require "vntk"
local ccomm = require "common.comm"
local portrait = require "portrait"

local logan -- Non-persistent state

local payment = 400e3
local cargo_space = 20

local npc_name = _("Logan")
local npc_portrait = "zalek/unique/logan.webp"
local npc_image = portrait.getFullPath( npc_portrait )

function create ()
   misn.setNPC( _("Za'lek Scientist"), npc_portrait, _("This Za'lek scientist seems to be looking for someone.") )  -- creates the scientist at the bar

   -- Planets
   mem.pickupWorld, mem.pickupSys  = spob.getS("Vilati Vilata")
   mem.delivWorld, mem.delivSys    = spob.getS("Oberon")
   -- Have to claim so Logan doesn't get cleared upon takeoff at destination
   if not misn.claim( mem.delivSys ) then
      misn.finish(false)
   end

   if mem.pickupWorld == nil or mem.delivWorld == nil then -- Must be landable
      misn.finish(false)
   end
   mem.origWorld, mem.origSys      = spob.cur()

--   origtime = time.get()
end

function accept ()
   local accepted = false

   vn.clear()
   vn.clear()
   vn.scene()
   local vnlogan = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()

   vnlogan(_([[This Za'lek scientist seems to be looking for someone. As you approach, he begins to speak. "Excuse me, but do you happen to know a ship captain who can help me with something?"]]))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("nospace")
   vn.na(fmt.f(_([[You need at least {amount} of free cargo space to accept this mission.]]),
      {amount=fmt.tonnes(cargo_space)}))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_space then
         vn.jump("nospace")
         return
      end
      accepted = true
   end )
   vnlogan(_([[You say that he is looking at one right now. Rather surprisingly, he laughs slightly, looking relieved. "Thank you, captain! I was hoping that you could help me. My name is Dr. Andrej Logan. The job will be a bit dangerous, but I will pay you handsomely for your services."]]))
   vnlogan(fmt.f(_([["I need a load of equipment that is stuck at {pickup_pnt} in the {pickup_sys} system. Unfortunately, that requires going through the pirate-infested fringe space between the Empire, Za'lek and Dvaered. Seeing as no one can agree whose responsibility it is to clean the vermin out, the pirates have made that route dangerous to traverse. I have had a devil of a time finding someone willing to take the mission on. Once you get the equipment, please deliver it to {dropoff_pnt} in the {dropoff_sys} system. I will meet you there."]]),
      {pickup_pnt=mem.pickupWorld, pickup_sys=mem.pickupSys, dropoff_pnt=mem.delivWorld, dropoff_sys=mem.delivSys}))

   vn.run()

   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Za'lek Shipping Delivery") )
   misn.setReward( _("A handsome payment.") )
   misn.setDesc( _("You agreed to help a Za'lek scientist pick up some cargo way out in the sticks. Hopefully this'll be worth it.") )

   mem.pickedup = false
   mem.droppedoff = false

   mem.marker = misn.markerAdd( mem.pickupWorld, "low" )  -- pickup
   -- OSD
   misn.osdCreate( _("Za'lek Cargo Jockey"), {
      fmt.f(_("Go pick up some equipment at {pnt} in the {sys} system"), {pnt=mem.pickupWorld, sys=mem.pickupSys}),
      fmt.f(_("Drop off the equipment at {pnt} in the {sys} system"), {pnt=mem.delivWorld, sys=mem.delivSys}),
   } )

   mem.landhook = hook.land ("land")
   mem.flyhook = hook.takeoff ("takeoff")
end

function land ()
   if spob.cur() == mem.pickupWorld and not mem.pickedup then
      if player.pilot():cargoFree() < cargo_space then
         vntk.msg( _("No Room"), fmt.f(_([[You need at least {amount} of free cargo space to pick up the cargo.]]),
            {amount=fmt.tonnes(cargo_space)}))
         return

      else
         vntk.msg( _("Deliver the Equipment"), _([[Once planetside, you find the acceptance area and type in the code Dr. Logan gave you to pick up the equipment. Swiftly, a group of droids backed by a human overseer loads the heavy reinforced crates on your ship and you are ready to go.]]) )
         local c = commodity.new( N_("Equipment"), N_("Some heavy reinforced crates of equipment.") )
         mem.cargoID = misn.cargoAdd( c, cargo_space )
         mem.pickedup = true

         misn.markerMove( mem.marker, mem.delivWorld )  -- destination

         misn.osdActive(2)  --OSD
      end
   elseif spob.cur() == mem.delivWorld and mem.pickedup and not mem.droppedoff then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[You land on the Za'lek planet and find your ship swarmed by dockhands in red and advanced-looking droids rapidly. They unload the equipment and direct you to a rambling edifice for payment.]]))
      vn.na(_([[When you enter, your head spins at a combination of the highly advanced and esoteric-looking tech so casually on display as well as the utter chaos of the facility. It takes a solid ten hectoseconds before someone comes to you asking what you need, looking quite frazzled. You state that you delivered some equipment and are looking for payment. The man types in a wrist-pad for a few seconds and says that the person you are looking for has not landed yet. He gives you a code to act as a beacon so you can catch the shuttle in-bound.]]))
      vn.run()

      misn.cargoRm (mem.cargoID)
      misn.markerRm(mem.marker)
      misn.osdDestroy ()

      mem.droppedoff = true
   end
end

function takeoff()
   if system.cur() == mem.delivSys and mem.droppedoff then
      logan = pilot.add( "Gawain", "Independent", player.pos() + vec2.new(-500,-500), _("Dr. Logan") )
      logan:setFaction("Za'lek")
      logan:setFriendly()
      logan:setInvincible()
      logan:setVisplayer()
      logan:setHilight(true)
      logan:hailPlayer()
      logan:control()
      logan:moveto(player.pos() + vec2.new( 150, 75), true)
      vntk.msg( _("Takeoff"), _([[You feel a little agitated as you leave the atmosphere, but you guess you can't blame the scientist for being late, especially given the lack of organization you've seen on the planet. Suddenly, you hear a ping on your console, signifying that someone's hailing you.]]) )
      mem.hailhook = hook.pilot(logan, "hail", "hail")

      -- TODO probably handle the case the player ignores Logan
   end
end

function hail()
   vn.clear()
   vn.scene()
   local vnlogan = ccomm.newCharacter( vn, logan )
   vnlogan(_([["Hello again. It's Dr. Logan. I am terribly sorry for the delay. As agreed, you will be paid your fee. I am pleased by your help, captain; I hope we meet again."]]))
   vn.func( function ()
      player.pay( payment )
      faction.modPlayerSingle("Za'lek", 5)
   end )
   vn.sfxVictory()
   vn.na(fmt.f(_([[You check your account balance as he closes the comm channel to find yourself {credits} richer. A good compensation indeed. You feel better already.]]),
      {credits=fmt.credits(payment)} ) .."\n\n"..fmt.reward(payment) )
   vn.run()

--   eventually I'll implement a bonus
--   vntk.msg( _("Bonus"), fmt.f(_([["For your trouble, I will add a bonus of {credits} to your fee. I am pleased by your help, captain; I hope we meet again."]]), {credits=fmt.credits(mem.bonus)} ) )

   mem.bonus = 0
   logan:setVisplayer(false)
   logan:setHilight(false)
   logan:setInvincible(false)
   logan:hyperspace()
   zlk.addMiscLog( _([[You helped a Za'lek scientist deliver some equipment and were paid generously for the job.]]) )

   player.commClose()
   misn.finish(true)
end
