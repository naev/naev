--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Negotiation 1">
 <unique/>
 <priority>2</priority>
 <chance>30</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Dvaered Delivery</done>
 <cond>
   if faction.playerStanding("Dvaered") &lt; 20 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Negociation 1
-- This is the mission of the Dvaered Recruitment arc when the player has to help a Warlord getting a second Goddard Battleship.
-- The player has to transport commandos to sabotage a ship's engine in a hangar.
-- Things don't go as expected and player has to intercept a ship (pretending to need fuel) and let the commandos board.
-- Afterwards, the player needs to kill a pirate shark with a Llama. Rem: if another dummy yacht gets added in the future, it could be added

   Stages :
   0) Way to Halir to search for the engine
   1) Way to Arcturus to intercept the transport Koala
   2) Interception of the Koala
   3) Way to Zhiru to deposit back the commandos
   4) Way to Zacron to get intercepted by the pirate Shark
   5) Battle
   6) Way back for getting paid
--]]

local fmt      = require "format"
local portrait = require "portrait"
local vn       = require 'vn'
local vntk     = require 'vntk'
local dv       = require "common.dvaered"
local pir      = require "common.pirate"


-- Define the cargo commodity
local cargo_misn
local function _cargo()
   if not cargo_misn then
      cargo_misn = commodity.new( N_("Saboteurs"), N_("A group of saboteurs"), {gfx_space="person.webp"} )
   end
   return cargo_misn
end

local agentPort = "dvaered/dv_military_m2.webp"
local cyborPort = "neutral/male12.webp"
local civilPort = "neutral/female2.webp"

local silent_taunts = { _("Poor lone little Llama!"),
                        _("I am the 'Silent Death'!"),
                        _("I'm as silent as silence itself!"),
                        _("I'm as deadly as… er… death!"),
                        _("Enjoy your last moments as they are your… er… last moments!"),
                        _("Ahahahahah!"),
                        _("I know no mercy!"),
                        _("I am cruel and ruthless! And heartless! And… and unkind!"),
                        _("Your destiny is sealed!"),}

local silent_beg = { _("Hey? Stop shooting at me!"),
                     _("Please don't kill me!"),
                     _("I beg you!"),
                     _("Noooooooo!"),
                     _("Please! Have mercy!"),
                     _("I can make you rich if you spare my life!"),} -- Actually, this could become later a way to switch to a Goddard campaign

function create()
   mem.enginpnt, mem.enginsys  = spob.getS("Halir") -- Planet for the engine deposit
   mem.paypnt,   mem.paysys    = spob.getS("Zhiru") -- Where you get paid
   mem.koalasys  = system.get("Arcturus") -- System of the Koala
   mem.koalosys  = system.get("Mural") -- Origin of the Koala
   mem.sharksys  = system.get("Zacron") -- System of the Shark
   mem.sharosys  = system.get("Ogat") -- Origin of the Shark

   if not misn.claim({mem.sharksys,mem.koalasys}) then misn.finish(false) end -- Claim

   misn.setNPC( _("Dvaered Soldier"), agentPort, _("This Dvaered soldier may have a task for a private pilot") )
end

