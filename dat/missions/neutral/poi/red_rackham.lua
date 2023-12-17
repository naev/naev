local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"

local misnvar = "poi_red_rackham"

local reward = poi.data_str(1)
local reward2 = outfit.get("Rackham's Razor")

return function(mem)
   -- Must be locked
   if not mem.locked then
      return
   end

   -- Need to have started the poi data stuff
   if poi.data_get_gained() <= 0 then
      return
   end

   -- Already finished
   local mvar = var.peek(misnvar) or 0
   if mvar >= 3 then
      return
   end

   -- Determine type of derelict based on mission number
   local ship
   if mvar == 0 then
      ship = "Pirate Shark"
   elseif mvar == 1 then
      ship = "Pirate Rhino"
   else
      ship = "Zebra"
   end
   return {
      type = "function",
      ship = ship,
      func = function()

         local sai = tut.vn_shipai()
         vn.appear(sai, tut.shipai.transition)
         sai(_([[Your ship AI appears as you access the system.
"Curious. This derelict is a particularly ancient one, my records date this model as having become obsolete almost 50 Cycles ago. It has sustained significant battle damage, but most of the interior compartments are still pressurized."]]))
         vn.disappear(sai, tut.shipai.transition)
         vn.na(_(
            [[The airlock opens with a hiss of stale, dusty air. From what you know of this wreck, you're not surprised when the smell hits you. It's exactly what you'd expect from a 50-cycle-old ship.]]))

         if mvar == 0 then
            vn.na(_(
               [[A quick exploration of the ship reveals that its systems are mostly operational, but the engines are dead and the cargo hold is full of nothing but spoiled liqour. You do however recover an age-weathered notebook from the crew quarters. You flip carefully through the few legible entries on your way back to the bridge.]]))
            local log = vne.notebookStart()
            log(_([[Captain's log, UST 548:3851
No luck today either. Lark told me this spot was like a buffet line of traders, he didn't mention that's because they're all travelling together! Transports or no, the big lugs they've got with em are more turret than ship, and all the little fish just swarm around them. Damn cattle travelling in herds, best we can do is pick off a straggler here or there. There must be an easier way to make a living.]]))
            log(_([[Captain's log, UST 548:3889
Become a pirate they said. Riches and infamy they said. Well we've been sitting out here for the devil knows how many periods now, and we've barely got 10k to show for it! It's not just the convoys, oh no that'd be bad enough, but now a couple gangs have moved into the area and they're snapping up what few stragglers there were! If this keeps up, we won't even be able to keep theaw ship running much longer.]]))
            log(_([[Captain's log, UST 548:3953
That's torn it. Some hotshot got a lucky torp through our shields, now our left retroburner's busted and we can barely manage 50 clicks a second. I guess I should be grateful it didn't blow a hole in the hull instead. Even if we manage to limp back to a station, I don't know if we have enough for repairs.

Maybe we should take that Rackham guy up on his offer after all. Sure sounds like a better deal than drifting in space forever. I just hope the old girl still has enough life in her to get us there.]]))
            vne.notebookEnd()
            reward = poi.data_str(1)
            vn.na(fmt.f(_(
               [[Reaching the bridge, you access the ship's systems and manage to extract {reward}. You then return to your ship with the data, pondering the unfortunate tale of these would-be pirates. Who exactly was this mysterious 'Rackham' they mentioned?

{rewardmsg}]]), {
               reward = reward,
               rewardmsg = fmt.reward(reward)
            }))
         elseif mvar == 1 then
            vn.na(_(
               [[A quick exploration of the ship reveals that most of its systems are completely dead, only the lights and basic life support remaining online after all this time. You eventually reach the crew quarters, but find nothing besides an age-weathered notebook. You flip carefully through it on your way towards the cargo hold, and a few entries catch your interest in particular.]]))
            local log = vne.notebookStart()
            log(_([[Captain's log, UST 551:1348
Another Empire patrol spotted us today, but the wolfpack chased them off. I couldn't help but laugh seeing them falling over themselves trying to get away once they realized how many of us there were! I tell you, it was a thing of beauty. Running into a patrol that size used to keep me up at night, but now I guess the shoe's on the other foot. I swear, signing up with Red Rackham's unit was the best damn decision I ever made.]]))
            log(_([[Captain's log, UST 551:1372
Couldn't help myself. There was a big, fat, juicy Mule right there, almost begging to get boarded! My boys haven't had a good firefight in a while, so I ordered the all ahead and guess what we find? They were carrying practically a full hold of nickel! Talk about a score! Must've been on their way back from a mining gig or something. Either way, boss is gonna be thrilled. This'll go a long way towards that crazy project of his.]]))
            log(_([[Captain's log, UST 551:1525
I swear, I thought the boss might've had a few screws loose at first. I mean, who would go to all this trouble gathering supplies just to build their own ship? But I saw it docked at the station today. The damn thing is massive! I don't know how the hell he came up with something like that, but I guess I shouldn't be surprised, he is the boss after all.

Makes me wonder though. One of my boys said the name 'Red Rackham' came from some old Earth story, but if that's not his real name then who exactly is he?]]))
            log(_([[Captain's log, UST 551:3701
Damn it. I can't believe it, it just isn't possible. The boss can't just be gone, right? There's no way that massive ship of his could've been taken down, but the boys are saying he hasn't been seen in almost 200 periods. He just disappeared without a trace. There are rumors that his last message was about ambushing some trader convoy full of gold, but the higher ups are refusing to tell us anything. Some people think they're trying to hoard the treasure for themselves.

If this keeps up, the clan is going to tear itself apart. I think it's time to get out while we're ahead, but maybe we can 'borrow' some supplies before we go.]]))
            vne.notebookEnd()
            reward = commodity.get("Industrial Goods")
            vn.na(fmt.f(_(
               [[As you finish reading and arrive at the ship's cargo hold, you realize it's full of crudely scavenged, but functional {reward}. You load up as much as you can, pondering what you've learned about Red Rackham. You wonder, how could such a dangerous pirate just disappear without a trace?]]), {
               reward = reward,
            }))
         else
            vn.na(_(
               [[As you explore the dimly lit interior of the ship, you quickly realize that the crew clearly never escaped. The withered skeletons make for a haunting experience as you wander the halls of the floating graveyard. Despite the chills creeping up your spine however you note that, scattered among the plain grey uniforms of the crew, there are more than a few skeletons with the skull-and-crossbones on their jackets.]]))
            vn.na(_(
               [[Your search of the ship eventually leads you to the ship's core systems, which you quickly realize was the site of some sort of explosion. The entire mainframe is fried beyond repair, leaving only the emergency lights and life support online. You also notice two charred skeletons nearby, whose half-burnt clothes mark the first as the captain of the ship, and the second as the leader of the pirates who had boarded it.]]))
            vn.na(_(
               [[You quickly search the room, and discover evidence of a fight between the captain and the pirate. Blaster burns pockmark the walls, and the captain's uniform is covered in bloodstained slashes. You note with some curiosity that the pirate seemed to have been using a curved, nanosteel saber instead of a blaster. You pick up the dusty blade and note its excellent craftsmanship. With the ornate engravings covering the blade, the sword is as much a work of art as it is a weapon.]]))
            vn.na(_(
               [[Despite the seemingly imbalanced weaponry at play, you can't tell for certain who won the duel. Judging by the result it might be fair to say they both lost. One thing is for certain though, a few blaster shots aren't enough to explain the explosion which clearly took place here. You do find a singed journal in the captain's pocket however, and start leafing through the most recent entries.]]))

            local log = vne.notebookStart()
            log(_([[Captain's log, UST 551:3480
This route has been getting more dangerous of late, the pirates are more bold than ever. It's no surprise, considering the significant fortune we've been carving from the asteroids, but I've also heard talk of some great, charismatic pirate leader by the name of Red Rackham. Supposedly none have seen his ship and lived to tell the tale. Personally, I put little stock in tall tales of immortal pirates. What I do trust are my ship, and my men. Any pirate foolish enough to take us lightly will have a rude awakening indeed.]]))
            log(_([[Captain's log, UST 551:3508
It isn't just the pirates who keep swarming around us unbidden. The merchantmen are spooked, and they've taken to hiding under the shadow of a bigger fish. Another half dozen joined up with us in the last period alone! No doubt they hope that in the event of a pirate attack, the slowest vessel will be left behind as a sacrifice. Particularly since that vessal, namely mine, is hauling such an appetizing cargo. I almost hope pirates do attack, if only so that I can show those cowards what a real trader looks like.]]))
            log(_([[Final log, UST 551:3517
Red Rackham is here. He jumped out of hyperspace right beside us somehow and opened fire. His ship was like nothing I've ever seen before, maybe like nothing else that's ever been built. The merchantmen scattered, but it seemed like he wasn't planning on letting any of them go. They drew enough of his fire for my crew to bring his shields down before he realized what was happening. I saw one of his engines explode right before he disabled us. We must have hit something critical, because the next thing we knew we were getting boarded by fighters.

They've taken over entire ship now, with Rackham himself leading the charge. He only left enough of us alive to make up his missing crew, no doubt he's planning to commandeer the ship and make us fly it back to some pirate station. After that though, they won't need us anymore. All I have is my blaster and one thermal detonator. If I am to die, then on my honour as captain I swear I will take Red Rackham with me.]]))
            vne.notebookEnd()
            reward = commodity.get("Gold")
            vn.na(fmt.f(_(
               [[You carefully pocket the captain's notebook, your hands shaking in anticipation, then head to the cargo hold. Sure enough, you're greeted by hundreds of tons of pure {reward}, a sizeable haul even by modern standards. You load up as much as you can and then head back to your ship, pondering the unlikely end to this strange treasure hunt. It's only as you settle into your chair that you realize {reward2} is still in your hand. As you gaze at your reflection in the ornately engraved blade, you wonder idly if your name will show up some day in the legend of Red Rackham's treasure.]]), {
               reward = reward,
               reward2 = reward2,
            }))
            vn.na(fmt.reward(reward2))
         end

         -- Set up cargo for 2nd and 3rd mission
         local function add_cargo(q)
            q = q or rnd.rnd(200, 250)
            local added
            vn.func(function()
               added = player.fleetCargoAdd(reward, q)
            end)
            vn.na(function()
               return fmt.f(_("You have received #g{amount} of {reward}#0."), {
                  amount = fmt.tonnes(added),
                  reward = reward
               })
            end)
         end

         vn.func(function()
            if mvar == 0 then
               poi.data_give(1)
            elseif mvar == 2 then
               player.outfitAdd( reward2 )
            end
            if mvar < 2 then
               poi.log(fmt.f(_(
                  [[You found an old derelict pirate ship in the {sys} system and were able to recover {reward}.]]), {
                  sys = mem.sys,
                  reward = reward
                  }))
            elseif mvar == 2 then
               poi.log(fmt.f(_(
                  [[You found an old derelict pirate ship in the {sys} system and were able to recover {reward}, and also {reward2}.]]), {
                  sys = mem.sys,
                  reward = reward,
                  reward2 = reward2
                  }))
            end
            var.push(misnvar, mvar + 1)
         end)
         if mvar > 0 then
            add_cargo()
         end
      end
   }
end
