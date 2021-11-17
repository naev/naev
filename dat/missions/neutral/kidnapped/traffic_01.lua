--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Lost Brother">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>Kidnapped</done>
  <chance>12</chance>
  <location>Bar</location>
  <faction>Empire</faction>
  <faction>Dvaered</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Sirius</faction>
 </avail>
 <notes>
  <campaign>Kidnapping</campaign>
 </notes>
</mission>
--]]
--[[

   The Ruse

   A girl asks you to find his brother but it turns out it is her assassination target.
   Creates a ship in one out of 3 listed systems. The system and planet the brother is is chosen randomly.
   spawns some mercenaries that were supposed to protect the brother to intercept the player.
   Why is it in the Kidnapped campaign? Because this one never got finished and we can later claim the guy that got killed was involved in the human trafficking

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas

--]]

local fmt = require "format"
local neu = require "common.neutral"

local reward = 200e3

local badguys, broship, misn_marker -- Non-persistent state
local spawn_baddies -- Forward-declared functions
-- luacheck: globals do_msg do_msg2 got_boarded got_hailed idle sys_enter (Hook functions passed by name)

function create ()
   mem.targetsys = {system.get("Mural"),system.get("Darkstone"),system.get("Haleb")}

   -- randomly select spawn system and planet where brother will be
   mem.brosys = mem.targetsys[math.random(3)]
   mem.bropla = mem.brosys:planets()[math.random(#mem.brosys:planets())]

   if not misn.claim(mem.brosys) then
      misn.finish(false)
   end

   misn_marker = {}

   -- Spaceport bar stuff
   misn.setNPC( _("Ordinary Woman"), "neutral/unique/fakesister.webp", _("The woman waves at you a bit desperately.") )
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( _("In the Bar"), _([["I must find my dear brother! Please help me. I think he is in danger! I don't have a ship and he is the only family I have left. Could you please help me?"]]) ) then
      tk.msg(_("Sorry, I can't"), _([["How can you be such a heartless person? What has this universe become?..."]]))
      misn.finish()
   end
   misn.accept()

   -- Some variables for keeping track of the mission
   mem.misn_done      = false
   mem.attackedTraders = {}
   mem.fledTraders = 0
   mem.misn_base, mem.misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( _("The Lost Brother") )
   misn.setReward( _("Some money and a happy sister.") )
   local desc = fmt.f(_("Locate the brother in the {1} system, the {2} system, or the {3} system"), mem.targetsys)
   misn.setDesc( desc )
   misn.osdCreate(_("The Lost Brother"), {
	   desc,
	   _("Hail the Poppy Seed and board it to reunite the siblings"),
   })
   misn_marker = {[1]=misn.markerAdd( mem.targetsys[1], "low" ), [2]=misn.markerAdd( mem.targetsys[2], "low" ), [3]=misn.markerAdd( mem.targetsys[3], "low" )}

   -- Some flavour text
   tk.msg( _("In the Bar"), _([[The woman calms down as you signal your willingness to help. "Oh, thank goodness! I was told where he usually hangs around. Please take me there and tell him that I have to talk to him.
    And please hurry. Someone was sent to assassinate him. I don't have much to give, but whatever I have saved, you can have."]]) )

   -- Set hooks
   hook.jumpin("sys_enter")
end

-- gets the nearest jumppoint from a pilot
local function get_nearest_jump(pilot)
   local jpts = system.cur():jumps()
   -- basically the distance that the map can have at
   local dist = 2*system.cur():radius()
   local index = 0
   for i,jpt in ipairs(jpts) do
      local dist1 = vec2.dist(jpt:pos(),pilot:pos())
      if dist1 < dist then
         dist = dist1
         index = i
      end
   end
   return jpts[index]
end


-- Entering a system
-- checking if it is right system, updating OSD, if right system: create ship and wait for hail

function sys_enter ()
   -- Check to see if reaching target system
   if system.cur() ~= mem.brosys then
      mem.nmsys = #mem.targetsys
      for i=1,#mem.targetsys do
         if system.cur() == mem.targetsys[i] then
            table.remove(mem.targetsys,i)
            misn.markerRm(misn_marker[i])
            table.remove(misn_marker,i)
            -- we can break, we found what we were looking for
            hook.timer( 3.0, "do_msg" )
            break
         end
      end
      -- if we visited a system without the brother: update OSD
      if mem.nmsys ~= #mem.targetsys then
         misn.osdDestroy()
         local desc
         if #mem.targetsys == 2 then
            desc = fmt.f(_("Locate the brother in the {1} system or the {2} system"), mem.targetsys)
         else
            desc = fmt.f(_("Locate the brother in the {1} system"), mem.targetsys)
         end
         misn.osdCreate(_("The Lost Brother"), {
            desc,
            _("Hail the Poppy Seed and board it to reunite the siblings"),
         })
         misn.setDesc(desc)
         misn.osdActive(1)
         --update OSD
      end
   else
      hook.timer( 3.0, "do_msg2" )
      broship = pilot.add( "Gawain", "Independent", mem.bropla:pos() + vec2.new(-200,-200), _("Poppy Seed"), {ai="trader"} ) -- fast Gawain
      broship:outfitRm("cores")
      broship:cargoRm("all")
      broship:outfitAdd("Unicorp D-2 Light Plating")
      broship:outfitAdd("Unicorp PT-68 Core System")
      broship:outfitAdd("Tricon Zephyr Engine")
      broship:setHealth(100,100)
      broship:setEnergy(100)
      broship:setInvincible(true)
      broship:control()
      broship:setHilight(true)
      broship:setVisible(true)
      broship:setFuel(true)
      broship:moveto(mem.bropla:pos() + vec2.new( 400, -400), false)
      -- just some moving around, stolen from baron missions ;D
      mem.idlehook = hook.pilot(broship, "idle", "idle",broship,mem.bropla)
      misn.osdActive(2)
      -- get point between jumpgate and broship to spawn mercenaries disencouraging him from following
      mem.jpt = get_nearest_jump(broship)
      -- set spawn point between the broship and jumppoint
      local sp = mem.jpt:pos() * (2/3) + broship:pos() * (1/3)
      badguys = {}
      badguys = spawn_baddies(sp)

      hook.pilot(broship,"hail","got_hailed",broship,mem.jpt,badguys)
   end
end

-- if hailed: stop vessel let it be boarded
function got_hailed(shipp)
   tk.msg(_("Comm Channel"), _([[You radio the ship with a message saying you have his sister on board and that she has a message for him.
    "My sister? What the heck could she want from me? Prepare for docking."]]))
   shipp:taskClear()
   shipp:brake()
   shipp:setActiveBoard(true)
   hook.pilot(shipp, "board", "got_boarded",shipp,mem.jpt,badguys)
   hook.rm(mem.idlehook)
   player.commClose()
end

-- send ship to nearest jumppoint to escape, spawn baddies that might frighten you of from chasing
function got_boarded(shipp)
   player.unboard()
   shipp:setHilight(false)
   shipp:setActiveBoard(false)
   --get nearest jumppoints and let ship escape in this direction
   shipp:hyperspace(mem.jpt:dest())
   tk.msg(_("The Deception"), _([[The woman stands next to you while the airlock opens. You see the grin on the man's face change to a baffled expression, then hear the sound of a blaster. Before you even realize what has happened, the lady rushes past you and closes the airlock.
    You find an arrangement of credit chips she left in your ship along with a note: "Sorry."]]))
   -- turn mercs hostile
   for i=1,#badguys do
     badguys[i]:setHostile(true)
   end

   player.pay(reward)
   neu.addMiscLog( _([[You were tricked into aiding an assassination. A woman claimed she needed help finding her brother, but when you brought her to her "brother", she killed him and ran off, leaving behind an arrangement of credit chips and a note that simply said, "Sorry."]]) )
   misn.finish(true)
end
-- idle
function idle(shipp,pplanet)
    shipp:moveto(pplanet:pos() + vec2.new( 400,  400), false)
    shipp:moveto(pplanet:pos() + vec2.new(-400,  400), false)
    shipp:moveto(pplanet:pos() + vec2.new(-400, -400), false)
    shipp:moveto(pplanet:pos() + vec2.new( 400, -400), false)
end
--delay for msgs because if no delay they will pop in mid transit from system to system. A wait function would be awesome...
function do_msg ()
   tk.msg( _("Wrong system"), _([["I don't think he is here. He must be in one of the other systems. Please hurry!"]]) )
end
function do_msg2 ()
   tk.msg( _("Right system"), _([["I think this is it! We found him!"]]) )
end

function spawn_baddies(sp)
   badguys = {}
   --hyenas
   for i=1,2 do
      badguys[i] = pilot.add("Za'lek Light Drone", "Mercenary", sp, _("Mercenary") )
      badguys[i]:setHostile(false)

      --Their outfits must be quite good
      badguys[i]:outfitRm("all")
      badguys[i]:outfitRm("cores")

      badguys[i]:outfitAdd("Unicorp D-2 Light Plating")
      badguys[i]:outfitAdd("Unicorp PT-16 Core System")
      badguys[i]:outfitAdd("Tricon Zephyr Engine")

      badguys[i]:outfitAdd("Laser Cannon MK2",3)
      badguys[i]:outfitAdd("Improved Stabilizer") -- Just try to avoid fight with these fellas

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
   for i=3,4 do
      badguys[i] = pilot.add( "Lancelot", "Mercenary", sp, _("Mercenary") )
      badguys[i]:setHostile(false)

      --Their outfits must be quite good
      badguys[i]:outfitRm("all")
      badguys[i]:outfitRm("cores")

      badguys[i]:outfitAdd("Unicorp D-4 Light Plating")
      badguys[i]:outfitAdd("Unicorp PT-68 Core System")
      badguys[i]:outfitAdd("Tricon Zephyr II Engine")

      badguys[i]:outfitAdd("Mass Driver")
      badguys[i]:outfitAdd("Shredder",2)
      badguys[i]:outfitAdd("Ripper Cannon")
      badguys[i]:outfitAdd("Shield Capacitor I",2)

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
   for i=5,6 do
      badguys[i] = pilot.add( "Admonisher", "Mercenary", sp, _("Mercenary") )
      badguys[i]:setHostile(false)

      badguys[i]:outfitRm("all")
      badguys[i]:outfitRm("cores")

      badguys[i]:outfitAdd("Unicorp D-12 Medium Plating")
      badguys[i]:outfitAdd("Unicorp PT-200 Core System")
      badguys[i]:outfitAdd("Tricon Cyclone Engine")

      badguys[i]:outfitAdd("Razor Turret MK2",2)
      badguys[i]:outfitAdd("Unicorp Headhunter Launcher",2)

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
   return badguys
end
