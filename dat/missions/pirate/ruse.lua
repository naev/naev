--[[

   Pirate The Ruse

   A girl asks you to find his brother but it turns out it is her assassination target.
   Creates a ship in one out of 3 listed systems. The system and planet the brother is is chosen randomly.
   spawns some mercenaries that were supposed to protect the brother to intercept the player

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas

--]]

-- variables:
targetsys = {system.get("Mural"),system.get("Darkstone"),system.get("Haleb")}
misn_marker = {}

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
   -- Bar information
   bar_desc = "This woman is looking at you. She then, a bit hecticly and desperately, waves at you."

   -- Mission details
   misn_title  = "The lost Brother"
   misn_reward = "Some money and a happy sister." -- Possibly some hard to get contraband once it is introduced
   misn_desc   = {}
   misn_desc[1] = "Locate the brother. He has to be in the %s system."
   misn_desc[2] = "Locate the brother. He is either in the %s or %s system."
   misn_desc[3] = "Locate the brother. He is either in the %s, %s or %s system."
   misn_desc[4] = "Hail the Poppy Seed and board it to reunite the siblings."

   -- Text
   title    = {}
   text     = {}
   title[1] = "In the Bar"
   title[2] = "Wrong system"
   title[3] = "Right system"
   title[4] = "Com Channel"
   title[5] = "The Deception"
   title[6] = "Mission Complete"
   text[1]  = [[After you give her attention the woman immediately starts to prattle. "Oh my dear brother, I must find him. 
   Please! Please help me. I think he is in danger. Please! Please help me! I don't have a ship and he is the only family I have left!"]]
   text[2] = [[After you signal your willingness to help the woman calms down. "Oh thank goodness! I was told where he usually hangs around. Please take me there and tell him that I have to talk to him. 
   And please hurry. I was told someone was sent to assassinate him. I don't have much to give, but whatever I have saved you can have.]]
   text[3] = [["I don't thnk he is here, probably he is in one of the other systems. Please, hurry!"]]
   text[4] = [["I think this is it! We found him!" says the woman full excitement. 
   "I just need to get something from my stuff." she says disappearing into another compartment.]]
   text[5] = [["Calling the Poppy Seed. I have your sister on board that wants to see you and told me she has urgent information for you."]]
   text[6] = [["My sister? What the heck could she want from me? Prepare for docking."]]
   text[7] = [[The woman is standing next to you while the airlock opens. You see the man. The grin on his face changes to a baffled expression. You hear the sound of a blaster. A dark spot on chest of the man. The lady rushes past you and closes the airdock before you entirely realize what happens. While the airdock mechanism locks in you see the Poppy Seed depart.]]
   text[8] = [[You find some of the stuff she left in your ship that you can turn to cash and a note saying:"Sorry"]]
   text[9] = [[Your com starts beeping: "We received confirmation that you were invovled in the killing of our associate. Prepare to be boarded."]]
   text[10] = [[What do you do: Chase the Poppy Seed or run away from the mercenaries closing in on you]]
   refusetitle = "Sorry, I can't"
   refusetext = [["How can you be such a heartless person?" asks you the woman half weepingly. " What has this universe become..."]]

   -- Messages
   msg      = {}
   msg[1]   = "MISSION SUCCESS!"
end

function create ()
   -- Note: this mission does not make any system claims. 

   -- Spaceport bar stuff
   misn.setNPC( "Ordinary Woman",  "neutral/female1")
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
   misn.setReward( string.format( misn_reward, credits) )
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
-- checking if it is right system, updating osd, if right system: create ship and wait for hail

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
      -- if we visisted a system w/o the brother: update osd
      if nmsys ~= #targetsys then
         misn.osdDestroy()
         if #targetsys == 2 then
            misn.osdCreate(misn_title, {misn_desc[2]:format(targetsys[1]:name(),targetsys[2]:name()),misn_desc[4]})
            misn.setDesc(misn_desc[2]:format(targetsys[1]:name(),targetsys[2]:name()))
         else
            misn.osdCreate(misn_title, {misn_desc[1]:format(targetsys[1]:name()),misn_desc[4]})
            misn.setDesc(misn_desc[1]:format(targetsys[1]:name()))
         end
         misn.osdActive(1)
         --update osd
      end
   else
      hook.timer( 3000,"do_msg2")
      broship = pilot.add("Civilian Gawain", "trader", bropla:pos() + vec2.new(-400,-400))[1] -- or Gawain?
      broship:addOutfit("Tricon Zephyr II Engine")
      broship:setFaction("Civilian")
      broship:rename("Poppy Seed")
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
      spx, spy = (jpx+px)/2, (jpy+py)/2
      sp = vec2.new(spx,spy)
      player.msg(tostring(px))
      player.msg(tostring(py))
      badguys = {}
      badguys = spawn_baddies(sp)

      hook.pilot(broship,"hail","got_hailed",broship,jpt,badguys)
   end
end

-- if hailed: stop vessel let it be boarded 
function got_hailed(shipp)
   tk.msg(title[4], text[5])
   tk.msg(title[4], text[6])
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
   --get nearest jumppoints at let ship escape in this way
   shipp:hyperspace(jpt:dest())
   tk.msg(title[5], text[7])
   tk.msg(title[5], text[8])
   tk.msg( title[4], text[9] )
   -- turn mercs hostile
   for i=1,#badguys do
      badguys[i]:setHostile(true)
   end
   tk.msg( title[4], text[10] )
   player.pay(rnd.rnd(40,60)*1000)
   misn.finish(true)
end
-- idle
function idle(shipp,pplanet)
   shipp:goto(pplanet:pos() + vec2.new( 400,  400), false)
   shipp:goto(pplanet:pos() + vec2.new(-400,  400), false)
   shipp:goto(pplanet:pos() + vec2.new(-400, -400), false)
   shipp:goto(pplanet:pos() + vec2.new( 400, -400), false)
end
--delay for msgs because if no delay they will pop in mid transit from system to system
function do_msg ()
   -- does nothing, but i can't find a wait function...
   tk.msg( title[2], text[3] )
end
function do_msg2 ()
   tk.msg( title[3], text[4] )
end

function spawn_baddies(sp)
   badguys = {}
   --hyenas
   for i=1,2 do
      badguys[i] = pilot.addRaw( "Hyena","mercenary", sp, "Mercenary" )
      badguys[i]:setHostile(false)
      
      badguys[i]:rename("Mercenary")
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
      
      badguys[i]:rename("Mercenary")
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
      badguys[i]:rename("Mercenary")
      
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")
      
      badguys[i]:addOutfit("Unicorp D-8 Medium Plating")
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
