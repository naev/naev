--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Deal with the FLF agent">
 <unique />
 <priority>2</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="FLF/DV Derelicts">If you choose to help Gregar</done_evt>
  <campaign>Join the FLF</campaign>
 </notes>
</mission>
--]]
--[[
   This is the first "prelude" mission leading to the FLF campaign. The player takes a FLF agent onboard, then either turns him in to the Dvaered or delivers him to a hidden FLF base.
   stack variable flfbase_intro:
       1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
       2 - The player has rescued the FLF agent. Conditional for flf_pre02
       3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03
--]]

local fleet = require "fleet"
local flf = require "missions.flf.flf_common"
local fmt = require "format"
local dv = require "common.dvaered"

local fleetFLF -- Non-persistent state

function create()
   local missys = {system.get(var.peek("flfbase_sysname"))}
   if not misn.claim(missys) then
      abort() -- TODO: This claim should be in the event that starts this mission!
   end
   faction.get("FLF"):setKnown(true)

   misn.accept() -- The player chose to accept this mission by boarding the FLF ship

   mem.flfdead = false -- Flag to check if the player destroyed the FLF sentinels
   mem.basefound = false -- Flag to keep track if the player has seen the base

   mem.destsys = system.get(var.peek("flfbase_sysname"))

   tk.msg(_("Gregar joins the party"), fmt.f( _([[A haggard-looking man emerges from the airlock. He says, "Thank goodness you're here. My name is Gregar, I'm with the Frontier Liberation Front. I mean you no harm." He licks his lips in hesitation before continuing. "I have come under attack from a Dvaered patrol. I wasn't violating any laws, and we're not even in Dvaered territory! Anyway, my ship is unable to fly."
   You help Gregar to your cockpit and install him in a vacant seat. He is obviously very tired, but he forces himself to speak. "Listen, I was on my way back from a mission when those Dvaered bastards jumped me. I know this is a lot to ask, but I have little choice seeing how my ship is a lost cause. Can you take me the rest of the way? It's not far. We have a secret base in the {sys} system. Fly there and contact my comrades. They will take you the rest of the way."
   Gregar nods off, leaving you to decide what to do next. Gregar wants you to find his friends, but harboring a known terrorist, let alone helping him, might not be looked kindly upon by the authorities...]]), {sys=mem.destsys}))

   misn.osdCreate(_("Deal with the FLF agent"), {
      fmt.f( _("Take Gregar, the FLF agent, to the {sys} system and make contact with the FLF"), {sys=mem.destsys}),
      _("Alternatively, turn Gregar in to the nearest Dvaered base"),
   })
   misn.setDesc(_("You have taken onboard a member of the FLF. You must either take him where he wants to go, or turn him in to the Dvaered."))
   misn.setTitle(_("Deal with the FLF agent"))
   misn.setReward(_("A chance to learn more about the FLF"))
   misn.markerAdd(mem.destsys, "low")

   local c = commodity.new( N_("Gregar"), N_("A rescued FLF pilot.") )
   mem.gregar = misn.cargoAdd( c, 0 )

   hook.enter("enter")
   hook.land("land")
end

