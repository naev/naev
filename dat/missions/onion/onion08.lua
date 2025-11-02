--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 08">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <done>Onion Society 07</done>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
 <tags>
  <tag>fleetcap_10</tag>
 </tags>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion 08

   Player has to take down lonewolf4 in his modified zebra uber-carrier
--]]
local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local onion = require "common.onion"
local love_shaders = require "love_shaders"
local trigger = require "trigger"
--local fleet = require "fleet"
local lmisn = require "lmisn"
local pilotai = require "pilotai"
local equipopt = require "equipopt"
local pulse = require "luaspfx.pulse"
local alert = require "luaspfx.alert"
local blink = require "luaspfx.blink"
local CTS = require "constants"
local tut = require "common.tutorial"

-- Reference to honeypot (trap)
local title = _("The Lone Wolf")
local reward = onion.rewards.misn08

local SYSTEM_START = system.get("Oxuram")
local SYSTEM_END = system.get("PSO")
local SPOB_EPILOGUE = spob.get("PSO 2434")
local SPOB_WAKEUP = spob.get("PSO Monitor")

-- Mission states
local STATE_START = 0
local STATE_FIGHT1_START = 1
local STATE_FIGHT1_UNCOVERED = 2
local STATE_WOLF_RANAWAY = 2
local STATE_WOLF_DEFEATED = 3
local STATE_EPILOGUE = 4
mem.state = nil