function accept()

   misn.accept()
   mem.mass = 1

   -- Discussion
   vn.clear()
   vn.scene()
   local cyb = vn.newCharacter( _("Cyborg"), { image=portrait.getFullPath(cyborPort) } )
   local sol = vn.newCharacter( _("Dvaered Soldier"), { image=portrait.getFullPath(agentPort) } )
   local doaccept = false

   vn.transition()
   sol(_("Good day, citizen. I am Colonel Okran, second in command under Lord Fatgun. Do you know Lord Fatgun?"))
   vn.menu{
      {_("Of course! I am his greatest fan!"), "fan"},
      {_("I'm afraid I do not know this person."), "info"},
      {_("I couldn't care less."), "dontcare"},
   }

   vn.label("dontcare")
   sol(_([[Oh, really? Then I guess you won't be interested in the job I have to offer.]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.label("fan")
   sol(_([[For real? Yes, Lord Fatgun truly is amazing! His career is a real success story! And he is a true full-time warlord. Not a fake media warrior like Lady Bitterfly or Lord Chainsaw who only invade a planet once in a while in order to get free advertisement for their shitty books.]]))
   vn.jump("task")

   vn.label("info")
   sol(_([[Lord Fatgun is the most successful Warlord of our time. He has managed to obtain many great victories. For example, he defeated Lady Bitterfly on Icarus and Lord Richthofen in orbit of Oni. Even before he obtained his title of Warlord, when he used to be a general for the High Command, he made some really outstanding achievements. He conducted the famous raid on Tora in Gruz, and nuked fifteen cities on that planet.]]))
   vn.jump("task")

   vn.label("task")
   sol(_([[So, as you may think, Lord Fatgun is very successful, and many lesser Warlords envy him and dream of invading his planets. For that reason, we are in need of more weapons of all kinds. I have recently bought two hundred armored ground vehicles that will help us finally kick off the troops of Lady Proserpina on Golem in Palin, where the war is raging for two cycles.]]))
   sol(_([[But before definitely bringing peace to this planet, we need to deliver those weapons there. And Lord Richthofen, who has been humiliated by his recent defeats, tends to ambush our convoys. This is why we need our transport to be escorted by a complete fleet, including a battlecruiser.]]))
   sol(_([[As you probably know, most warlords only own one Goddard battlecruiser. This is due to both the High Command being reluctant to give the authorization, and to House Goddard often refusing to sell a second battlecruiser. However, in our particular situation, having a second Goddard battlecruiser would ease our logistics by a lot.]]))
   sol(_([[And I managed to get the special authorization from the High Command. I should soon present my request for the ship to House Goddard, but before doing so, we need to make sure that Goddard shareholders are on our side. And that is why we need you.]]))
   vn.menu{
      {_("What can I do for you?"), "continue"},
      {_("I don't want to get involved in that mess."), "dontcare"},
   }

   vn.label("continue")
   sol(_([[We identified a shareholder whose name is Agrippina Grosjean. She is an allied of Lord Richthofen and she will be firmly against us buying a second battlecruiser. This is why I have elaborated a negotiation plan that consists in eliminating her. What do you think of that negotiation plan?]]))
   vn.menu{
      {_("That is a very Dvaered kind of negotiation."), "congratulate"},
      {_("This plan is very subtle Indeed."), "congratulate"},
      {_("I think it is a shitty plan."), "dontcare"},
      {_("Isn't it kind of… illegal?"), "illegal"},
   }

   vn.label("congratulate")
   sol(_([[Indeed, it is. But let me explain further.]]))
   vn.jump("illegal")

   vn.label("illegal")
   sol(_([[Actually, my plan is totally legal… Mwell, not totally. Let's say… mostly legal.]]))
   sol(fmt.f(_([[Let me explain: Grosjean has a very special hobby that consists in flying a Pirate Shark in {sys} and killing lone traders under the name 'Silent Death'. Dvaered patrols in the system are under the responsibility of her friend, the dishonourable Lord Richthofen. We have collected data on her habits, and we know she should be cruising in the system soon. So we need you to go there, pretend you are a harmless trader, wait for her to attack, and kill her.]]), {sys=mem.sharksys}))
   sol(_([[The problem is that she tends to only attack Llamas as it is probably the less dangerous ship that exists. This means that you will have to fly such a ship for her to engage you. Now, you may wonder how you are supposed to catch her in a Llama if she decides to run away, and that is where the less legal part of the mission comes into play.]]))
   sol(fmt.f(_([[Before entering {sharksys}, we will have to sabotage her ship's engine. We know that she has purchased a new engine that will be delivered to her soon. This engine will transit in the main warehouse of Tricon on {enginpnt}. Your first task will be to shuttle a group of… hem… special… workers to that planet for them to infiltrate the warehouse and sabotage the engine. You will then deposit them back on {pnt}.]]),{sharksys=mem.sharksys, enginpnt=mem.enginpnt, pnt=mem.paypnt}))
   sol(_([[And only afterwards, you will go and encounter the target. Her engine will malfunction at the very moment you will hit her ship with whatever weapon, and she won't be able to run away anymore. Moreover, as you know, the combat ability of an interceptor like the Shark is closely linked to its maneuverability, which means that this sabotage will make the target much less dangerous for your own safety. So, what do you say? Are you in?]]))
   vn.menu{
      {_("Agree"), "agree"},
      {_("Refuse"), "refuse"},
   }

   vn.label("agree")
   sol(fmt.f(_([[Hehe. I knew you would accept. It's time for you to finally work for someone really important! The first part of your mission is really straightforward: my special team will fly in your ship to {pnt} in {sys} where they will take care of the engine. Let me guide you to them.]]), {sys=mem.enginsys, pnt=mem.enginpnt}))
   vn.na(_([[Colonel Okran guides you through narrow gangways. You arrive into a large hall and cross a checkpoint guarded by spider-cyborgs and Okran tells you: "those are the soldiers of the 26th multi-terrain assault regiment (ground forces), they are deployed on this spaceport because our intelligence detected a high risk of FLF terrorist attack this sector in the close future. They look a bit odd with their biologically-grown and mechanically-enhanced pawns, but they are friendly… I mean mostly."]]))
   vn.na(_([[You finally arrive in a tiny ill-enlightened room, where a strange group is waiting for you. Two of them are much higher than average, wear an intimidating amount of implants on their skin, and carry impressive guns while the two other seem unarmed. All of them wear an incoherent assortment of colourful civilian clothes.]]))
   sol(_([[Sergeant Krakadak! Why are you all wearing ridiculous outfits? What happened here?]]))
   cyb(_([[My Colonel! You ordered us to dress like civilians. Private Krokodok told us the brother of the neighbour of his sister-in-law is a civilian, but he does not really know how this guy dresses. So we let our imagination do the work.]]))
   vn.na(_([[Colonel Okran presents you the members of the team: Sergeant Krakadak and Private Krokodok are both gamma-class cyborgs, who were detached from Dvaered High Command's Space Infantry to the personnel multirole commando company of Lord Fatgun. The two others are light engine specialists from Lord Fatgun's interception squadrons, whose role will be to actually sabotage the target.]]))
   vn.na(fmt.f(_([[Okran finally guides you to the spacedocks, where you find your ship back. The four soldiers enter it, carrying large backpacks. Before leaving, Okran tells you to meet him on {pnt} once the mission is completed. It is time to fly to {sys}.]]), {pnt=mem.paypnt, sys=mem.enginsys}))
   vn.func( function () doaccept = true end )
   vn.done()

   vn.label("refuse")
   sol(_([[Ah. I see. You are probably what is called a coward. Too bad.]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end

   -- Mission details
   mem.credits = 100e3
   misn.setTitle(_("Dvaered Negotiation 1"))
   misn.setReward( mem.credits )
   misn.setDesc( fmt.f(_("A Dvaered Warlord needs you to kill a Goddard shareholder. Strangely enough, this operation will be mostly legal.")))
   mem.misn_marker = misn.markerAdd( mem.enginpnt )
   mem.misn_state = 0
   misn.osdCreate( _("Dvaered Negotiation 1"), {
      fmt.f(_("Go to {sys} and land on {pnt}."), {sys=mem.enginsys, pnt=mem.enginpnt} ),
      fmt.f(_("Deposit the commandos back on {pnt} in {sys}."), {sys=mem.paysys,pnt=mem.paypnt} ),
      fmt.f(_("Go to {sys} with a Llama. Find the 'Silent Death' and kill her."), {sys=mem.sharksys} ),
      fmt.f(_("Get your payment on {pnt} in {sys}."), {sys=mem.paysys,pnt=mem.paypnt} ),
   } )

   -- hooks
   hook.enter("enter")
   hook.land("land")
   hook.jumpout("escape")

   -- Add the cargo
   local cmisn = _cargo()
   mem.cid = misn.cargoAdd( cmisn, mem.mass )
end

function enter()
   -- Player jumps in Arcturus to intercept the Koala
   if mem.misn_state == 1 and system.cur() == mem.koalasys then
      for k,f in ipairs(pir.factions) do
         pilot.toggleSpawn(f)
         pilot.clearSelect(f) -- We don't want them to kill our Siren of Halir
      end

      mem.misn_state = 2
      hook.timer(3.0,"spawnKoala")
      misn.osdActive(2)

   -- Player jump in Dvaered system to intercept Agrippina Grosjean
   elseif mem.misn_state == 4 and system.cur() == mem.sharksys and player.pilot():ship():nameRaw() == "Llama" then
      mem.misn_state = 5
      pilot.toggleSpawn()
      pilot.clear()
      hook.timer(2.0,"spawnShark")
   end
end

function land()
   escape() -- Test if player is not escaping a situation

   -- Player makes it to the first planet.
   if mem.misn_state == 0 and spob.cur() == mem.enginpnt then
      vn.clear()
      vn.scene()
      vn.transition( )
      vn.na(_([[You land and the saboteurs team leave your ship to disappear among the crowd on the docks. You head to the bar to wait for their return behind a drink. The atmosphere in the spaceport seems unusual. There are groups of workers wandering around and policemen guarding the stores in the shopping alley. You finally ask someone for information.]]))
      local cyb = vn.newCharacter( _("Sergeant Krakadak"), { image=portrait.getFullPath(cyborPort) } )
      local civ = vn.newCharacter( _("Worker"), { image=portrait.getFullPath(civilPort) } )
      civ(_([["We are on strike! That is what's going on! Nexus docks, who runs the spaceport, decided to lower our wages to only 0.026 credits per tonne for loaders, and 0.051 credits per kilometer for drivers. It is totally impossible for non-modified workers to survive like that, and costs for maintenance are also increasing for cyborg-workers. There are planets where the gravity is lower than here, but the wages are higher. According to some workers, there are even docker companies that offer a fixed salary, on other planets. So we are on strike to force Nexus docks to give us better remuneration."]]))
      vn.jump("menu")

      vn.label("menu")
      vn.menu{
         {_([["Why don't you go to one of those other planets, then?"]]), "other"},
         {_([["Your strike will mostly handicap the clients of the docks, not Nexus docks itself."]]), "independent"},
         {_([["Does the Imperial laws authorize strikes?"]]), "legal"},
         {_([["Do you think Tricon main warehouse might be impacted by the strike?"]]), "tricon"},
         {_("Leave"), "leave"},
      }

      vn.label("other")
      civ(_([["That is unfortunately impossible: a cheap spacebus ticket costs about 50 credits at least. That is about what we dockers can spare in several cycles. Besides, most of us have our roots on this planet. Our parents, families, children that we cannot abandon like that. You know, you are used to travelling across the stars and such, but remember that most of the population won't ever make an interplanetary travel in their whole life!"]]))
      vn.jump("menu")

      vn.label("independent")
      civ(_([["It is of the responsibility of Nexus docks to do all that is needed to satisfy their clients. And this includes providing us workers (who actually do the work) with suitable wages. Nowadays, with our current remuneration, we had to work about 5 periods per decaperiod, and at this rhythm, non-modified workers start having severe health issues at the age of 20 cycles, while cyborgs tend to fall apart even earlier. This is why Nexus docks is the only one to blame for this situation and we workers have not to be accused to defend our right to live longer than 25 cycles and to feed our children without having to force them to work."]]))
      vn.jump("menu")

      vn.label("legal")
      civ(_([["Actually, nowadays, only few people really care about Imperial laws. All I know is that Nexus sent their private police after us when we created our labor union. Then we did throw large bolts at the cops and they never came back. So I suppose what we do is legal."]]))
      civ(_([["By the way, the General Inquisitor of Nexus private police on the planet was found dead in his bath at that same period. Some say that a foreign power sent agents to disorganize the repression against our union. I don't know if it is true, or if it is just an other attempt at flagging the union as an 'agent of hostile foreign powers'."]]))
      vn.jump("menu")

      vn.label("tricon")
      civ(_([["Ahah! Of course, it is! Nothing enters nor gets out of that warehouse! I have informations that suggest that Tricon is already lobbying for Nexus docks to accept our claims because they cannot afford that strike!"]]))
      vn.jump("menu")

      vn.label("leave")
      vn.na(_([[You leave the workers and head to the bar, where you were supposed to meet the saboteurs once they have completed their mission. You soon notice the two gamma-class cyborgs and approach them.]]))
      cyb(fmt.f(_([["Unexpected event, {player}. Some lazy workers are on strike, and the engine in question was not delivered to the warehouse. We managed to contact Colonel Okran and he got the information that it was sent to a secondary warehouse in Mural to avoid the strike. It is too late to get it there, but it will soon be transferred again. Let us go back to the ship: we will intercept it in {sys}."]]), {player=player.name(),sys=mem.koalasys}))
      cyb(_([["The new plan is the following: you will find and hail that transport ship, whose name is the 'Siren of Halir'. You will pretend you're in need of fuel and they will board you. Meanwhile, we are going to do a small spacewalk and infiltrate the ship. Once we have done what is planned, we will evacuate the 'Siren of Halir'."]]))
      cyb(_([["That is actually the tricky part: you are going to have to recover us in space (like when a miner gathers ore). In the Space Forces, the officers call that kind of stunt an "extra-vehicular recovery operation", but I prefer to refer to it as "one of the stupid stunts I would refuse to do if half my brain had not been amputated before birth in that damn embryo factory". So please, please don't miss us when we will be helplessly floating out there."]]))

      vn.done()
      vn.run()

      mem.misn_state = 1
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Negotiation 1"), {
         fmt.f(_("Go to {sys}."), {sys=mem.koalasys} ),
         _("Ask the 'Siren of Halir' for fuel."),
         _("Follow the ship and recover the commandos in space."),
         fmt.f(_("Deposit the commandos back on {pnt} in {sys}."), {sys=mem.paysys,pnt=mem.paypnt} ),
         fmt.f(_("Go to {sys} with a Llama. Find the 'Silent Death' and kill her."), {sys=mem.sharksys} ),
         fmt.f(_("Get your payment on {pnt} in {sys}."), {sys=mem.paysys,pnt=mem.paypnt} ),
      } )
      misn.markerMove( mem.misn_marker, mem.koalasys )

   -- Player deposits the commandos
   elseif mem.misn_state == 3 and spob.cur() == mem.paypnt then
      vntk.msg("",_([[You drop the commandos off. The chief of the group wishes you good luck with the 'Silent Death' and they head to the stand of the Dvaered consulate on the spaceport.]]))
      misn.cargoRm( mem.cid )
      mem.misn_state = 4
      misn.osdActive(5)
      misn.markerMove( mem.misn_marker, mem.sharksys )

   -- Player gets paid
   elseif mem.misn_state == 6 and spob.cur() == mem.paypnt then
      vn.clear()
      vn.scene()
      vn.transition( )
      vn.na(_([[After landing, you notice Colonel Okran waiting for you on the dock.]]))
      local sol = vn.newCharacter( _("Colonel Okran"), { image=portrait.getFullPath(agentPort) } )
      sol(fmt.f(_([["Hello again, citizen {name}. We have been informed by House Goddard of the tragic death of Mrs Grosjean. Lord Fatgun sent flowers to her family, and I believe you may have come to receive your reward. We will re-contact you in case we need your services again in the future."
"Oh, and as an additional reward, I made sure you can now purchase the Heavy Weapon License in case you don't already have it."]]),{name=player.name()}))
      vn.na(fmt.f(_([[Colonel Okran pays you {credits}.]]), {credits=fmt.credits(mem.credits)}))

      vn.done()
      vn.run()

      -- TODO once the whole recruitment campaign is stabilized: faction.get("Dvaered"):modPlayerRaw(someQuantity)
      if diff.isApplied( "heavy_weapons_license" ) then
         dv.addStandardLog( _([[You performed a negotiation mission for Lord Fatgun, who needs to purchase a second Goddard battlecruiser. This mission consisted in killing a shareholder of Goddard who was opposed to this contract.]]) )
      else -- Player does not have the license
         dv.addStandardLog( _([[You performed a negotiation mission for Lord Fatgun, who needs to purchase a second Goddard battlecruiser. This mission consisted in killing a shareholder of Goddard who was opposed to this contract. Completing this mission has granted you access to the Heavy Weapon License.]]) )
         diff.apply("heavy_weapons_license")
      end
      player.pay(mem.credits)
      misn.finish(true)
   end
end

-- Tests to determine if the player is running away
function escape()
   -- Player should be hailing the Siren of Halir
   if mem.misn_state == 2 then
      vntk.msg("",_([[You were supposed to intercept the 'Siren of Halir'
Your mission is a failure!]]))
      misn.finish(false)

   -- Player should be killing Agrippina Grosjean
   elseif mem.misn_state == 5 then
      vntk.msg("",_([[You were supposed to kill Agrippina Grosjean!
Your mission is a failure!]]))
      misn.finish(false)
   end
end

-- Spawn the Siren of Halir for the player to board it
function spawnKoala()
   mem.koala = pilot.add("Koala", "Independent", mem.koalosys, _("Siren of Halir"))
   mem.koala:setHilight()
   hook.pilot(mem.koala,"land","koalaEscaped")
   hook.pilot(mem.koala,"jump","koalaEscaped")
   mem.hailhook = hook.pilot(mem.koala,"hail","koalaHailed")
end

-- Koala hooks
function koalaHailed()
   player.commClose()
   local price = 1e3

   if player.credits() >= price then
      vntk.msg("",fmt.f(_([[When you ask the pilot for Fuel, they first sound surprised, but propose to help for {pr} credits. You accept and they come to board you. The commandos proceed to equip their space outfits.]]), {pr=fmt.credits(price)}))
      player.pay( -price, false )
      mem.koala:control()
      mem.koala:refuel(player.pilot())
      hook.rm(mem.hailhook)
      mem.idlehook = hook.pilot(mem.koala,"idle","koalaBoard")
   else -- Player is too poor
      vntk.msg("",fmt.f(_([[When you ask the pilot for Fuel, they first sound surprised, but propose to help for {pr} credits. You realize you don't have enough credits for that!]]), {pr=fmt.credits(price)}))
   end
end
function koalaEscaped()
   vntk.msg("",_([[It appears the Siren of Halir did leave the system. Your mission is a failure!]]))
   misn.finish(false)
end
function koalaBoard()
   vntk.msg("",_([[When the 'Siren of Halir' is close enough, the members of the team leave your ship by the airlock and you wish them to safely reach the target. Once fuel transfer is finished, it is time for you to follow the Siren, waiting for the team to jump out of the ship.]]))
   misn.cargoRm( mem.cid )
   hook.timer( 15, "spreadCommando" )
   misn.osdActive(3)
   mem.koala:control(false) -- Free the Siren
   hook.rm(mem.idlehook)
end

-- Spawn the Commandos gatherable
function spreadCommando()
   local vel = mem.koala:vel()
   local poC = mem.koala:pos() - vel/vel:mod() * 10
   mem.mrk = system.markerAdd( poC, _("Commandos") )
   system.addGatherable( _cargo(), 1, poC, vel*0.5, 3600, true ) -- Spawn the commando (player-only gatherable) just behind the Koala
   audio.soundPlay( "target" )
   player.msg("#o".._("Spacewalking commandos in sight.").."#0")
   player.autonavReset(5)
   mem.gathHook = hook.gather("gather")
end

-- Player gathers the commandos
function gather( comm, qtt )
   local cmisn = _cargo()
   -- Test commodity type
   if comm~=cmisn then
      return
   end
   system.markerRm( mem.mrk )
   hook.rm(mem.gathHook)
   pilot.cargoRm( player.pilot(), comm, qtt ) -- Remove standard cargo and add mission cargo
   mem.cid = misn.cargoAdd( cmisn, mem.mass )
   audio.soundPlay( "afb_disengage" )
   player.msg( "#g".._("Commandos recovered.").."#0" )
   misn.osdActive(4)
   misn.markerMove( mem.misn_marker, mem.paypnt )
   mem.misn_state = 3
end

-- Spawn the Silent Death for the player to kill it
function spawnShark()
   mem.shark = pilot.add("Pirate Shark", "Marauder", mem.sharosys, _("Silent Death"))
   hook.pilot(mem.shark,"land","sharkEscaped")
   hook.pilot(mem.shark,"jump","sharkEscaped")
   hook.pilot(mem.shark,"death","sharkKilled")
   mem.atkhook = hook.pilot(mem.shark,"attacked","sharkAttacked")
   hook.timer( 5.0, "makeSilentDeathAnnoying" )

   -- Make sure the battle actually happends
   player.pilot():setVisible()
   mem.shark:setHilight()
   mem.shark:control()
   mem.shark:attack(player.pilot())
   mem.agressive = true
end

-- Silent Death won't shut their mouth
function makeSilentDeathAnnoying()
   if mem.shark:exists() then
      local messg
      if mem.shark:health() >= 95 then
         messg = silent_taunts[rnd.rnd(1,#silent_taunts)]
      else
         messg = silent_beg[rnd.rnd(1,#silent_beg)]
         if mem.agressive then -- Manage Silent running away
            mem.agressive = false
            mem.shark:taskClear()
            mem.shark:runaway(player.pilot())
         end
      end
      mem.shark:broadcast( messg )
      hook.timer( 5.0, "makeSilentDeathAnnoying" )
   end
end

-- Hooks for the Shark
function sharkEscaped()
   vntk.msg("",_([[It appears the Silent Death did leave the system. Your mission is a failure!]]))
   misn.finish(false)
end
function sharkAttacked()
   hook.rm(mem.atkhook)
   audio.soundPlay( "explosion0", mem.shark:pos(), mem.shark:vel() )
   mem.shark:setSpeedLimit(50)
end
function sharkKilled()
   player.msg( _("Target eliminated.") )
   pilot.toggleSpawn()
   misn.osdActive(6)
   misn.markerMove( mem.misn_marker, mem.paypnt )
   mem.misn_state = 6
end