-- Handle the FLF encounter, Gregar's intervention, and ultimately the search for the base.
function enter()
   if system.cur() == mem.destsys and not mem.flfdead and not mem.basefound then
      -- Collect information needed for course calculations
      local spread = 45 -- max degrees off-course for waypoints
      local basepos = vec2.new(-8700,-3000) -- NOTE: Should be identical to the location in asset.xml!
      local jumppos = jump.pos( system.cur(), "Behar" )

      -- Calculate course
      local dist = vec2.dist(basepos, jumppos) -- The distance from the jump point to the base
      local course = basepos - jumppos -- The vector from the jump point to the base
      local cx, cy = course:get() -- Cartesian coordinates of the course
      local courseangle = math.atan2(cy, cx) -- Angle of the course in polar coordinates

      -- Generate new angles deviating off the courseangle
      local angle2 = courseangle + (rnd.rnd() - 0.5) * 2 * spread
      local angle1 = courseangle + (rnd.rnd() - 0.5) * 2 * spread

      -- Set FLF base waypoints
      -- Base is at -8700,-3000
      -- Waypoints deviate off the course by at most spread degrees
      mem.waypoint2 = jumppos + vec2.newP(dist / 3, angle2)
      mem.waypoint1 = jumppos + course * 1 / 3 + vec2.newP(dist / 3, angle1)
      mem.waypoint0 = basepos -- The base will not be spawned until the player is close to this waypoint.

      pilot.toggleSpawn(false)
      pilot.clear()

      -- Add FLF ships that are to guide the player to the FLF base (but only after a battle!)
      fleetFLF = fleet.add(3, "Vendetta", "FLF", jumppos, nil, {ai="flf_norun"})
      local c = player.pilot():ship():class()
      if c == "Cruiser" or c == "Carrier" or c == "Battleship" then
         local p = pilot.add( "Pacifier", "FLF", jumppos, nil, {ai="flf_norun"} )
         fleetFLF[#fleetFLF + 1] = p
      end

      faction.get("FLF"):setPlayerStanding( -100 )

      hook.timer(2.0, "commFLF")
      hook.timer(15.0, "wakeUpGregarYouLazyBugger")
   end
end

-- There are two cases we need to check here: landing on the FLF base and landing on a Dvaered world.
function land()
   -- Case FLF base
   if diff.isApplied("FLF_base") and spob.cur() == spob.get("Sindbad") then
      tk.msg(_("Gregar leaves the party"), fmt.f( _([[You and Gregar step out of your airlock and onto Sindbad. You are greeted by a group of five or six FLF soldiers. They seem relieved to see Gregar, but they clearly regard you with distrust. You are taken to meet with a senior officer of the base. Gregar doesn't come with you, as he seems to have urgent matters to attend to - away from prying ears like your own.
   "Alright, {player}," the officer begins. "I don't know who you are or what you think you're doing here, but you shouldn't kid yourself. The only reason you are in my office and not in a holding cell is because one of my trusted colleagues is vouching for you." The officer leans a little closer to you and pins you with a level stare. "I don't think you're a Dvaered spy. The Dvaered don't have the wit to pull off decent espionage. But you shouldn't get any ideas of running to the Dvaered and blabbing about our presence here. They're neither a trusting nor a grateful sort, so they'd probably just arrest you and torture you for what you know. So, I trust you understand that your discretion is in both our interests."]]), {player=player.name()}))
      tk.msg(_("Gregar leaves the party"), _([[The moment of tension passes, and the officer leans back in his chair.
   "That threat delivered, I should at least extend my gratitude for helping one of ours in his time of need, though you had no reason to do so. That's why I will allow you to move freely on this station, at least to some extent, and I will allow you to leave when you please, as well as to return if you see the need. Who knows, maybe if you hit it off with the personnel stationed here, we might even come to consider you a friend."
   You exchange a few more polite words with the officer, then leave his office. As you head back to your ship, you consider your position. You have gained access to a centre of FLF activity. Should you want to make an enemy of House Dvaered, perhaps this would be a good place to start...]]))
      var.push("flfbase_intro", 2)
      var.pop("flfbase_flfshipkilled")
      flf.addLog( _([[You helped escort FLF Lt. Gregar Fletcher to the secret FLF base, Sindbad. This has earned you a small level of trust from the FLF and enabled you to freely access the FLF base.]]) )
      misn.finish(true)
   -- Case Dvaered planet
   elseif spob.cur():faction() == faction.get("Dvaered") and not mem.basefound then
      if tk.yesno(_("An opportunity to uphold the law"), _([[You have arrived at a Dvaered controlled world, and you are harboring a FLF fugitive on your ship. Fortunately, Gregar is still asleep. You could choose to alert the authorities and turn him in, and possibly collect a reward.
   Would you like to do so?]])) then
         tk.msg(_("Another criminal caught"), _([[It doesn't take Dvaered security long to arrive at your landing bay. They board your ship, seize Gregar, and take him away before he even comprehends what's going on.
   "You have served House Dvaered adequately, citizen," the stone-faced captain of the security detail tells you. "In recognition of your service, we may allow you to participate in other operations regarding the FLF terrorists. If you have further questions, direct them to our public liaison."
   The officer turns and leaves without waiting for an answer, and without rewarding you in any tangible way. You wonder if you should scout out this liaison, in hopes of at least getting something out of this whole situation.]]))
         faction.get("Dvaered"):modPlayerSingle(5)
         var.push("flfbase_intro", 1)
         var.pop("flfbase_flfshipkilled")
         dv.addAntiFLFLog( _([[You turned in FLF Lt. Gregar Fletcher to Dvaered authorities. The Dvaered captain who took him off your hands said that you could join in on a campaign against the FLF terrorists; you can direct questions to a Dvaered public liaison. You may want to scout out this liaison.]]) )
         misn.finish(true)
      end
   end
end

function commFLF()
   fleetFLF[1]:comm(_("Nothing personal, mate, but we're expecting someone and you ain't him. No witnesses!"))
end

-- Gregar wakes up and calls off the FLF attackers. They agree to guide you to their hidden base.
-- If the player has destroyed the FLF ships, nothing happens and a flag is set. In this case, the player can only do the Dvaered side of the mini-campaign.
function wakeUpGregarYouLazyBugger()
   mem.flfdead = true -- Prevents failure if ALL the ships are dead.
   for i, p in ipairs(fleetFLF) do
      if p ~= nil and p:exists() then
         p:setInvincible(true)
         p:setFriendly()
         p:setHealth(100,100)
         p:changeAI("flf_norun")
         p:setHilight(true)
         p:control()
         p:follow(player.pilot(), true)
         mem.flfdead = false
      end
   end
   if not mem.flfdead then
      tk.msg(_("Gregar puts an end to hostilities"), _([["Wha- hey! What's going on!"
   You were too busy dodging incoming fire, rebalancing your shields, and generally trying to kill your attackers before they kill you to notice that Gregar, your passenger, has roused from his slumber. Clearly the noise and the rocking have jolted him awake. You snap at him not to distract you from this fight, but he desperately interrupts.
   "These guys are my contacts, my friends! I was supposed to meet them here! Oh crap, this is not good. I didn't realize I'd be out this long! Look, I need to use your comm array right now. Trust me!"
   Before you have a chance to ask him what he thinks he's doing, Gregar begins tuning your communications array, and soon finds the frequency he wants.
   "FLF sentinel formation, this is Lt. Gregar Fletcher, authorization code six-six-niner-four-seven-Gamma-Foxtrot! Cease fire, I repeat, cease fire!" He then turns to you. "Same to you. Stop shooting. This is a misunderstanding, they're not your enemies."]]))
      tk.msg(_("Gregar puts an end to hostilities"), _([[You are skeptical at first, but a few seconds later it becomes apparent that the FLF fighters have indeed ceased firing. Then, there is an incoming comm from the lead ship.
   "This is FLF sentinel Alpha. Lt. Fletcher, authorization code verified. Why are you with that civilian? Where is your ship? And why didn't you contact us right away?"
   "Apologies, Alpha. It's a long story. For now, let me just tell you that you can trust the pilot of this ship. This pilot kindly helped me out of a desperate situation, and without that help I probably would never have returned alive. Request you escort us to Sindbad."
   "Copy that Lt. Fletcher." Alpha then addresses you. "Please follow us. We will guide you to our base. Stay close. Sensor range is lousy in these parts, and if you get separated from us, we won't be able to find you again, and you won't be able to find us or our base."
   With that, Alpha breaks the connection. It seems you have little choice but to do as he says if you ever want to take Gregar to his destination.]]))
      faction.get("FLF"):setPlayerStanding( 5 ) -- Small buffer to ensure it doesn't go negative again right away.
      misn.osdCreate(_("Deal with the FLF agent"), {
         fmt.f( _("Take Gregar, the FLF agent, to the {sys} system and make contact with the FLF"), {sys=mem.destsys}),
         _("Follow the FLF ships to their secret base. Do not lose them!"),
         _("Alternatively, turn Gregar in to the nearest Dvaered base"),
      })
      misn.osdActive(2)
      hook.timer(0.5, "inRange")
   end
end

function inRange()
   local mindist = 2000 -- definitely OOR.
   for i, p in ipairs(fleetFLF) do
      if p ~= nil and p:exists() then
         mindist = math.min(mindist, vec2.dist(p:pos(), player.pos()))
      end
   end
   if mindist < 500 then
      hook.timer(2.0, "annai")
      mem.OORT = hook.timer(10.0, "outOfRange")
   else
      hook.timer(0.5, "inRange")
   end
end

-- Fly the FLF ships through their waypoints
function annai()
   local poss = {}
   poss[1] = vec2.new(0,70)
   poss[2] = vec2.new(50, -50)
   poss[3] = vec2.new(-50, -50)
   poss[4] = vec2.new(0,120)
   local speed = player.pilot():stats().speed_max * 0.9
   for i, p in ipairs(fleetFLF) do
      if p ~= nil and p:exists() then
         if speed < p:stats().speed_max then
            p:setSpeedLimit(speed)
         end
         p:taskClear()
         p:control()
         p:moveto(player.pos()) -- NOT the player pilot, or the task may not pop properly.
         p:moveto(mem.waypoint2, false)
         p:moveto(mem.waypoint1, false)
         p:moveto(mem.waypoint0 + poss[i])
      end
   end
   mem.spawner = hook.timer(1.0, "spawnbase")
end

-- Part of the escort script
function spawnbase()
   local mindist = 2000 -- definitely OOR.
   for i, p in ipairs(fleetFLF) do
      if p ~= nil and p:exists() then
         mindist = math.min(mindist, vec2.dist(p:pos(), mem.waypoint0))
      end
   end
   if mindist < 1000 then
      diff.apply("FLF_base")

      -- Safety measure to ensure the player can land.
      mem.base = spob.get("Sindbad")
      mem.base:landOverride()

      mem.basefound = true
      hook.rm(mem.OORT)
   else
      mem.spawner = hook.timer(1.0, "spawnbase")
   end
end

-- Check if the player is still with his escorts
function outOfRange()
   local mindist = 2000 -- definitely OOR.
   for i, p in ipairs(fleetFLF) do
      if p ~= nil and p:exists() then
         mindist = math.min(mindist, vec2.dist(p:pos(), player.pos()))
      end
   end
   if mindist < 1500 then
      mem.OORT = hook.timer(2.0, "outOfRange")
   else
      tk.msg(_("You have lost contact with your escorts!"), _([[Your escorts have disappeared from your sensor grid. Unfortunately, it seems you have no way of knowing where they went.
   You notice that Gregar has disappeared from your cockpit. You search around your ship, but he's nowhere to be found. Seeing no other option, you give up on your search. Perhaps he'll turn up somewhere later.]]))
      abort()
   end
end

function abort()
   var.pop("flfbase_flfshipkilled")
   misn.finish(false)
end
