--[[

   The beginner player tutorial.

   Does simple stuff like teach the player to fly around or use his communications system.

   Step 1
      Basic flight.

   Step 2
      Braking.

   Step 3
      Targeting.

   Step 4
      Dummy combat.

   Step 5
      Board dummy.

   Step 6
      Real pirate.

   Step 7
      Landing & Finish

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}
   title[1] = "NAEV Tutorial"
   text[1] = [[Would you like to run the tutorial to learn how to play NAEV?]]
   text[2] = [[Welcome to the tutorial, %s.]]
   -- Step 1
   title[2] = "Flight Tutorial"
   text[3] = [[We'll start off by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]]
   -- Step 2
   text[4] = [[Now, we'll try stopping. Since you can only accelerate forward, you'll have to turn around and accelerate to stop yourself. To make this easier, you can press %s to turn around, facing the opposite direction that you're moving. The stars in the background are good indicators of your velocity. 

Try stopping yourself now.]]
   text[5] = [[Now you're getting the hang of it. Moving well is integral to success in NAEV.]]
   -- Step 3
   title[3] = "Targeting Tutorial"
   text[6] = [[Next, we'll work on targeting.

You have multiple options:
 %s cycles through ships in the system.
 %s cycles the ships backwards.
 %s targets the nearest ship.
 %s targets the nearest enemy.
 %s clears the current target.]]
   text[7] = [[Your targeted ship is highlighted on the radar (Top right). You can use the %s and %s keys to change the scale of the radar. Yellow is neutral, red is hostile, green is friendly and grey is disabled.

Several ships will be brought in so you can practice targeting. The new ships will be yellow.]]
   -- Step 4
   title[4] = "Combat Tutorial"
   text[8] = [[Now, let's try using the weapon systems.

There are three important keys:
 %s fires primary weapons.
 %s selects secondary weapon.
 %s fires secondary weapon.]]
   text[9] = [[Your ship, a Llama, has two laser cannons as its primary weapons, so we'll try using them. Captain Target Practice, our designated test pilot, will soon enter. You can target him with %s, %s or %s. Following that, approach him and shoot him with %s until he has been disabled. You can also use %s to auto-face targets you have selected.

Do be warned that Captain T. Practice is known for his... talkative nature.]]
   -- Step 5
   text[10] = [[Good. Now that the Llama is disabled, you can board it. To board a ship you must target it and then stop directly above it.

Once you're on top of the ship you can press %s which will allow you to board the ship to steal resources.

Try boarding the ship now.]]
   -- Back to Step 4
   text[11] = [[You killed Captain T. Practice! You should be ashamed. Oh well, we'll ship out a replacement momentarily.]]
   text[12] = [[If the Llama had money or cargo, you could attempt to steal it. Your probability of success is based on the size of your crew versus theirs. The larger your crew, the higher your odds of success. Since Captain T. Practice has nothing, just take off and we'll introduce a real enemy.]]
   -- Step 6
   text[13] = [[Now comes the real challenge. I'll bring in a real pirate to see how you fare with him. 

Don't worry, help is on the way.]]
   text[14] = [[That wasn't so hard, was it? We'll now learn more about some essential non-combat skills.

Landing allows you to not only regenerate your shield and armour, but to trade goods, purchase outfits for your ship, and buy new ships.

It takes time to land, so if you're in a hurry for a mission, it's best to make as few stops as possible.]]
   -- Step 7
   title[5] = "Landing Tutorial"
   text[15] = [[Landing is also how you save in NAEV. This can be beneficial, but it's best to be careful. If you've failed an important mission, you should reload before landing again.

The keys involved with landing are:
 %s cycles through planets.
 %s lands or targets the nearest planet if none selected. 

The first press acquires landing clearance, the second lands the ship.]]
   text[16] = [[In order to land, you need to come to a near-stop above the planet or station you're trying to land on, much like boarding a ship. 

Try landing now.]]

   -- Mission details
   misn_title = "NAEV Tutorial"
   misn_reward = "Gameplay knowledge in spades."
   misn_desc = "Learning how to survive in the universe."
   -- Aborted mission
   msg_abortTitle = "Tutorial Aborted"
   msg_abort = [[Well, now. Seems you've already done some studying, I'll leave you to your own devices. Good luck out there.]]
   -- Rejected mission
   msg_rejectTitle = "NAEV Tutorial"
   msg_reject = [[Are you sure? If you reject this portion of the tutorial, you will be unable to complete the other portions.
   
Press yes to abort the tutorial, or no to continue it.]]
   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - Flight"
   osd_msg[1]   = {
      "Flying Around",
      "Come to a Stop"
   }
   osd_title[2] = "Tutorial - Targeting"
   osd_msg[2]   = {
      "Target ships with:",
      naev.getKey("target_next") .. " cycles through ships.",
      naev.getKey("target_prev") .. " cycles backwards.",
      naev.getKey("target_nearest") .. " targets nearest ship.",
      naev.getKey("target_hostile") .. " targets nearest hostile.",
      naev.getKey("target_clear") .. " clears the target."
   }
   osd_title[3] = "Tutorial - Combat"
   osd_msg[3]   = {
      "Disable the Llama",
      "Board the Llama",
      "Fight off a Hyena",
   }
   osd_title[4] = "Tutorial - Landing"
   osd_msg[4]   = {
      "Land on %s",
      "Keys to land:",
      naev.getKey("target_planet") .. " cycles through planets.",
      naev.getKey("land") .. " lands or gets acknowledgement."
   }
end

function create()

   var.push("version", 040)
   -- Hack to add lasers
   pilot.player():addOutfit( "Laser Cannon MK1", 2 )

   if forced == 1 then
      start()
   else
      if tk.yesno(title[1], text[1]) then
         start()
      else
         reject()
      end
   end
end

function start()
   misn.accept()

   -- Clear area of enemies.
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Set basic mission information.
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)

   -- Create OSD
   misn.osdCreate(osd_title[1], osd_msg[1])

   -- Give indications on how to fly.
   misn_stage = 1
   tk.msg(title[1], string.format(text[2], player.name()))
   tk.msg(title[2], string.format(text[3], naev.getKey("left"), naev.getKey("right"), naev.getKey("accel")))
   hook.timer(15000, "flightOver")

   -- Set Hooks
   hook.land("tutLand")
   hook.enter("tutEnter")
end

function flightOver()
   -- Update OSD
   misn.osdActive(2)

   -- Update mission stuff
   misn_stage = 2
   tk.msg(title[2], string.format(text[4], naev.getKey("reverse")))
   hook.timer(1000, "brakeOver")
end

function brakeOver()
   player = pilot.player()

   -- Check if player successfully braked
   if player:vel():mod() < 10 then

      -- New OSD
      misn.osdCreate(osd_title[2], osd_msg[2])

      -- Text and mission stuff
      misn_stage = 3
      tk.msg(title[2], text[5])
      tk.msg(title[3], string.format(text[6], naev.getKey("target_next"), naev.getKey("target_prev"), naev.getKey("target_nearest"), naev.getKey("target_hostile"), naev.getKey("target_clear")))
      tk.msg(title[3], string.format(text[7], naev.getKey("mapzoomin"), naev.getKey("mapzoomout")))
      traders = pilot.add("Sml Trader Convoy", "dummy")
      for k,v in ipairs(traders) do
         v:setFaction("Dummy")
         v:rename("Dummy")
         v:setInvincible()
      end
      hook.timer(15000, "targetEnding")
   else
      -- Keep on trying until player brakes
      hook.timer(1000, "brakeOver")
   end
end

function targetEnding()
   for k,v in ipairs(traders) do
      v:changeAI("flee")
      v:setHealth(100, 100)
   end
   hook.timer(10000, "targetOver")
end

function targetOver()
   misn_stage = 4
   
   
   -- New OSD
   misn.osdCreate(osd_title[3], osd_msg[3])

   -- Tell about combat.
   tk.msg(title[4], string.format(text[8], naev.getKey("primary"), naev.getKey("secondary_next"), naev.getKey("secondary")))
   tk.msg(title[4], string.format(text[9], naev.getKey("target_hostile"), naev.getKey("target_nearest"), naev.getKey("target_next"), naev.getKey("primary"), naev.getKey("face")))

   -- Clear pilots again.
 --  pilot.clear()

   addLlamaDummy()
end

function addLlamaDummy()
   -- Add the combat dummy.
   llamadummy = pilot.add("Trader Llama", "dummy")
   for k,v in ipairs(llamadummy) do
      v:setFaction("Dummy")
      llamaname = v:rename("Target Practice")
      hook.pilot(v, "disable", "llamaDisabled")
      hook.pilot(v, "death", "llamaDead")
      hook.pilot(v, "board", "llamaBoard")
	  pilot.setHostile(v)
	  v:comm("Your mother smells worse than the algae of Tau Prime!")
      shieldtaunt = 0
      armourtaunt = 0
	  hook.timer(5000, "taunt")
   end
end

function taunt()
	for k,v in ipairs(llamadummy) do
		armour, shield = v:health()
		hook.timer(1000, "taunt2")
	end
end

function taunt2()
      shield30 = {
            "Bring on the pain!",
            "You're no match for the fearsome T. Practice!",
            "I've been shot by ships ten times your size!",
            "Let's get it on!",
            "I  haven't got all day.",
            "You're as threatening as an unborn child!",
            "I've snacked on foes much larger than yourself!",
            "Who's your daddy? I am!",
            "You're less intimidating than a fruit cake!",
            "My ship is the best in the galaxy!",
            "Bow down before me, and I may spare your life!",
            "Someone's about to set you up the bomb.",
            "Your crew quarters are as pungent as an over-ripe banana!",
            "I'm invincible, I cannot be vinced!",
            "You think you can take me?",
            "When I'm done, you'll look like pastrami!",
            "I've got better things to do. Hurry up and die!",
            "You call that a barrage? Pah!",
            "You mother isn't much to look at, either!",
            "You're not exactly a crack-shot, are you?",
            "I've had meals that gave me more resistance than you!",
            "You're a pathetic excuse for a pilot!",
            "Surrender or face destruction!",
            "That all you got?",
            "I am Iron Man!",
            "My shields are holding fine!",
            "You'd be dead if I'd remembered to pack my weapons!",
            "I'll end you!",
            "... Right after I finish eating this bucket of fried chicken!",
            "Em 5 Fried Chicken. Eat only the best.",
            "This is your last chance to surrender!",
            "Don't do school, stay in milk.",
            "I'm going to report you to the NPC Rights watchdog.",
            "Keep going, see what happens!",
            "You don't scare me!",
            "What do you think this is, knitting hour?",
            "I mean, good lord, man.",
            "It's been several minutes of non-stop banter!",
            "Haven't I sufficiently annoyed you yet?",
            "Go on, shoot me.",
            "You can do it! I believe in you.",
            "Shoot me!",
            "Okay, listen. I'm doing this for attention.",
            "But if you don't shoot me, I'll tell the galaxy your terrible secret.",
            "...",
            "...",
            "...",
            "Go away! There are no Easter Eggs here.",
            }
       armour31 = {
            "Okay, that's about enough.",
            "You can stop now.",
            "I was wrong about you.",
            "Forgive and forget?",
            "Let's be pals, I'll buy you an ale!",
            "Game over, you win.",
            "I've got a wife and kids! And a cat!",
            "Surely you must have some mercy?",
            "Please stop!",
            "I'm sorry!",
            "Leave me alone!",
            "What did I ever do to you?",
            "I didn't sign up for this!",
            "Not my ship, anything but my ship!",
            "We can talk this out!",
            "I'm scared! Hold me.",
            "Make the bad man go away, mommy!",
            "If you don't stop I'll cry!",
            "Here I go, filling my cabin up with tears.",
            "U- oh it a-pears my te-rs hav- da--age t-e co-mand cons-le.",
            "I a- T. Pr-ct---! Y-- w--l fe-r m- na-e --- Bzzzt!"
            }
	for k,v in ipairs(llamadummy) do
      if misn_stage >= 5 then
         return
      else
         if shield >= 40 then
            if #shield30 > shieldtaunt then
               shieldtaunt = shieldtaunt + 1
               v:comm(shield30[shieldtaunt])
               hook.timer(4000, "taunt")
            else
                hook.timer(4000, "taunt")
            end
         elseif armour >= 31 then
            if #armour31 > armourtaunt then
               armourtaunt = armourtaunt + 1
               v:comm(armour31[armourtaunt])
               hook.timer(4000, "taunt")
            else
               tk.msg("Bzzzt!", "It appears our friend has suffered a systems failure. Oh well, he was getting tiresome anyhow.")
               for k,v in ipairs(llamadummy) do
                  v:disable()
                  hook.timer(2000, "llamaDisabled")
               end
            end
         end
      end
   end
end

function llamaDisabled()
   -- Update OSD
   
   misn.osdActive(2)

   misn_stage = 5
   tk.msg(title[4], string.format(text[10], naev.getKey("board")))
end


function llamaDead()
   if misn_stage < 6 then
      -- New OSD
      misn.osdCreate(osd_title[3], osd_msg[3])

      misn_stage = 4
      tk.msg(title[4], text[11])
      addLlamaDummy()
   end
end

function llamaBoard()
   -- Update OSD
   misn.osdActive(3)

   misn_stage = 6
   tk.msg(title[4], text[12])
   hook.timer(3000, "boardEnding")
end

function boardEnding()
   for k,v in ipairs(llamadummy) do
   v:changeAI("flee")
   v:setHealth(100, 100)
   v:comm("I've restored my power! You'll rue the day you crossed T. Practice!")
   v:setInvincible()
   end
   hook.timer(8000, "boardOver")
end

function boardOver()
   tk.msg(title[4], text[13])
   hyena = pilot.add("Pirate Hyena")
   for k,v in ipairs(hyena) do
       v:rmOutfit("all") -- Make weaker
       v:addOutfit("Laser Cannon MK1", 2)
       hook.pilot(v, "death", "hyenaWait")
       hook.pilot(v, "jump", "hyenaWait") -- Treat jump as dead
   end
   hook.timer(5000, "bringHelp")
end

function bringHelp()
   pilot.add("Empire Lancelot")
   pilot.add("Empire Lancelot")
   lancelots = pilot.get( { faction.get("Empire") } )
   for k,v in ipairs(lancelots) do
      v:setInvincible()
   end
end

function hyenaWait()
   hook.timer(7500, "hyenaDead")
end

function hyenaDead()
   misn_stage = 7

   -- Create OSD
   osd_msg[4][1] = string.format(osd_msg[4][1], system.cur():planets()[1]:name())
   misn.osdCreate(osd_title[4], osd_msg[4])

   -- Messages
   tk.msg(title[4], text[14])
   tk.msg(title[5], string.format(text[15], naev.getKey("target_planet"), naev.getKey("land")))
   tk.msg(title[5], text[16])
end

function tutLand()
   -- Shouldn't be landing yet.
   if misn_stage ~= 7 then
      succeed()
   else
      tutEnd()
   end
end

function tutEnter()
   enter_sys = system.cur()
   if misn_stage ~= 8 then
      abort()
   elseif enter_sys ~= misn_sys then
      hook.timer(5000, "tutEnd")

   end
end

function tutEnd()
	misn_stage = 9
	misn.finish(true)
   
end

function succeed()
   tk.msg("Tutorial Skipped", "You're a little early, but since you're here, I'll let you proceed with the next stage of the tutorial.")
   misn.finish(true)
   var.push("version", 042)
end

function abort()
   tk.msg(msg_abortTitle, msg_abort)
   var.push("tutorial_aborted", true)
   misn.finish(false)
   var.push("version", 042)
end

function reject()
   if tk.yesno(msg_rejectTitle, msg_reject) then
      abort()
   else
      start()
   end
end