function create()
   if not var.peek("testing") then return misn.finish() end -- Not ready yet

   -- Hard claims now
   if not misn.claim( {SYSTEM_START, SYSTEM_END} ) then return misn.finish(false) end

   local prt = love_shaders.shaderimage2canvas( love_shaders.hologram(), onion.img_l337b01() )
   misn.setNPC( _("l337_b01"), prt.t.tex, _([[Follow up with l337_b01.]]) )
   misn.setReward(_("???") )
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[Confront lonewolf4.]])))
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01()
   vn.newCharacter( l337 )
   vn.transition("electric")

   l337(_([[l337_b01's familiar avatar pops up on your holodeck.]]))
   if not mem.talked then
      l337(_([["Heyo!"]]))
      l337(_([["I've been tracking the fake bounty that they set on you. Their nearly completely covered their trails, however, they are no match for my counterespionage skills!"]]))
      local guess = var.peek("onion_guess_insider")
      if guess=="lonewolf4" then
         l337(_([["It looks like we were right with our guess! The signs point to lonewolf4, and it also seems like I was able to pinpoint their location to boot!"]]))
      else
         l337(fmt.f(_([["It looks like it wasn't {guess} as you guessed, but lonewolf4. Seems like my technomancer instincts were right on the spot. Not only was I able figure out their identity, but I've been able to pinpoint their location too!"]]),
            {guess=guess}))
      end
      l337(fmt.f(_([["I was able to track the signal all the way to the {sys} system, which is a bit surprising given that the Nexus connection there must be unreliable, but they must have ways to work around that or something."]]),
         {sys=SYSTEM_START}))
      l337(_([["All that is left is to head down there and confront them, finally putting a stop to add these horrors."]]))
      vn.menu{
         {_([["The end is neigh!"]]), "01_end"},
         {_([["Justice for Trixie."]]), "01_trixie"},
         {_([["Are you alright?"]]), "01_alright"},
      }

      vn.label("01_end")
      l337(_([["All's well that ends well. Although I don't think we can reach the happy ending any more, not without Trixie..."
They let out a sigh.]]))
      vn.jump("01_cont")

      vn.label("01_trixie")
      l337(_([["Trixie will never be forgotten! There was so much they wanted to do... I guess the best way to honour their memory is to follow their dreams."]]))
      vn.jump("01_cont")

      vn.label("01_alright")
      l337(_([["Better than before, thank you. But can't stop now with the end in sight! Once this is over I'll finally have some peace of mind and can clean up stuff. There is so much about Trixie I still need to look into."]]))
      vn.jump("01_cont")

      vn.label("01_cont")
      l337(fmt.f(_([["We would have never made it this far if it wasn't for you, {player}. Thank your again!"
Determination builds up in their voice.
"You ready to put an end to lonewolf4's rampage?"]]),
         {player=player.name()}))

      vn.func( function () mem.talked = true end )
   else
      l337(_([["Ready to put a stop to lonewolf4's rampage?"]]))
   end
   vn.menu{
      {_([["Let's do this."]]), "accept"},
      {_([["I need time to prepare."]]), "later"},
   }

   vn.label("later")
   l337(_([["I'll keep tracking them. Get in touch with me when you're ready."]]))
   vn.done("electric")

   vn.label("accept")
   vn.func( function() accepted=true end )
   l337(fmt.f(_([["Let's put an end to this! On to {sys}! I'll be hitching a ride on your electronics, hopefully your Ship AI doesn't mind."]]),
      {sys=SYSTEM_START}))
   vn.na(_([[You think you hear a small electronic voice saying "I do mind.", but it could just be your imagination...]]))

   vn.done("electric")
   vn.run()

   if not accepted then return end
   misn.accept()

   mem.state = STATE_START
   hook.enter("enter")
   hook.land("land")

   misn.markerAdd( SYSTEM_START )
   misn.osdCreate( title, {
      fmt.f(_("Confront lonewolf4 at the {sys} system!"),
      {sys=SYSTEM_START}),
   })
end

local function get_fct()
   return faction.dynAdd("Mercenary", _("lonewolf4"), _("lonewolf4"), {
      player=-100,
      ai="baddie",
   })
end

local function spawn_wolf( pos, hologram, part2 )
   local p = pilot.add("Zebra Wolfie", get_fct(), pos, _("lonewolf4"), {naked=true} )
   p:intrinsicSet( "shield", 1000 )
   p:intrinsicSet( "armour", 1000 )
   p:intrinsicSet( "absorb", -25 )
   p:intrinsicSet( "shield_regen_mod", -60 ) -- Or too bulky
   p:intrinsicSet( "fbay_capacity", -50 ) -- Lower number of drones
   p:intrinsicSet( "fbay_reload", 100 ) -- Instead give reload bonus
   p:intrinsicSet( "cpu_max", 1000 )
   local params = {
      max_same_weap = 2,
      rnd = 0,
      fighterbay = 1.5,
      pointdefence = 1.5,
      outfits_add = {
         "Guardian Overseer System",
         "Guardian Interception System",
      },
      type_range = {
         ["Point Defence"] = { min=1 },
      },
   }
   if part2 then
      p:intrinsicSet( "shield", -1e6 ) -- no shields
      p:intrinsicSet( "armour_regen_mod", -1e6 ) -- No armour regen
      table.insert( params.outfits_add, "Heavy Laser Turret" )
      params.fighterbay = 0
      params.turret = 3
   end
   equipopt.zalek( p, params )
   if hologram then
      p:intrinsicSet( "weapon_damage", -10000 )
      p:intrinsicSet( "fbay_damage", -10000 )
   end
   p:memory()._hologram = hologram
   return p
end

function enter ()
   local scur = system.cur()
   if scur==SYSTEM_START and mem.state==STATE_START then
      pilot.clear()
      pilot.toggleSpawn(false)
      hook.timer( 5, "fight1_start1" )

   elseif scur==SYSTEM_END and mem.state==STATE_WOLF_RANAWAY then
      pilot.clear()
      pilot.toggleSpawn(false)
      hook.timer( 1, "fight2_start1" )

   elseif mem.state~=STATE_START and mem.state~=STATE_WOLF_RANAWAY then
      return lmisn.fail("you abandoned the attack on lonewolf4!")

   end
end

function energy_surge_hook( pos )
   local fct = get_fct()
   local drones = {}
   for k,s in ipairs{
      "Za'lek Light Drone",
      "Za'lek Light Drone",
      "Za'lek Bomber Drone",
      "Za'lek Bomber Drone",
      "Za'lek Heavy Drone",
      "Za'lek Heavy Drone",
   } do
      local p = pos + vec2.newP( 300*math.sqrt(rnd.rnd()), rnd.angle() )
      local d = pilot.add( s, fct, p )
      d:effectAdd("Blink")
      d:setHostile(true)
      -- Slow them down so the player can "dodge"
      d:intrinsicSet("speed_mod", -50)
      d:intrinsicSet("accel_mod", -50)
      table.insert( drones, d )
   end
   hook.timer( 5, "energy_surge_end_hook", drones )
end
function energy_surge_end_hook( drones )
   for k,d in ipairs(drones) do
      if d:exists() then
         blink( d, d:pos() )
         d:rm()
      end
   end
end
local function energy_surge( pos )
   player.autonavReset( 5 )
   player.msg("#r".._("Energy surge detected!").."#0",true)
   alert( pos, {
      size = 300,
   } )
   hook.timer( 2.2, "energy_surge_hook", pos )
end

local function energy_surge_at_player ()
   local pp = player.pilot()
   local pos = pp:pos() + 3 * pp:vel()
   local range = 50
   if pp:flags("stealth") then
      range = 500
   end
   energy_surge( pos + vec2.newP( math.sqrt(rnd.rnd())*range, rnd.angle() ) )
end

--[[
Stage 1 fight:
1. Sends drones periodically while moving around - there are 2 holograms doing the same (but they are hologram drones)
2. When player damages the real one switches into combat mode.
3. WHen shield drops to 0, jumps out in a cloud of drones.
--]]
local real, fake1, fake2, bosses, discovered, fight1_start3
function fight1_start1 ()
   mem.state = STATE_FIGHT1_START

   player.autonavReset(5)
   player.msg(_("l337_b01: What's this? I'm getting some unusual readings?"),true)

   local rad = system.cur():radius()*0.8
   local ang = player.pos():angle()
   local off = rnd.permutation{ -math.pi*0.5, math.pi*0.5, math.pi }
   local pos = {}
   for k,o in ipairs(off) do
      pos[k] = vec2.newP( rad, ang+o )
   end

   -- Spawn them
   real  = spawn_wolf( pos[1], false )
   fake1 = spawn_wolf( pos[2], true )
   fake2 = spawn_wolf( pos[3], true )

   -- Generate waypoints
   local waypoints = {}
   local N = 16
   local reverse = 1
   if rnd.rnd() < 0.5 then reverse = -1 end
   for i=1,N do
      table.insert( waypoints, vec2.newP( rad, math.pi*2 * (i-1)/N * reverse ) )
   end

   -- Set them up
   local min_stealth = 3000/CTS.STEALTH_MIN_DIST*100
   bosses = { real, fake1, fake2 }
   for k,b in ipairs(bosses) do
      -- They should only be uncoverable at 3000 units
      b:intrinsicSet( "ew_stealth", -99.999 )
      b:intrinsicSet( "ew_stealth_min", min_stealth )
      b:tryStealth()
      b:memory().enemyclose = 3000

      hook.pilot( b, "discovered", "fight1_discovered" )
      hook.pilot( b, "attacked", "fight1_attacked" )
   end
   pilotai.patrol( bosses, waypoints )

   hook.timer( 5, "fight1_start2")
end
function fight1_start2 ()
   local mpos = system.cur():waypoints("lonewolf4_start")
   player.msg(_([[l337_b01: "I need to do a full sweep. Head to the marked position!"]]), true)
   system.markerAdd( mpos )
   trigger.distance_player( mpos, 1000, fight1_start3 )
end
function fight1_check_holograms ()
   local nearby = 0
   for k,p in ipairs( pilot.getInrange( player.pos(), 1500 ) ) do
      if p:memory()._hologram then
         nearby = nearby+1
      end
   end
   if nearby >= 4 then
      player.msg(_([[l337_b01: "Wait, some ships are not doing damage... Are they holograms?"]]), true)
      return
   end
   hook.timer( 1, "fight1_check_holograms" )
end
function fight1_start3 ()
   system.markerClear()
   local pos = player.pos() + vec2.newP( 200, rnd.angle() )
   pilot.add( "Za'lek Scout Drone", get_fct(), pos, nil, {ai="baddiepatrol"} )
   pulse( pos, nil, {
      col = {0.1, 0.8, 0.3, 0.5},
   } )
   local pp = player.pilot()
   pp:intrinsicSet("ew_stealth", 300 )
   for k,p in ipairs(pp:followers()) do
      p:intrinsicSet("ew_stealth", 300 )
   end
   player.msg(_([[l337_b01: "We've been spotted! Looks like lonewolf4 wants a fight!"]]),true)
   hook.timer( 1, "fight1_check_holograms" )
   fight1_launch()
end
-- Take turns launching
local last_launch, last_hilighted, launched
function fight1_launch()
   -- Finishes when he's uncovered
   if mem.state >= STATE_FIGHT1_UNCOVERED then
      return
   end

   last_launch = ((last_launch or rnd.rnd(1,#bosses)) % #bosses)+1
   local b = bosses[last_launch]

   launched = launched or {}
   local hologram = b:memory()._hologram
   local fct = get_fct()
   local drones = {}
   for i=1,3 do
      local shp = "Za'lek Light Drone"
      if i==1 then
         shp = "Za'lek Heavy Drone"
      end
      local p = pilot.add( shp, fct, b:pos()+vec2.newP( 20*rnd.rnd(), rnd.angle() ), nil, {ai="baddiepatrol"} )
      p:intrinsicSet( "ew_hide", 100 )
      p:intrinsicSet( "ew_signature", -25 )
      p:intrinsicSet( "ew_detect", 250 )
      if hologram then
         p:intrinsicSet( "weapon_damage", -10000 )
         p:memory()._hologram = hologram
      end

      table.insert( drones, p )
      table.insert( launched, p )
   end

   -- Always try to mark the last drone coming in
   if last_hilighted and last_hilighted:exists() then
      last_hilighted:setHilight(false)
   end
   drones[1]:setHilight(true)
   last_hilighted = drones[1]
   trigger.distance_player( drones[1], 1500, function ()
      drones[1]:setHilight(false)
   end )

   -- Delay based on number of existing fighters
   -- plts => plts^1.25
   -- 5  => 8
   -- 10 => 18
   -- 15 => 30
   -- 20 => 42
   local nplts = #pilot.get(fct)-3
   hook.timer( 5+5*rnd.rnd()+nplts^1.25, "fight1_launch", b )
end
function fight1_discovered( plt )
   plt:setHilight(true)
   if discovered then return end
   player.msg(_([[l337_b01: "Looks like a heavily modified Zebra. Take it out!"]]), true)
   discovered = true
end
function fight1_attacked( plt )
   -- Only matters at the beginning
   if mem.state ~= STATE_FIGHT1_START then return end

   if plt:memory()._hologram then
      -- Clear deployed drones
      for k,p in ipairs(plt:followers()) do
         p:effectAdd("Fade-Out")
      end
      plt:effectAdd("Fade-Out")
      local b = {}
      for k,p in ipairs(bosses) do
         if p~=plt and p:exists() then
            table.insert(b,p)
         end
      end
      bosses = b
      if #bosses==2 then
         player.msg(_([[l337_b01: "A hologram!? The real ship must be out there still."]]), true)
      else
         player.msg(_([[l337_b01: "Another hologram!?"]]), true)
      end
   else
      mem.state = STATE_FIGHT1_UNCOVERED
      for k,p in ipairs(launched) do
         if p:exists() then
            p:effectAdd("Fade-Out")
         end
      end

      real:broadcast(_("Thy greed doth sow the seeds of thine undoing!"))
      real:setHilight(true)
      hook.timer( 0.5, "fight1_timer" )
   end
end

function fight1_timer ()
   if not real:exists() or real:shield() < 0.01 then
      local pos = real:pos()
      blink( real, pos )
      real:rm()
      energy_surge_at_player()

      trigger.timer_chain{
         { 9, _("l337_b01: What the hell was that?") },
         { 5, fmt.f(_("l337_b01: Wait, they're nearby in the {sys} system!"),
            {sys=SYSTEM_END}) },
         { 0, function ()
            misn.osdCreate( title, {
               fmt.f(_("Chase lonewolf4 to the {sys} system!"),
                  {sys=SYSTEM_END}),
            })
            misn.markerRm()
            misn.markerAdd( SYSTEM_END )
            mem.state = STATE_WOLF_RANAWAY
         end },
         { 5, fmt.f(_("l337_b01: No time to lose, onwards to {sys} system!"),
            {sys=SYSTEM_END}) },
      }
   else
      hook.timer( 0.5, "fight1_timer" )
   end
end

--[[
Stage 2 fight:
0. Shields are down permanently
1. Does an energy surge attack every so often
2. When starts to take damage, charges up for 10 seconds and jumps away
--]]
local finalboss
local health_state
function fight2_start1 ()
   local pos = system.cur():waypoints("lonewolf4_spawn")
   finalboss = spawn_wolf( pos, false, true )
   finalboss:effectAdd("Fade-In")
   finalboss:setHilight(true)
   finalboss:setVisplayer(true)
   finalboss:setNoDisable(true)

   health_state = 1
   hook.pilot( finalboss, "death", "fight2_death" )
   hook.timer( 1, "fight2_health" )
   trigger.timer_chain{
      { 5, _([[l337_b01: "Stop this lonewolf4! It doesn't have to end this way!"]]) },
      { 6, function ()
         finalboss:broadcast(_("Have at thee cursed wench!"))
      end },
      { 3, function ()
         fight2_energy_surge()
      end },
   }
end
local last_surge = 0
function fight2_energy_surge ()
   if not finalboss:exists() then return end

   local t = naev.ticksGame()
   local off = t-last_surge
   if off < 15 then
      hook.timer( 25-off, "fight2_energy_surge" )
      return
   end
   last_surge = t

   energy_surge_at_player()
   hook.timer( 25 + rnd.rnd()*5, "fight2_energy_surge" )
end
local health_threshold = {
   {75 , _("From whence hast thou been devising such treachery, l337_b01?"), _("You took the words out of my mouth!")},
   {50,  _("Now my eyes doth see the truth, Tenebros Station was naught but a start!"), _("Let me explain!")},
   {25,  _("O bitter fate, to think we once walked the same path!"), _("Stop this madness!")},
   {10, _("Curse you and your betrayal!")},
}
function fight2_health ()
   if not finalboss:exists() then return end
   local ht = health_threshold[health_state]
   if not ht then return end
   if finalboss:armour() < ht[1] then
      health_state = health_state+1
      finalboss:broadcast( ht[2] )
      if ht[3] then
         trigger.timer_chain{
            { 5, fmt.f(_([[l337_b01: "{msg}"]]), {msg=ht[3]} ) },
         }
      end
      energy_surge_at_player()
      last_surge = naev.ticksGame()

      local pos = finalboss:pos()
      blink( finalboss, pos )
      finalboss:setPos( pos + vec2.newP( 1500+500*rnd.rnd(), rnd.angle() ) )
   end
   hook.timer(1, "fight2_health" )
end
local last_blink
function fight2_death ()
   -- Avoiding hook triggering itself
   if last_blink then return end
   last_blink = true
   blink( finalboss, finalboss:pos() )
   finalboss:rm()

   -- Clear all the other ships too
   for k,p in ipairs(pilot.get(get_fct())) do
      blink( p, p:pos() )
      p:rm()
   end
   hook.timer( 10, "fight2_epilogue" )
end
function fight2_epilogue ()
   diff.apply( "onion08" )
   misn.markerRm()
   misn.markerAdd( SPOB_EPILOGUE )
   mem.state = STATE_WOLF_DEFEATED
   misn.osdCreate( title, {
      fmt.f(_("Investigate {spb} ({sys} system)"),
      {spb=SPOB_EPILOGUE, sys=SYSTEM_END}),
   })

   vn.clear()
   vn.scene()
   local l337 = onion.vn_l337b01()
   vn.newCharacter( l337 )
   vn.transition("electric")

   l337(_([[A flustered l337_b01 pops up on your holodeck.
"Betrayal? What the hell were they going on about?"]]))
   l337(fmt.f(_([["Wait, I've tracked their signal! It seems lie the ship crash-landed on {spb}. Hurry, they might still be alive!"]]),
      {spb=SPOB_EPILOGUE}))

   vn.done("electric")
   vn.run()
end

-- End of it all
local tint, epilogue
function land ()
   if mem.state==STATE_EPILOGUE then epilogue() end

   -- Land hook only runs at the final epilogue when landing
   if mem.state~=STATE_WOLF_DEFEATED or spob.cur()~=SPOB_EPILOGUE then return end

   local stormshader = love_shaders.sandstorm{
      colour = {0.9, 0.2, 0.8, 0.5},
   }
   local stormsound
   local function storm_strength( str )
      -- TODO modify sounds too
      stormshader:send( "u_strength", str )
      vn.musicVolume( stormsound, str )
   end
   local function start_storm( str )
      str = str or 0.5
      local lw, lh = love.graphics.getDimensions()
      vn.setBackground( function ()
         vn.setColour( {1, 1, 1, 1} )
         local oldshader = love.graphics.getShader()
         love.graphics.setShader( stormshader )
         love.graphics.draw( love_shaders.img, 0, 0, 0, lw, lh )
         love.graphics.setShader( oldshader )
      end )
      vn.setUpdateFunc( function( dt )
         stormshader:update(dt)
      end )
      stormsound = vn.music( "snd/sounds/loops/sandstorm.mp3" )
      storm_strength( str )
   end
   local function stop_storm ()
      vn.setBackground( nil )
      vn.setUpdateFunc( nil )
      vn.musicStop( stormsound )
   end

   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[You enter the orbit of {spb} and scour the violent atmosphere for signals of the downed Zebra carrier.]]),
      {spb=SPOB_EPILOGUE}))

   local l337 = onion.vn_l337b01()
   vn.appear( l337, "electric" )
   l337(_([["Here, let me adjust the tune the scanning frequencies to the natural harmonics."]]))
   vn.na(_([[As if magic, your sensors begin to hazily pick up a large man-made structure that has had some unexpected percussive maintenance performed on it by the planet's surface.]]))
   l337(_([["There it is, we have to check it out!"]]))

   vn.move( l337, "right" )
   local sai = tut.vn_shipai{ pos="left" }
   vn.appear( sai, "electric" )

   sai(_([["Pardon for the intrusion, but I would advise against atmospheric entry. My models predict a survival rate of 7.3%."]]))
   l337(_([["And my models predict there's no way in hell we can give up now! We have to get to the bottom of this."]]))
   vn.menu{
      {_([["We're going in!"]]), "01_goin" },
      {fmt.f(_([["{sai}, do a double check."]]), {sai=tut.ainame()}), "01_double"},
   }

   vn.label("01_goin")
   l337(_([["I knew you'd not give up now! We're coming for you lonewolf4!"]]))
   vn.jump("01_cont")

   vn.label("01_double")
   sai(_([["New calculations point to NO PROBLEM."
The avatar flickers a second.
"... p...nt to NO PROBLEM."]]))
   l337(_([["All is good, let's go!"]]))
   vn.na(_([[Looks like there is no choice but to go in.]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   l337(_([["Nice AI btw, wonder why it takes up so much computational power though. Hard to fit with it."]]))
   vn.func( start_storm )
   vn.na(_([[You push the throttle and begin the approach to the wreckage, as the ferocious planetary eternal storm begins to wrack your ship. This might get bumpy.]]))
   vn.na(_([[Your ship breaks through the atmosphere, as the planet promptly makes sure you understand that the Demon-class label is not just for show. You quickly have to shut off the emergency warning systems before they permanently damage your hearing.]]))
   vn.na(_([[You think you hear something coming from your Ship AI, but can't make it out through the howling of your atmospheric rendezvous, so you pump up the volume on the holodeck.]]))
   storm_strength( 0.8 )
   sai(_([["SHIELDS AT 7%. WILL NOT HOLD MUCH LONGER."]]))
   l337(fmt.f(_([["I'VE REROUTED EXCESS VITAL ENERGY TO SHIELDS, NO PROBLEMO. {player} HAS GOT IT COVERED."]]),
      {player=string.upper(player.name())}))
   vn.na(_([[You hear a long crunch as some part of your ship decides to practice free fall skydiving, that's not good.]]))
   sai(_([["SHIELDS DOWN. HULL INTEGRITY FAILING."]]))
   l337(_([["RE- *CRACKLE* -ERGY- *POP*"
l337_b01's avatar freezes. Seems like the storm is incompatible with transmissions.]]))
   vn.na(_([[Trying to hold your balance, you guesstimate the heading of the Zebra wreckage, and wing it.]]))
   vn.na(_([[After what seems like an eternity of a cacophony of fuselage discontentment, your ship crashes into something, sending you flying. ]]))

   vn.scene()
   vn.transition("blur")
   vn.func( stop_storm )
   local memory = vne.flashbackTextStart( _("Haziness"), {transition="blinkin"})
   local function m( txt ) memory("\n"..txt,true) end
   memory(_([[So soft... So quiet...]]))
   m(_([[What is that?]]))
   m(_([[...]]))
   m(_([[A gust of wind. Relaxing.]]))
   m(_([[...]]))
   m(_([[What were you doing?]]))
   m(_([[Probably not important...]]))
   m(_([[...]]))
   m(_([[Back to sleep.]]))
   m(_([[Wait... ... ...What is that?]]))
   m(_([[Trixie?]]))
   vne.flashbackTextEnd{ notransition=true }

   --vn.scene() -- vn.scene() is done in vne.flashbackTextEnd
   vn.func( function ()
      start_storm( 0.4 )
   end )
   vn.newCharacter( l337 )
   vn.newCharacter( sai )
   vn.transition("blinkout")

   vn.na(_([[You gasp for breath. Shit, feels like your lungs are on fire. Now it's all over the floor. Nasty.]]))
   l337(fmt.f(_([["{player}! {player}! Don't scare me lie that! Not after Trixie!"]]),
      {player=player.name()}))
   sai(_([["Vital signs confirmed stabilized."]]))
   vn.na(_([[Wait, where are you? Your ship? What about the storm? You... crashed?]]))
   l337(fmt.f(_([["You with us, {player}?"]]),
      {player=player.name()}))
   vn.menu{
      {_([["Give me a second."]]), "02_second"},
      {_([["I'me fine."]]), "02_fine"},
   }

   vn.label("02_second")
   vn.na(_([[You take a few deep breaths. Once the adrenaline wears off, you're going to be in for a world of pain.]]))
   vn.jump("02_cont")

   vn.label("02_fine")
   vn.na(_([[You try to smile, but it comes out more as a grimace. You hope that's not a broken rib.]]))
   vn.jump("02_cont")

   vn.label("02_cont")
   sai(_([["Recommend urgent medical treatment. Preliminary prognostic at least 2 minor fractures."]]))
   l337(_([["We're so close, I've managed to break the code to the Wolfie! You should be able to break into to it easy now."]]))
   vn.menu{
      {_([["Wolfie?"]]), "03_wolfie"},
      {_([["What happened?"]]), "03_what"},
   }

   vn.label("03_wolfie")
   l337(_([["lonewolf4's carrier! It's quite a remarkable design. The only thing that really remains of the Zebra is the outer hull, the interior has been completely refitted!"]]))
   vn.jump("03_cont")

   vn.label("03_what")
   l337(_([["You made it to lonewolf4's carrier! It's quite a remarkable design. The only thing that really remains of the Zebra is the outer hull, the interior has been completely refitted!"]]))
   vn.jump("03_cont")

   vn.label("03_cont")
   vn.na(_([[You clamber around until you can pull out a med kit and give yourself a nice stim boost. That should keep you going for a bit, but you'll need proper medical care afterwards. Assuming there is an afterwards...]]))
   l337(_([["It seems like there is somewhat of a clearing here generated by whatever the hell the Wolfie is sporting. Not sure how long it will last."]]))
   l337(_([["There's still lifeform readings, but they aren't strong. You have to hurry and check it out!"]]))
   vn.na(_([[You groan as you lift yourself up. Looks like it's time to finish this.]]))
   sai(_([["Correction, at least 3 minor fractures now."]]))
   vn.na(fmt.f(_([[Ignoring {sai}'s complaints, you don an atmospheric suit and head outside.]]),
      {sai=tut.ainame()}))
   vn.na(_([[As you exit the ship's lock, you quickly realize that outside is actually inside, as your ship seems to have crashed directly into lonewolf4's carrier.]]))
   vn.na(_([[Weapon in hand you make your way through the wreck of the ship. It seems like there's not much of corridors, it's all maintenance tubes which force you to crawl through, occasionally having to blast through debris. What the hell is with this ship's design?]]))
   storm_strength( 0.2 )
   vn.na(_([[You push yourself through another tunnel and find yourself in a surprisingly wide room with some faint illumination. In the centre seems to be a damaged pod with someone in it. Wait is that blood?]]))

   l337(_([["lonewolf4? Let me see if I can interface with it!"]]))
   vn.na(_([[You look around the room, it looks like a mess, even before everything was scattered around in the crash.]]))
   l337(_([["Got it!"]]))

   vn.move( l337, "farright" )
   local wolf = onion.vn_lonewolf4()
   vn.appear( wolf, "electric" )
   wolf(_([["..."
There is a long pause as the wolf avatar stares at you.
"Cometh to gloat l337_b01? Doth the fruits of thy schemes delight thy heart?"]]))
   l337(fmt.f(_([["YOU were the one that peeled Trixie. YOU are the one who tried to kill {player} and even me if you had the chance! YOU ARE THE ONE FUCKING SHIT UP!"]]),
      {player=player.name()}))
   wolf(_([[lonewolf4 seems to speak a bit slower than usual.
"Evenst on the brink of triumph, dost thy tongue naught but dealeth guile. Wilt thou, all keys in hand, take for thyself the seat of God?" ]]))
   l337(_([["Even now you speak in riddles. Can't you just make it easier! Why... after Trixie... after everything..."]]))
   wolf(_([[There is a small pause.
"Tenebros Station. Thou slayeth the entire Station, and with that, my family."]]))
   l337(_([["Tenebros Station? You know nothing about Tenebros station!! That was a set-up!"]]))
   wolf(_([[Unphased, lonewolf4 continues, "Upon the last breath of v3c70r, doth hath realized the keys were...   ...were at hand, and thus thou trodth'... trodth' on the path of betrayal."]]))
   l337(_([["That's all wrong! *sniff* You've got it all wrong! v3c70r, Trixie, we all did it for the greater good!"]]))
   wolf(_([["Doth thou not wonder, if perhaps we have lived too long? These games, are they naught but born of the rotten body and mind?"]]))
   l337(_([[...]]))
   wolf(_([["Look at me l337_b01, lookth' at the... ...the pass of the centa-cycles."]]))
   l337(_([["You can't quit now! You've got it all wrong!"]]))
   wolf(_([["It is... what it... is."]]))
   -- TODO change music
   l337(_([[...]]))
   sai(_([["External vital signs extinguished."]]))
   wolf(_([[The avatar is motionless, almost placid.]]))
   l337(_([["Enough."]]))
   vn.disappear( wolf, "electric")
   vn.move( l337, "centre" )

   vn.label("questions")
   vn.menu{
      {_([["What was that?"]]), "04_cont"},
      {_([["Tenebros Station?"]]), "04_cont"},
      {_([["Centa-cycles?"]]), "04_cont"},
      {_([["v3c70r?"]]), "04_cont"},
      {_([["..."]]), "04_cont"},
   }

   vn.label("04_cont")
   l337(_([[They let out a deep sigh.
"I'm sure you have a lot of questions, but I think I should explain from the beginning."]]))
   l337(_([["Us, the Onion Society, we're an old society. Now you see us bickering and fighting, but we were quite a tight group originally: l337_b01, DOG, lonewolf4, notasockpuppet, underworlder, v3c70r, and Trixie."]]))
   l337(_([["We would spend most of our time doing stupid stunts, but one day, we got lucky and stumbled upon a backdoor to the Nexus backbone."]]))
   l337(_([["Even young, we realized the potential of it: the full control of the entire inter-galactic network. After arguing, we decided to lock it and split the keys, and pass the problem of dealing with it to our future selves. I'll spare you the details, but we made it so without all the keys, you wouldn't be able to unlock it. So unless we ever got together, the Nexus would stay as it was."]]))
   l337(_([["We also made sure that if anything happened to any one of us, the keys would after some time, a Dead Man's switch. So it would never get completely lost. And then, we just went about our business as usual, now with each of us having a key as a Keeper of the Secrets."]]))
   vn.label("05_menu")
   vn.menu{
      {_([["How old is old?"]]), "05_old"},
      {_([["Stupid Stunts?"]]), "05_stunts"},
      {_([[Continue.]]), "05_cont"},
   }

   vn.label("05_old")
   l337(_([["Over 100 cycles, closer to 200 I think. Living so long takes a hard toll on the body... and the mind. It takes a lot of resources to keep organics running for so long..."]]))
   l337(_([["Maybe lonewolf4 is right. Maybe we have lived too long?"]]))
   vn.menu{
      {_([["Short lives are for a reason."]]), "05_old_yes"},
      {_([["Time shouldn't confine us."]]), "05_old_no"},
   }
   vn.label("05_old_yes")
   l337(_([["I guess so. Maybe having a deadline makes you appreciate things more."]]))
   vn.jump("05_menu")

   vn.label("05_old_no")
   l337(_([["Maybe you are right. This is not a problem of longevity, but of character."]]))
   vn.jump("05_menu")

   vn.label("05_stunts")
   l337(_([["The usual script kiddie stuff: hijack a planetary intercom system, reroute thousands of tonnes of manure to be dumped on aristocrat mansions, leak corporate and government databases. The usual an aspiring technomancer does."]]))
   l337(_([[They let out a sigh.
"Those were carefree times..."]]))
   vn.jump("05_menu")

   vn.label("05_cont")
   l337(_([["Going about our things, some of us noticed something big happening at Tenebros Station. Like conspiracy-type stuff, but it turned out to be true. At the time there was an Imperial project experimenting with mind-control drugs, something they developed by reverse engineering the Sirius or something like that. Hard to tell when half the documents are lies."]]))
   l337(_([["I think v3c70r stumbled upon it while tracking some refugee diversion database changes, and then they brought me, l337_b01, and Trixie into it."]]))
   vn.menu{
      {_([["You and l337_b01!?!"]]), "06_l337"},
      {_([[Let them continue.]]), "06_cont"},
   }

   vn.label("06_l337")
   l337(_([["I'll get that to a second, but as you may have guessed, I wasn't always l337_b01..."]]))
   vn.jump("06_cont")

   vn.label("06_cont")
   l337(_([["So, we put together what we called Operation Dissonance, to figure out what was going on, and if possible, sabotage the project. Back then, as we were younger, l337_b01 and v3c70r decided to go in person, while Trixie and I stayed back providing support."]]))
   l337(_([["Things didn't go on as expected. We quickly found out that the project was actually in the last stage, and was going to be activated on all the inhabitants of the station."]]))
   l337(_([["You have to understand, that if that happened, this could easily spread around the galaxy, with tons of sleeper agents able to do who knows what. So we had to take a tough decision, either take out the station and avoid the worst, or try to clean up afterwards."]]))
   l337(_([[They let out another deep sigh.
"The project looked really bad, so we decided to... sabotage the station and terminate the life support. v3c70r ad l337_b01 were still there, so I was against it, but time was running out, and we had few choices..."]]))
   l337(_([["But, that's where... that's where I screwed up..."
They pause.
"I triggered a fail-safe system, and all the leeway we may have had disappeared..."]]))
   l337(_([["They... they didn't make it."
You hear a gulp.
"Trixie was desperate, and tried everything, but she also took a toll. The neural feedback damaged her... they was never the same."]]))
   l337(_([["Modern medicine can do wonders to the body, but the mind, technology hasn't changed much in hundreds of cycles... and Trixie was no exception..."]]))
   l337(_([["At the end, l337_b01, my mentor, was able to pass their private codes to me, to keep their legacy. To fully become l337_b01, I peeled myself and pinned all the blame on DEADBEEF, my old self."]]))
   l337(_([["While taking care of Trixie, who barely remembered anything, I found out that lonewolf4 had some family on the station. And they never forgot and... never forgave..."]]))
   l337(_([["The Onion Society, and most of us were never the same after that..."]]))
   l337(_([[There is a pause.
"I think you're the first person I share all of this with."]]))
   vn.label("07_menu")
   vn.menu{
      {_([["So what happened to the station?"]]), "07_station"},
      {_([["l337_b01 was your mentor?"]]), "07_l337"},
      {_([["Mind Control?"]]), "07_mindcontrol"},
      {_([["How did lonewolf4 not notice?"]]), "07_lonewolf4"},
      {_([["Thanks for sharing."]]), "07_share"},
      {_([["What do we do next?"]]), "07_next"},
   }

   vn.label("07_station")
   l337(_([["With no life support everyone died. The project was scrapped and attributed to a malfunction, so that inspections would be cut short. The entire place was cordoned off, and probably blown into bit by the Incident. There shouldn't be any public records left. It was over 100 cycles ago, and I made sure to purge all the data."]]))
   l337(_([["I still do think that there might have been another way. If only we noticed sooner..."]]))
   vn.jump("07_menu")

   vn.label("07_l337")
   l337(_([["Yes. They took me up as a disciple and taught be all the technomancery I know. If it wasn't for l337_b01, I probably would have gotten fried by gangs, pirates, or even bureaucrats a long time ago. I owe them my life."]]))
   l337(_([["l337_b01 was the best technomancer there was. I've never been able to be half as good as they were."]]))
   vn.jump("07_menu")

   vn.label("07_mindcontrol")
   l337(_([["It was one of the endless horrible Imperial projects destined to run out of control. Their plan was to embed conditioning as a virus into humans so that they could trigger it remotely to control people. It was also engineered to spread like a virus, so it would spread across the Empire."]]))
   l337(_([["I have no idea what they were thinking or how well it would work, but it was a recipe for disaster on all levels. Probably got approved and developed through chained bureaucratic debacles. But the risk was real, and it all had to be stopped."]]))
   l337(_([["I made sure all data of the plans was completely wiped so it couldn't be developed again. Haven't heard about it since."]]))
   vn.jump("07_menu")

   vn.label("07_lonewolf4")
   l337(_([["It was surprisingly kept very well as a secret. Only a small team was developing it. If we hadn't stumbled upon it by chance, I don't think we would have ever found it."]]))
   l337(_([["Not to mention it's much harder to notice things when you aren't looking. It's one thing to hack into governmental databases, and another to do the same level of scrutiny to an arbitrary small station, even if your family are there."]]))
   l337(_([["If we had known lonewolf4's family was involved, we would have brought him into the operation, but there was so little time. It was just one thing after another."]]))
   vn.jump("07_menu")

   vn.label("07_share")
   l337(_([["It does feel like I took a big weight off my chest. Even though it's been a long time, I still sometimes wake up from nightmares of Tenebros Station. Maybe I'll be able to finally rest."]]))
   vn.jump("07_cont")

   vn.label("07_next")
   l337(_([[They pause a second.
"That's a great question. Let me think."]]))
   vn.jump("07_cont")

   vn.label("07_cont")
   l337(_([["First things first, we have to get you out of here. You're fine for now, but I'm not sure how long the lull in the storm will last, and when it's gone, you're not going to have much time."]]))
   vn.move( l337, "right" )
   sai(fmt.f(_([["Analysis show that the Wolfie has a non-standard blink engine with 78% functionality remaining. Proposal: rewire blink engine target to the {shipname} to exist atmosphere. Projections show +INF% chance of survival versus remaining."]]),
      {shipname=player.pilot():name()}))
   l337(_([["What's the projected survival chance?"]]))
   sai(_([["NaN%"]]))
   l337(_([["Not good chances, but I don't think we have many other choices. Can the ship survive an atmospheric ascend?"]]))
   sai(_([["Negative."]]))
   l337(_([["Looks like we're going to have to trust lonewolf4's engineering. They were always an ace at this stuff, so it may work?"]]))
   l337(fmt.f(_([["{player}, I think I can handle this, you should head back to the ship."]]),
      {player=player.name()}))
   vn.na(_([[You pay your respects to lonewolf4 and leave the Wolfie behind.]]))
   storm_strength( 0.8 )
   vn.na(_([[Your ship groans around you as the storm shifts in intensity.]]))
   sai(_([["Shield integrity beginning to fail."]]))
   l337(_([["Give me a second, almost got it. I'm going to have to disable the protective field first, so hang on tight."]]))
   vn.na(_([[You sense a deep rumbling as the howling winds continue to pick up.]]))
   sai(_([["Shields down."]]))
   l337(_([["One sec..."]]))
   vn.na(_([[You grab on to your commander chair as the ship begins to slide, pulled by the storm. Your fractures kindly decide to remind you of their existence as the stim effect begins to fade.]]))
   l337(_([["Now!"]]))
   vn.func( stop_storm )
   vn.na(_([[Your stomach lurches as reality warps around you. You slowly open your eyes, and it looks like you are once more in space. Nice predictable empty space.]]))
   sai(_([["Shields recovering."]]))
   l337(_([["You made it!"]]))
   vn.na(_([[And with that, your body gives out as it becomes aware of its grievances.]]))

   -- Fade to black and then set a black shader before we transition
   vne.flashbackTextStart( _("Haziness"), {transition="blinkin"})
   tint = love_shaders.tint{ colour = {0, 0, 0, 1} }
   vn.func( function ()
      tint.shader:addPPShader()
   end )
   vn.run()

   -- Update description
   diff.remove("onion08")
   diff.apply("onion08v2")
   -- Can't stay landed on this hellhole
   player.takeoff()
   mem.state = STATE_EPILOGUE
   hook.safe( "epilogue_land" )
end

function epilogue_land ()
   -- Player was out for 20 periods or 56 hours
   time.inc( time.new(0,20,0) )
   player.land( SPOB_WAKEUP )
end

function epilogue ()
   vn.clear()
   vn.scene()

   -- Undo the global shader stuff
   vne.flashbackTextStart( _("Haziness"), {transition="blinkin"})
   vn.func( function ()
      tint.shader:rmPPShader()
   end )
   vne.flashbackTextEnd{ notransition=true }
   --vn.scene() -- vn.scene() is done in vne.flashbackTextEnd
   local l337 = onion.vn_l337b01{ pos="right" }
   local sai = tut.vn_shipai{ pos="left" }
   vn.newCharacter( l337 )
   vn.newCharacter( sai )
   vn.transition("blinkout")

   sai(fmt.f(_([["{player} has become conscious. Terminating program to find a new ship captain. I mean, welcome back to the world of the living."]]),
      {player=player.name()}))
   l337(_([["You're back! I was so worried!"]]))
   vn.menu{
      {_([["Where am I?"]]), "01_cont"},
      {_([["What happened?"]]), "01_cont"},
   }

   vn.label("01_cont")

   vn.sfxVictory()
   vn.func( function () player.pay( reward ) end )
   vn.na(fmt.reward(reward))

   vn.done("electric")
   vn.run()

   misn.finish(true)
end
