--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The lost Brother">
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

require "portrait.lua"
require "missions/neutral/common.lua"


-- Bar information
bar_desc = _("The woman waves at you a bit desperately.")

-- Mission details
misn_title  = _("The lost Brother")
misn_reward = _("Some money and a happy sister.") -- Possibly some hard to get contraband once it is introduced
misn_desc   = {}
misn_desc[1] = _("Locate the brother in the %s system")
misn_desc[2] = _("Locate the brother in the %s system or the %s system")
misn_desc[3] = _("Locate the brother in the %s system, the %s system, or the %s system")
misn_desc[4] = _("Hail the Poppy Seed and board it to reunite the siblings")

-- Text
title    = {}
text     = {}
title[1] = _("In the Bar")
title[2] = _("Wrong system")
title[3] = _("Right system")
title[4] = _("Comm Channel")
title[5] = _("The Deception")
text[1]  = _([["I must find my dear brother! Please help me. I think he is in danger! I don't have a ship and he is the only family I have left. Could you please help me?"]])
text[2] = _([[The woman calms down as you signal your willingness to help. "Oh, thank goodness! I was told where he usually hangs around. Please take me there and tell him that I have to talk to him. 
    And please hurry. Someone was sent to assassinate him. I don't have much to give, but whatever I have saved, you can have."]])
text[3] = _([["I don't think he is here. He must be in one of the other systems. Please hurry!"]])
text[4] = _([["I think this is it! We found him!"]])
text[5] = _([[You radio the ship with a message saying you have his sister on board and that she has a message for him.
    "My sister? What the heck could she want from me? Prepare for docking."]])
text[7] = _([[The woman stands next to you while the airlock opens. You see the grin on the man's face change to a baffled expression, then hear the sound of a blaster. Before you even realize what has happened, the lady rushes past you and closes the airlock.
    You find an arrangement of credit chips she left in your ship along with a note: "Sorry."]])
refusetitle = _("Sorry, I can't")
refusetext = _([["How can you be such a heartless person? What has this universe become?..."]])

log_text = _([[You were tricked into aiding an assassination. A woman claimed she needed help finding her brother, but when you brought her to her "brother", she killed him and ran off, leaving behind an arrangement of credit chips and a note that simply said, "Sorry."]])


function create ()
   -- Note: this mission does not make any system claims.

   targetsys = {system.get("Mural"),system.get("Darkstone"),system.get("Haleb")}
   misn_marker = {}
   reward = rnd.rnd(40,60)*1000

   -- Spaceport bar stuff
   misn.setNPC( _("Ordinary Woman"), "neutral/unique/fakesister" )
   misn.setDesc( bar_desc )
end


--[[
Mission entry point.
--]]
function accept ()
   -- Mission details:
   if not tk.yesno( title[1], text[1] ) then
      tk.msg(refusetitle, refusetext)
      misn.finish()
   end
   misn.accept()

   -- Some variables for keeping track of the mission
   misn_done      = false
   attackedTraders = {}
   attackedTraders["__save"] = true
   fledTraders = 0
   misn_base, misn_base_sys = planet.cur()

   -- Set mission details
   misn.setTitle( misn_title )
   misn.setReward( string.format( misn_reward, reward) )
   misn.setDesc( string.format( misn_desc[1], targetsys[1]:name(), targetsys[2]:name(), targetsys[3]:name() ) )
   misn.osdCreate(misn_title, {misn_desc[3]:format(targetsys[1]:name(), targetsys[2]:name(), targetsys[3]:name()), misn_desc[4]})
   misn_marker = {[1]=misn.markerAdd( targetsys[1], "low" ), [2]=misn.markerAdd( targetsys[2], "low" ), [3]=misn.markerAdd( targetsys[3], "low" )}

   -- Some flavour text
   tk.msg( title[1], text[2] )
   -- randomly select spawn system and planet where brother will be
   brosys = targetsys[math.random(3)]
   bropla = brosys:planets()[math.random(#brosys:planets())]
   -- Set hooks
   hook.jumpin("sys_enter")
end


-- Entering a system
-- checking if it is right system, updating OSD, if right system: create ship and wait for hail

function sys_enter ()
   -- Check to see if reaching target system
   if system.cur() ~= brosys then
      nmsys = #targetsys
      for i=1,#targetsys do
         if system.cur() == targetsys[i] then
            table.remove(targetsys,i) 
            misn.markerRm(misn_marker[i])
            table.remove(misn_marker,i)
            -- we can break, we found what we were looking for
            hook.timer( 3000,"do_msg")
            break
         end
      end
      -- if we visited a system without the brother: update OSD
      if nmsys ~= #targetsys then
         misn.osdDestroy()
         if #targetsys == 2 then
            misn.osdCreate(misn_title, {misn_desc[2]:format(targetsys[1]:name(), targetsys[2]:name()), misn_desc[4]})
            misn.setDesc(misn_desc[2]:format(targetsys[1]:name(), targetsys[2]:name()))
         else
            misn.osdCreate(misn_title, {misn_desc[1]:format(targetsys[1]:name()), misn_desc[4]})
            misn.setDesc(misn_desc[1]:format(targetsys[1]:name()))
         end
         misn.osdActive(1)
         --update OSD
      end
   else
      hook.timer( 3000,"do_msg2")
      broship = pilot.add("Civilian Gawain", "trader", bropla:pos() + vec2.new(-200,-200))[1] -- fast Gawain
      broship:addOutfit("Tricon Zephyr II Engine")
      broship:setFaction("Civilian")
      broship:rename(_("Poppy Seed"))
      broship:setInvincible(true)
      broship:control()
      broship:setHilight(true)
      broship:goto(bropla:pos() + vec2.new( 400, -400), false)
      -- just some moving around, stolen from baron missions ;D
      idlehook = hook.pilot(broship, "idle", "idle",broship,bropla)
      misn.osdActive(2)
      -- get point between jumpgate and broship to spawn mercenaries disencouraging him from following
      jpt = get_nearest_jump(broship)
      jpx,jpy = jpt:pos():get()
      px,py = broship:pos():get()
      -- set spawn point between the broship and jumppoint
      spx, spy = (2*jpx/3+px/3), (2*jpy/3+py/3)
      sp = vec2.new(spx,spy)
      badguys = {}
      badguys = spawn_baddies(sp)

      hook.pilot(broship,"hail","got_hailed",broship,jpt,badguys)
   end
end

-- if hailed: stop vessel let it be boarded 
function got_hailed(shipp)
   tk.msg(title[4], text[5])
   shipp:taskClear()
   shipp:brake()
   shipp:setActiveBoard(true)
   hook.pilot(shipp, "board", "got_boarded",shipp,jpt,badguys)
   hook.rm(idlehook)
end

-- send ship to nearest jumppoint to escape, spawn baddies that might frighten you of from chasing
function got_boarded(shipp)
   player.unboard()
   shipp:setHilight(false)
   shipp:setActiveBoard(false)
   --get nearest jumppoints and let ship escape in this direction
   shipp:hyperspace(jpt:dest())
   tk.msg(title[5], text[7])
   -- turn mercs hostile
   for i=1,#badguys do
     badguys[i]:setHostile(true)
   end

   player.pay(reward)
   addMiscLog( log_text )
   misn.finish(true)
end
-- idle
function idle(shipp,pplanet)
    shipp:goto(pplanet:pos() + vec2.new( 400,  400), false)
    shipp:goto(pplanet:pos() + vec2.new(-400,  400), false)
    shipp:goto(pplanet:pos() + vec2.new(-400, -400), false)
    shipp:goto(pplanet:pos() + vec2.new( 400, -400), false)
end
--delay for msgs because if no delay they will pop in mid transit from system to system. A wait function would be awesome...
function do_msg ()
   tk.msg( title[2], text[3] )
end
function do_msg2 ()
   tk.msg( title[3], text[4] )
end

function spawn_baddies(sp)
   badguys = {}
   --hyenas
   for i=1,2 do
      badguys[i] = pilot.addRaw("Za'lek Light Drone","mercenary", sp, "Mercenary" )
      badguys[i]:setHostile(false)
      
      badguys[i]:rename(_("Mercenary"))
      --Their outfits must be quite good
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")
      
      badguys[i]:addOutfit("Unicorp D-2 Light Plating")
      badguys[i]:addOutfit("Unicorp PT-100 Core System")
      badguys[i]:addOutfit("Tricon Zephyr Engine")
      
      badguys[i]:addOutfit("Laser Cannon MK2",2)
      badguys[i]:addOutfit("Unicorp Fury Launcher")
      badguys[i]:addOutfit("Improved Stabilizer") -- Just try to avoid fight with these fellas
      
      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
   for i=3,4 do
      badguys[i] = pilot.addRaw( "Lancelot","mercenary", sp, "Mercenary" )
      badguys[i]:setHostile(false)
      
      badguys[i]:rename(_("Mercenary"))
      --Their outfits must be quite good
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")
      
      badguys[i]:addOutfit("Unicorp D-4 Light Plating")
      badguys[i]:addOutfit("Unicorp PT-200 Core System")
      badguys[i]:addOutfit("Tricon Zephyr II Engine")
      
      badguys[i]:addOutfit("Mass Driver MK1")
      badguys[i]:addOutfit("Shredder",2)
      badguys[i]:addOutfit("Ripper Cannon")
      badguys[i]:addOutfit("Shield Capacitor",2)
      
      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end

   for i=5,6 do
      badguys[i] = pilot.addRaw( "Admonisher","mercenary", sp, "Mercenary" )
      badguys[i]:setHostile(false)
      badguys[i]:rename(_("Mercenary"))
      
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")
      
      badguys[i]:addOutfit("Unicorp D-12 Medium Plating")
      badguys[i]:addOutfit("Unicorp PT-500 Core System")
      badguys[i]:addOutfit("Tricon Cyclone Engine")
      
      badguys[i]:addOutfit("Razor Turret MK3",2)
      badguys[i]:addOutfit("Unicorp Headhunter Launcher",2)
      
      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
   return badguys
end
-- gets the nearest jumppoint from a pilot
function get_nearest_jump(pilot)
   jpts = system.cur():jumps()
   -- basically the distance that the map can have at 
   dist = 2*system.cur():radius()
   index = 0
   for i,jpt in ipairs(jpts) do
      dist1 = vec2.dist(jpt:pos(),pilot:pos())
      if dist1 < dist then
         dist = dist1
         index = i
      end
   end
   return jpts[index]
end
