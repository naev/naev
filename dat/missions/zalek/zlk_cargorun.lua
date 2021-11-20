--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Shipping Delivery">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>10</chance>
  <faction>Za'lek</faction>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Cargo Run. adapted from Drunkard Mission
]]--
local fmt = require "format"
local zlk = require "common.zalek"

local logan -- Non-persistent state
-- luacheck: globals closehail hail land takeoff (Hook functions passed by name)

local payment = 800e3

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC( _("Za'lek Scientist"), "zalek/unique/logan.webp", _("This Za'lek scientist seems to be looking for someone.") )  -- creates the scientist at the bar

   -- Planets
   mem.pickupWorld, mem.pickupSys  = planet.getLandable("Vilati Vilata")
   mem.delivWorld, mem.delivSys    = planet.getLandable("Oberon")
   if mem.pickupWorld == nil or mem.delivWorld == nil then -- Must be landable
      misn.finish(false)
   end
   mem.origWorld, mem.origSys      = planet.cur()

--   origtime = time.get()
end

function accept ()
   if not tk.yesno( _("Spaceport Bar"), _([[This Za'lek scientist seems to be looking for someone. As you approach, he begins to speak. "Excuse me, but do you happen to know a ship captain who can help me with something?"]]) ) then
      misn.finish()

   elseif player.pilot():cargoFree() < 20 then
      tk.msg( _("No Room"), _([[You don't have enough cargo space to accept this mission.]]) )  -- Not enough space
      misn.finish()

   else
      misn.accept()

      -- mission details
      misn.setTitle( _("Za'lek Shipping Delivery") )
      misn.setReward( _("A handsome payment.") )
      misn.setDesc( _("You agreed to help a Za'lek scientist pick up some cargo way out in the sticks. Hopefully this'll be worth it.") )

      mem.pickedup = false
      mem.droppedoff = false

      mem.marker = misn.markerAdd( mem.pickupSys, "low" )  -- pickup
      -- OSD
      misn.osdCreate( _("Za'lek Cargo Monkey"), {
         fmt.f(_("Go pick up some equipment at {pnt} in the {sys} system"), {pnt=mem.pickupWorld, sys=mem.pickupSys}),
         fmt.f(_("Drop off the equipment at {pnt} in the {sys} system"), {pnt=mem.delivWorld, sys=mem.delivSys}),
      } )

      tk.msg( _("A long haul through tough territory"), fmt.f(_([[You say that he is looking at one right now. Rather surprisingly, he laughs slightly, looking relieved. "Thank you, captain! I was hoping that you could help me. My name is Dr. Andrej Logan. The job will be a bit dangerous, but I will pay you handsomely for your services.
    "I need a load of equipment that is stuck at {pickup_pnt} in the {pickup_sys} system. Unfortunately, that requires going through the pirate-infested fringe space between the Empire, Za'lek and Dvaered. Seeing as no one can agree whose responsibility it is to clean the vermin out, the pirates have made that route dangerous to traverse. I have had a devil of a time finding someone willing to take the mission on. Once you get the equipment, please deliver it to {dropoff_pnt} in the {dropoff_sys} system. I will meet you there."]]), {pickup_pnt=mem.pickupWorld, pickup_sys=mem.pickupSys, dropoff_pnt=mem.delivWorld, dropoff_sys=mem.delivSys} ) )

      mem.landhook = hook.land ("land")
      mem.flyhook = hook.takeoff ("takeoff")
   end
end

function land ()
   if planet.cur() == mem.pickupWorld and not mem.pickedup then
      if player.pilot():cargoFree() < 20 then
         tk.msg( _("No Room"), _([[You don't have enough cargo space to accept this mission.]]) )  -- Not enough space
         misn.finish()

      else

         tk.msg( _("Deliver the Equipment"), _([[Once planetside, you find the acceptance area and type in the code Dr. Logan gave you to pick up the equipment. Swiftly, a group of droids backed by a human overseer loads the heavy reinforced crates on your ship and you are ready to go.]]) )
         local c = misn.cargoNew( N_("Equipment"), N_("Some heavy reinforced crates of equipment.") )
         mem.cargoID = misn.cargoAdd( c, 20 )
         mem.pickedup = true

         misn.markerMove( mem.marker, mem.delivSys )  -- destination

         misn.osdActive(2)  --OSD
      end
   elseif planet.cur() == mem.delivWorld and mem.pickedup and not mem.droppedoff then
      tk.msg( _("Success"), _([[You land on the Za'lek planet and find your ship swarmed by dockhands in red and advanced-looking droids rapidly. They unload the equipment and direct you to a rambling edifice for payment.
    When you enter, your head spins at a combination of the highly advanced and esoteric-looking tech so casually on display as well as the utter chaos of the facility. It takes a solid ten hectoseconds before someone comes to you asking what you need, looking quite frazzled. You state that you delivered some equipment and are looking for payment. The man types in a wrist-pad for a few seconds and says that the person you are looking for has not landed yet. He gives you a code to act as a beacon so you can catch the shuttle in-bound.]]) )
      misn.cargoRm (mem.cargoID)

      misn.markerRm(mem.marker)
      misn.osdDestroy ()

      mem.droppedoff = true
   end
end

function takeoff()
   if system.cur() == mem.delivSys and mem.droppedoff then

      logan = pilot.add( "Gawain", "Independent", player.pos() + vec2.new(-500,-500) )
      logan:rename(_("Dr. Logan"))
      logan:setFaction("Za'lek")
      logan:setFriendly()
      logan:setInvincible()
      logan:setVisplayer()
      logan:setHilight(true)
      logan:hailPlayer()
      logan:control()
      logan:moveto(player.pos() + vec2.new( 150, 75), true)
      tk.msg( _("Takeoff"), _([[You feel a little agitated as you leave the atmosphere, but you guess you can't blame the scientist for being late, especially given the lack of organization you've seen on the planet. Suddenly, you hear a ping on your console, signifying that someone's hailing you.]]) )
      mem.hailhook = hook.pilot(logan, "hail", "hail")
   end
end

function hail()
   tk.msg( _("A Small Delay"), _([["Hello again. It's Dr. Logan. I am terribly sorry for the delay. As agreed, you will be paid your fee. I am pleased by your help, captain; I hope we meet again."]]) )

--   eventually I'll implement a bonus
--   tk.msg( _("Bonus"), fmt.f(_([["For your trouble, I will add a bonus of {credits} to your fee. I am pleased by your help, captain; I hope we meet again."]]), {credits=fmt.credits(mem.bonus)} ) )

   hook.update("closehail")
   player.commClose()
end

function closehail()
   mem.bonus = 0
   player.pay( payment )
   tk.msg( _("Check Account"), fmt.f(_([[You check your account balance as he closes the comm channel to find yourself {credits} richer. A good compensation indeed. You feel better already.]]), {credits=fmt.credits(payment)} ) )
   logan:setVisplayer(false)
   logan:setHilight(false)
   logan:setInvincible(false)
   logan:hyperspace()
   faction.modPlayerSingle("Za'lek", 5)
   zlk.addMiscLog( _([[You helped a Za'lek scientist deliver some equipment and were paid generously for the job.]]) )
   misn.finish(true)
end

function abort()
   misn.finish()
end
