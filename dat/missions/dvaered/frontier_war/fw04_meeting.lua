--[[
<?xml version='1.0' encoding='utf8'?>
 <mission name="Dvaered Meeting">
 <unique />
 <priority>2</priority>
 <chance>25</chance>
 <done>Dvaered Diplomacy</done>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <notes>
   <campaign>Frontier Invasion</campaign>
 </notes>
</mission>
 --]]
--[[
-- Dvaered Meeting
-- This is the 5th mission of the Frontier War Dvaered campaign.
-- The player has to secure a Dvaered Warlord Meeting.
-- Hamelsen lures out the player, while Strafer gets killed by a spy who runs away.
-- There are two parts in the mission : part 1 is mainly contemplative, and part 2 is tricky, with fights and so.

   Stages :
   0) Way to Dvaer Prime
   1) Briefing at Dvaer Prime and patrol around DHC
   2) Following Hamelsen towards Laars (main purpose of this is to let the player save the game because second part is tricky)
   3) Taking off from Laars
   4) Land on Laars and get rewarded
--]]
-- luacheck: globals toocloseControl1 toocloseControl2 toocloseControl3 toocloseControl4 toocloseControl5 incomingControl1 incomingControl2 incomingControl3 incomingControl4 incomingControl5

local fw = require "common.frontier_war"
local dv = require "common.dvaered"
require "proximity"
local portrait = require "portrait"
local fmt = require "format"
local cinema = require "cinema"

local alpha, attackers, canland, controls, hamelsen, jules, spy, targpos, toldya, wrlrds -- Non-persistent state
local StraferNspy, equipHyena, scheduleIncoming, spawn1Wrlrd, spawnAlpha, spawnBeta, strNpc -- Forward-declared functions

message = fw.message -- common hooks

-- TODO: hooks to penalize attacking people

-- Mission constants
local destpla, destsys = spob.getS("Dvaer Prime")
local targpla = spob.get("Dvaered High Command")
local haltpla = spob.get("Laarss")
local lore_text = {}

lore_text[1] = _([["Both squadrons of the DHC station's space security force will be deployed with a full range ships from Vendettas to Goddards. Those squadrons are the 'Beta-Storks' and the 'Beta-Hammer' and their mission will be to control medium and heavy ships and to provide heavy firepower in case of need. Our squadron, named 'Alpha-NightClaws', is in charge of fast ships (Yachts and Fighters). We will be flying Hyenas.
   "The plan is the following: any ship approaching the station will be assigned to a squad by the fleet leader, and then to a pilot by the squad leader (Captain Leblanc). When a ship is assigned to you, you will have to approach the ship within 1000 km. Their security clearance code will be automatically requested and processed by the system we'll install in your core unit. Afterwards, the ship will be allowed to land, or ordered to fly away. The same thing happens for ships that leave the station.
   "Finally, in case something unexpected happens, you will, of course, have to obey orders. Watch your messages closely. A few pilots will be kept in reserve close to the station.
   "Oh, and there is another thing I must warn you about: it's the warlord's odd sense of humour. When they see a small ship close to their Goddard, they may get the idea to shoot a small railgun-volley in your direction. Some of them tend to enjoy seeing pilots fend for their lives. Dvaered law allows warlords to do so provided they can assure the High Command that there was no hostile intention. That can be a bit annoying, sometimes."]])

lore_text[2] = _([[When you mention Colonel Hamelsen, Strafer cuts you off: "Ex-Colonel Hamelsen! She is a traitor and has lost all her commendations. Now she is nothing to the Dvaered, and things are better like that." You ask him if things may have turned differently for her and he answers:
   "Watch out, citizen: these kinds of questions lead to compassion. Compassion leads to weakness and weakness leads to death. Death for yourself and for those who trusted you. Death for your leaders and for your subordinates. Death for people you love and people you respect. Remember: if you want to be strong, don't listen to compassion. Don't even give compassion a chance to reach your heart."]])

lore_text[3] = _([[You ask Strafer why Major Tam requested you for the mission. "Actually, we did not need a private pilot. I just managed to convince Captain Leblanc to hire you." As you wonder why he did that, Strafer thinks a bit and smiles: "Well, I get the feeling that together we do a good job. Don't we?"]])

lore_text[4] = _([["You wish to become one of them?" Before you have a chance to deny, he continues: "Anybody can become a warlord. One just has to have achieved the '9th grade commendation', and to conquer a planet (or a station). In the army, every rank gives you a commendation grade, for example, I have the 3rd grade. Civilians also obtain commendation for their high deeds; you obtained the 1st grade commendation for your involvement in the FLF destruction, if I am right. The 9th grade commendation, that is associated to the rank of first-class General in the army, gives the right to own a military base, and by extension, to be granted regal power over a region.
   "In the Dvaered army, everybody starts as a raw soldier, no matter if you're an infantryman, a pilot, a medic, or even a General's child. And only Valor decides how quick you rise in the hierarchy. Warlord is the ultimate rank for the military and for private combat pilots, like yourself)"]])

local lords_chatter = { _("Ahoy, suckers! Here comes the master!"),
                  _("Look down, you weaklings."),
                  _("Only submission will save you from my anger!"),
                  _("Kneel, for I am destined to rule you all!"),
                  _("Worship my strength, or burn by my railguns."),
                  _("Here comes Daddy!"),
                  _("I am an artist of Pain and Destruction. Who wants to be part of my next masterpiece?"),
                  _("Make way for the supplier of Hell!"),
                  _("Death is everyone's ultimate destination. Pissing me off means taking a shortcut."), }

-- This might be hard to translate, but that is not a problem IMHO as it is actually supposed to be an automatically generated message.

local occupations = { _("caterer"),
                _("butcher"),
                _("brewer"),
                _("waiter"),
                _("vigil"),
                _("gladiator"),
                _("rat exterminator"),
                _("emergency doctor"),
                _("weapons dealer"),
                _("drug dealer"),
                _("torturer"), }

function create()
   if spob.cur() == destpla then
      misn.finish(false)
   end

   if not misn.claim(destsys) then
      misn.finish(false)
   end

   misn.setNPC(_("Lieutenant Strafer"), fw.portrait_strafer, _("Judging by how he looks at you, Strafer needs you for another mission along with the Dvaered Space Force."))
end

function accept()
   if not tk.yesno( _("We need you once more"), _([[As you sit at his table, the clearly anxious Dvaered pilot stops biting his nails and explains why he is here.
   "The High Command summoned, under request of General Klank, a special meeting of the high council of Warlords, and all of them have agreed to come..." You frown, and before you have a chance to ask why that's a problem, he continues: "... but we received an intelligence report according to which the ex-Colonel Hamelsen, who has already tried to murder Major Tam several times, is going to take advantage of this meeting to take action against us."
   "Do you want to help us against this threat?"]]) ) then
      tk.msg(_("Too bad"), _([[Mm. I see. You probably have much more interesting things to do than being loyal to the Dvaered Nation...]]))
      return
   end
   tk.msg(_("Here is the situation"), _([["General Klank has summoned the Warlords in order to present them with the Frontier invasion plan. When a meeting of the high council of Warlords occurs, a short truce takes place, and they all come to the DHC station with their Goddards. This fact alone is already enough to put the station's security service under pressure, as the warlords constantly provoke each other and start brawls. But this time, we believe that Hamelsen will try to either assassinate warlords, or record our invasion plan in order to sell it to hostile foreign powers.
   "This is why Major Tam wants our squadron from the Special Operations Forces to support the regular units of the station. Fly to Dvaer Prime and meet me in the bar there."]]))

   misn.accept()
   misn.osdCreate( _("The Meeting"), {fmt.f(_("Land on {pnt}"), {pnt=destpla})} )
   misn.setDesc(_("You are part of the space security service for a special meeting of the high council of Warlords, where the invasion plan of the Frontier will be discussed."))
   misn.setReward(_("The greatness of House Dvaered."))
   misn.markerAdd(destpla, "low")

   mem.stage = 0
   hook.land("land")
   mem.loadhook = hook.load("loading")
   mem.reward = fw.credits_04
end

function loading()
   if (mem.stage == 1 and spob.cur() == destpla) then
      strNpc() -- Put Strafer back at loading TODO: test whether it works
   end
end

function land()
   if (mem.stage == 0 and spob.cur() == destpla) then
      strNpc()
      mem.takhook = hook.takeoff("takeoff")
      mem.stage = 1
      misn.osdDestroy()
      misn.osdCreate( _("The Meeting"), {
         fmt.f(_("Stay close to {pnt} and wait for orders"), {pnt=targpla}),
         _("Intercept suspect ship for visual identification"),
      } )

   -- Player is running away
   elseif (mem.stage == 1 or mem.stage == 3 or (mem.stage == 2 and (spob.cur() ~= haltpla) )) then
      flee()

   -- Landing on Laars after Hamelsen
   elseif mem.stage == 2 then
      mem.npc = misn.npcAdd("discussOff", _("Officer"), portrait.getMil( "Dvaered" ), _("This Dvaered officer is probably waiting for you and might have information about ex-Colonel Hamelsen's escape."))
      mem.stage = 3

   -- Player killed attackers, and can finally land for reward
   elseif (mem.stage == 4 and spob.cur() == destpla) then
      tk.msg( _("Time for a gorgeous reward?"), _([[When you step out of your ship, you see an officer alone on the dock, obviously waiting for you. As you get closer, you recognize Major Tam. The cold wind pulls the lapels of his coat, and make them whip his sad face.
   "We had better days, eh, citizen? A spy managed to run away with what seems to be a copy of our invasion plan, they killed one of my best pilots, and Hamelsen escaped... Once more." Tam looks at the sky as it starts to rain "... and it's winter on the spacedock of Dvaer Prime. Shall we enter the building? I was told that the chemical plant works twice as hard in winter, and the rain often turns to acid."
   You enter and head to the military bar. Tam looks at you: "I grew up on Nanek in Allous. For 13 years, the only part of the universe I knew was my village on Nanek, and the only people I knew were its inhabitants. And now, I've seen hundreds of planets, and thousands of people all around the galaxy. But most of them have been killed at some point. Now they are corpses, drifting here and there in space, along with the pitiful remains of their defeated ships. The night sky is filled with the souls of dead pilots. Our control of space gave us access to experiences our forefathers could not even dream of, but you know what? No matter how cold the graves of my ancestors on Nanek are, they are warmer than the emptiness of infinite space."]]), ("portraits/" .. fw.portrait_tam) )
      tk.msg( _("Time for a gorgeous reward?"), fmt.f(_([[You start wondering if the major will remember to pay you, but his voice suddenly changes: "We definitely had better days, but you know, the true valour of a warrior reveals itself in times of adversity. The dark clouds that drift above the horizon, pushed by the cruel winds of despair, are here to challenge the strength of our Dvaered souls. And it is up to us to accept this challenge.
   "I did not anticipate that the traitor Hamelsen could reassemble her group of mercenaries so fast, but you already killed some of them, and Leblanc's squadron will kill even more in the near future. We will then hunt ex-Colonel Hamelsen down, and finally we will continue the invasion.
   "Anyway, for now, we will transfer {credits} to your account, as a reward for this mission, and be certain that we will need you again soon!"]]), {credits=fmt.credits(mem.reward)}), ("portraits/" .. fw.portrait_tam) )
      player.pay(mem.reward)

      local t = time.get():tonumber()
      var.push( "invasion_time", t ) -- Timer for the next mission

      shiplog.create( "frontier_war", _("Frontier War"), _("Dvaered") )
      shiplog.append( "frontier_war", _("You were part of the security for a meeting of Dvaered warlords to discuss the plan for the invasion of the Frontier. Unfortunately, ex-Colonel Hamelsen, and her squadron, managed to seize data and escape the system. Lieutenant Strafer was killed in action during this incident.") )
      misn.finish(true)
   end
end

-- Encounter with Strafer on Dvaer Prime
function strNpc()
   toldya = {0,0,0,0}
   misn.npcAdd("discussStr", _("Lieutenant Strafer"), fw.portrait_strafer, _("Harsh voice, frank gaze, and easy trigger. Lieutenant Strafer is a Dvaered pilot."))
end

function takeoff()
   -- Taking off from Dvaer Prime for first part of mission
   if mem.stage == 1 then
      --hook.rm(mem.takhook)
      hook.jumpout("flee")

      pilot.toggleSpawn(false)
      pilot.clear()
      targpos = targpla:pos()

      spawnBeta()
      spawnAlpha()
      scheduleIncoming()

   -- Taking off from Laars for the death of Strafer and the attack from henchmen
   elseif mem.stage == 3 then
      pilot.toggleSpawn(false)
      pilot.clear()
      targpos = targpla:pos()

      spawnBeta()
      spawnAlpha()
      StraferNspy()

      -- Chatter
      hook.timer( 0.7, "message", {pilot = alpha[1], msg = _("A-NightClaws Leader to all pilots: Engage and destroy this Schroedinger at all costs!")} )
      hook.timer( 1.4, "message", {pilot = alpha[2], msg = _("A-NightClaws Second to Leader: I'm on it!")} )
      hook.timer( 2.1, "message", {pilot = spy, msg = _("Bye, suckers!")} )
      hook.timer( 2.8, "message", {pilot = alpha[2], msg = _("Just wait for my shredders to...")} )
      hook.timer( 5.0, "message", {pilot = alpha[2], msg = p_("fw04", "Come on!")} )

      hook.timer( 4.0, "deathOfStrafer" )

      misn.osdDestroy()
      misn.osdCreate( _("The Meeting"), {
         fmt.f(_("Stay close to {pnt} and wait for orders"), {pnt=targpla}),
         _("Intercept suspect ship for visual identification"),
      } )
   end
end

-- Player flees from the system
function flee()
   tk.msg(_("Look who is running away!"), _("You were supposed to secure the meeting, not to run away like that!"))
   misn.finish(false)
end

-- Player discusses with Lieutenant Strafer
function discussStr()
   local c = tk.choice( _("Lieutenant Strafer"), _("What do you want to ask the lieutenant before taking off?"), _("Ask for a briefing"), _("Ask about Colonel Hamelsen"), _("Ask why you were hired for this mission"), _("Ask how one becomes a warlord"), _("I am ready for action!") )
   if c <= 4 then
      if toldya[c] >= 3 then -- Strafer gets annoyed if one asks several times the same question
         tk.msg( _("Lieutenant Strafer"), _("Is this some kind of joke?") )
         toldya[c] = 0
      elseif toldya[c] == 2 then
         tk.msg( _("Lieutenant Strafer"), _("You are obsessed with this question. Just move on, please.") )
         toldya[c] = 3
      elseif toldya[c] == 1 then
         tk.msg( _("Lieutenant Strafer"), _("Hem... You asked the exact same question earlier.") )
         toldya[c] = 2
      else
         tk.msg( _("Lieutenant Strafer"), lore_text[c] )
         toldya[c] = 1
      end
   end
end

-- Spawn the Beta squadrons
function spawnBeta()
   local beta = {}
   beta[1] = pilot.add( "Dvaered Vendetta", "Dvaered", targpla, _("B-Storks-4") )
   beta[2] = pilot.add( "Dvaered Vendetta", "Dvaered", targpla, _("B-Storks-3") )
   beta[3] = pilot.add( "Dvaered Vendetta", "Dvaered", targpla, _("B-Storks-2") )
   beta[4] = pilot.add( "Dvaered Vendetta", "Dvaered", targpla, _("B-Storks-Lead") )
   beta[5] = pilot.add( "Dvaered Ancestor", "Dvaered", targpla, _("B-Storks-7") )
   beta[6] = pilot.add( "Dvaered Ancestor", "Dvaered", targpla, _("B-Storks-6") )
   beta[7] = pilot.add( "Dvaered Ancestor", "Dvaered", targpla, _("B-Storks-5") )
   beta[8] = pilot.add( "Dvaered Phalanx", "Dvaered", targpla, _("B-Hammer-4") )
   beta[9] = pilot.add( "Dvaered Phalanx", "Dvaered", targpla, _("B-Hammer-4") )
   beta[10] = pilot.add( "Dvaered Phalanx", "Dvaered", targpla, _("B-Hammer-3") )
   beta[11] = pilot.add( "Dvaered Vigilance", "Dvaered", targpla, _("B-Hammer-2") )
   beta[12] = pilot.add( "Dvaered Goddard", "Dvaered", targpla, _("B-Hammer-Lead") )

   for i, p in ipairs(beta) do
      p:control()
      imDoingNothing( p )
      hook.pilot( p, "idle", "imDoingNothing", p )
      p:setVisible()
   end
end

-- Spawn the alpha squadron
function spawnAlpha()
   alpha = {}
   alpha[1] = pilot.add( "Hyena", "Dvaered", targpla, _("Captain Leblanc"), {ai="baddie"} )
   alpha[2] = pilot.add( "Hyena", "Dvaered", destpla, _("Lieutenant Strafer"), {ai="baddie"} )
   alpha[3] = pilot.add( "Hyena", "Dvaered", targpla, _("A-NightClaws-3"), {ai="baddie"} )
   alpha[4] = pilot.add( "Hyena", "Dvaered", targpla, _("A-NightClaws-4"), {ai="baddie"} )
   alpha[5] = pilot.add( "Hyena", "Dvaered", destpla, _("A-NightClaws-5"), {ai="baddie"} )

   for i, p in ipairs(alpha) do
      p:control()
      imDoingNothing( p )
      hook.pilot( p, "idle", "imDoingNothing", p )
      p:setVisible()
   end
end

-- Schedule the appearance of incoming ships
function scheduleIncoming()
   -- TODO: some other civilian ships should arrive (for decoration)

   -- First the Warlords
   mem.noWrlrd = 1
   wrlrds = {}

   spawnWrlrd()
   hook.timer( 50.0, "spawnWrlrd" )
   hook.timer( 100.0, "spawnWrlrd" )
   hook.timer( 150.0, "spawnWrlrd" )
   hook.timer( 200.0, "spawnWrlrd" )

   -- Then annoying people the player has to control
   controls = { nil, nil, nil, nil, nil }
   canland = { false, false, false, false, false } -- Marks wether the ship has been controlled by the player
   mem.noCtrl = 1
   spawnControl()
   hook.timer( 40.0, "spawnControl" )
   hook.timer( 80.0, "spawnControl" )
   hook.timer( 120.0, "spawnControl" )
   hook.timer( 160.0, "spawnControl" )
   mem.n2control = 0 -- This stores the number of ships the play has to control

   hook.timer( 200.0, "spawnHam" ) -- Hamelsen comes in

   -- Briefing by Leblanc.
   tk.msg( _("Read your messages"), _("Don't forget to read the messages Captain Leblanc will send to you. They contain valuable information.") )
   hook.timer( 4.0, "message", {pilot = alpha[1], msg = _("I hope everyone is listening carefully.")} )
   hook.timer( 8.0, "message", {pilot = alpha[1], msg = _("Every incoming pilot must be visually confirmed by one of us.")} )
   hook.timer( 12.0, "message", {pilot = alpha[1], msg = _("I will notify each of you when you have a pilot to confirm.")} )
   hook.timer( 16.0, "message", {pilot = alpha[1], msg = _("And you will hear the following audio signal.")} )
   hook.timer( 17.0, "beepMe" )
   hook.timer( 20.0, "message", {pilot = alpha[1], msg = _("Last thing: Remember to beware the Warlords: some of them may shoot at you because they love to watch lighter ships fend for their lives.")} )
end

-- Play a beep
function beepMe()
   audio.soundPlay( "jump" )
end

-- Spawn Warlords, who will annoy the player
function spawnWrlrd()
   spawn1Wrlrd( system.get("Gremlin") )
   mem.noWrlrd = mem.noWrlrd + 1
   spawn1Wrlrd( system.get("Radix") )
   mem.noWrlrd = mem.noWrlrd + 1
   spawn1Wrlrd( system.get("Urbanus") )
   mem.noWrlrd = mem.noWrlrd + 1
end

-- Spawn one warlord
function spawn1Wrlrd( origin )
   local lords = dv.warlords()
   wrlrds[mem.noWrlrd] = pilot.add( "Dvaered Goddard", fw.fct_warlords(), origin, lords[mem.noWrlrd] )
   wrlrds[mem.noWrlrd]:control()

   -- Decide if the Warlord will play at shooting at the player
   mem.dontShoot = true
   if rnd.rnd() <= .3 then
      mem.dontShoot = false
   end
   wrlrds[mem.noWrlrd]:land( targpla, mem.dontShoot )

   hook.pilot( wrlrds[mem.noWrlrd], "land", "warlordTaunt", wrlrds[mem.noWrlrd] )
end

-- Broadcast a stupid taunt upon arriving
function warlordTaunt( self )
   local ind = rnd.rnd( 1, #lords_chatter )
   self:broadcast( lords_chatter[ind] )
end

-- Spawn a pilot the player has to control
function spawnControl()
   local syst = {"Gremlin", "Radix", "Urbanus"}
   local theSys = syst[ rnd.rnd(1,#syst) ]
   local origin = system.get(theSys)

   local types = { "Gawain", "Llama", "Schroedinger", "Hyena" }
   local theTyp = types[ rnd.rnd(1,#types) ]

   controls[mem.noCtrl] = pilot.add( theTyp, "Independent", origin )
   controls[mem.noCtrl]:control()
   controls[mem.noCtrl]:land(targpla)

   hook.timer(0.5, "proximity", {location = targpos, radius = 10000, funcname = ("incomingControl"..tostring(mem.noCtrl)), focus = controls[mem.noCtrl]}) -- First one for detection
   hook.timer(0.5, "proximity", {location = targpos, radius = 1500, funcname = ("toocloseControl"..tostring(mem.noCtrl)), focus = controls[mem.noCtrl]}) -- Second one for loosing

   mem.noCtrl = mem.noCtrl + 1
end

-- Some patrol pilot is idle.
function imDoingNothing( self )
   local rad = rnd.rnd() * 1000 + 1000
   self:moveto( targpos + vec2.newP(rad, rnd.angle()) )
end

-- A controlled ship is too close from station
local function toocloseControl( ind )
   if (not canland[ind]) then -- Player failed to control a ship: penalty
      tk.msg( _("An unidentified ship came close to the station"), _("A ship managed to approach the station, and you failed to confirm it. Fortunately, it was identified by the station's sensors and is not hostile. However, your failure to intercept it could have led to problems. As a consequence, your reward has been decreased.") )
      mem.reward = mem.reward - 10e3
   end
end
function toocloseControl1()
   toocloseControl(1)
end
function toocloseControl2()
   toocloseControl(2)
end
function toocloseControl3()
   toocloseControl(3)
end
function toocloseControl4()
   toocloseControl(4)
end
function toocloseControl5()
   toocloseControl(5)
end

-- A ship approaches from DHC: assign it to player
local function incomingControl( self )
   audio.soundPlay( "jump" )
   alpha[1]:comm( fmt.f(_("A-NightClaws Leader to {player}: intercept {plt} and confirm their security clearance code"), {player=player.name(), plt=self} ) )
   self:setHilight()
   self:setVisible()
   mem.n2control = mem.n2control + 1
   misn.osdActive(2)
end
function incomingControl1()
   incomingControl( controls[1] )
   hook.timer(0.5, "proximity", {anchor = controls[1], radius = 1000, funcname = "checkClearance1"}) -- Just because I cannot pass an argument to proximity hooks :(
end
function incomingControl2()
   incomingControl( controls[2] )
   hook.timer(0.5, "proximity", {anchor = controls[2], radius = 1000, funcname = "checkClearance2"})
end
function incomingControl3()
   incomingControl( controls[3] )
   hook.timer(0.5, "proximity", {anchor = controls[3], radius = 1000, funcname = "checkClearance3"})
end
function incomingControl4()
   incomingControl( controls[4] )
   hook.timer(0.5, "proximity", {anchor = controls[4], radius = 1000, funcname = "checkClearance4"})
end
function incomingControl5()
   incomingControl( controls[5] )
   hook.timer(0.5, "proximity", {anchor = controls[5], radius = 1000, funcname = "checkClearance5"})
end

-- Player checks security clearance of a ship
local function checkClearance( self )
   mem.myjob = occupations[ rnd.rnd(1,#occupations) ]
   tk.msg( _("Controlling incoming ship"), fmt.f(_([[As you approach the ship, your targeting array focuses on it and processes its clearance code. You read on your control pad: "This citizen is an honourable {job} whose presence is required for the meeting: let the ship land on the station."]]), {job=mem.myjob}) )
   self:setHilight( false )

   -- Change osd if needed
   mem.n2control = mem.n2control - 1
   if mem.n2control <= 0 then
      mem.n2control = 0 -- Sanity
      misn.osdActive(1)
   end
end
function checkClearance1()
   checkClearance( controls[1] )
   canland[1] = true
end
function checkClearance2()
   checkClearance( controls[2] )
   canland[2] = true
end
function checkClearance3()
   checkClearance( controls[3] )
   canland[3] = true
end
function checkClearance4()
   checkClearance( controls[4] )
   canland[4] = true
end
function checkClearance5()
   checkClearance( controls[5] )
   canland[5] = true
end

-- Spawn Hamelsen in a hyena
function spawnHam()
   hamelsen = pilot.add( "Hyena", "Independent", system.get("Beeklo") )
   equipHyena( hamelsen )
   hamelsen:control()
   hamelsen:land(targpla)

   hook.pilot( hamelsen, "land", "hamelsenLanded" )
   hook.timer(0.5, "proximity", {location = targpos, radius = 10000, funcname = "incomingHamelsen", focus = hamelsen})

   -- Hamelsen's partner, whose purpose is to make a fight occur
   jules = pilot.add( "Hyena", "Independent", system.get("Beeklo") )
   equipHyena( jules )
   jules:control()
   jules:follow( hamelsen )
end

-- Equips a quick and strong Hyena
function equipHyena( p )
   p:outfitRm("cores")
   p:outfitRm("all")
   p:outfitAdd("Tricon Zephyr Engine")
   p:outfitAdd("Milspec Orion 2301 Core System")
   p:outfitAdd("S&K Ultralight Combat Plating")
   p:outfitAdd("Gauss Gun",3)
   p:outfitAdd("Improved Stabilizer")
   p:outfitAdd("Hellburner")
   p:setHealth(100,100)
   p:setEnergy(100)

   -- Remove all cargo (to make them lighter)
   p:cargoRm( "all" )
end

function hamelsenLanded()
   if mem.stage == 1 then -- Hamelsen managed to land. That is badbadbad.
      tk.msg( _("An unidentified ship landed on the station"), _("A ship managed to land on the station, and you failed to confirm it. Unidentified and potentially hostile individuals have entered the Dvaered High Command station: The mission is a failure.") )
      misn.finish(false)
   else -- Landing on Laarss: indicate the player he has to follow her
      tk.msg( _("Hi there!"), fmt.f(_([[The fleeing ship suddenly hails you. You answer and the face of Colonel Hamelsen emerges from your holoscreen. "No, you won't best me, {player}. Not this time. Not any more." Aware that she is now too far away for you to catch her, you ask her why she constantly harasses Major Tam. "This is all that I have left," she answers.
   "My hate for Tam and Klank is all that remains now that my Lord is dead. I dedicated my entire life to the glory of House Dvaered, I practiced and honed my skills to serve the Army. When I was recruited by Lord Battleaddict, I became faithful to him because he gave me the opportunity to serve House Dvaered through him. And then...
   "Since the day Klank assassinated my Lord, I have been rejected by the High Command. Rejected by the Warlords. Rejected by the nation that claims to reward Valour and Righteousness. Tell me, when did I give up Valour?! Tell me, when did I give up Righteousness?! Never! The Dvaered social contract is broken as far as I am concerned.
   "All that remains of me is a vassal without a ruler, a colonel without a regiment, a corpse without a grave. I will haunt you until your demise. I will squash all your hopes and dreams, be they big or small. There will be no forgiveness, no respite, no relief, neither for you nor for me."
   After this very rousing speech, Hamelsen cuts off the communication channel and lands.]]), {player=player.name()}) )
      tk.msg( _("Follow her!"), fmt.f(_([[A new message comes from Captain Leblanc. "This is obviously a diversion! Everyone, back to your positions! {player}, go and investigate on {pnt}. Bring me the head of ex-Colonel Hamelsen!"]]), {player=player.name(), pnt=haltpla} ) )
      misn.osdDestroy()
      misn.osdCreate( _("The Meeting"), {fmt.f(_("Land on {pnt}"), {pnt=haltpla}) } )
   end
end

-- Hamelsen is in range: do as usual
function incomingHamelsen()
   audio.soundPlay( "jump" )
   alpha[1]:comm( fmt.f(_("A-NightClaws Leader to {player}: intercept {plt} and confirm their security clearance code"), {player=player.name(), plt=hamelsen} ) )
   hamelsen:setHilight()
   hamelsen:setVisible()
   hook.timer(0.5, "proximity", {anchor = hamelsen, radius = 1000, funcname = "checkHamelsen"})
   misn.osdActive(2)
end

-- Player checks security clearance of Hamelsen: let the fun begin
function checkHamelsen()
   tk.msg( _("Incoming ship refuses confirmation"), fmt.f(_([[As you come within range, an alarm goes off. This ship does not have an invitation. Suddenly, the pilot charges the blockade around the station. You hear an order from Captain Leblanc: "A-NightClaws Leader to {player}: intercept and destroy {plt}".]]), {player=player.name(), plt=hamelsen} ) )
   hamelsen:setHostile()
   --hamelsen:rename( _("Suspect Hyena") )

   hamelsen:taskClear()
   hamelsen:memory().careful = true
   hamelsen:runaway( player.pilot(), haltpla ) -- Hamelsen heads towards Laars
   hamelsen:setNoDeath()
   hamelsen:setNoDisable()

   -- Let's rock a bit !
   jules:taskClear()
   jules:attack( player.pilot() )

   mem.stage = 2
   misn.osdDestroy()
   misn.osdCreate( _("The Meeting"), {fmt.f(_("Engage {plt}"), {plt=hamelsen}) } )
end

-- Discuss with an officer on Laarss
function discussOff()
   tk.msg( _("No trace of Hamelsen"), _([[You approach the soldier, who seems to recognize you. Captain Leblanc probably sent your image ahead of time. "After we received the message from your captain, we seized the Hyena you were pursuing, but the pilot managed to escape unnoticed."
   Having no way to track Hamelsen on land, you decide it's better to take off again and to wait for further instructions from Leblanc.]]) )
   misn.npcRm( mem.npc )
end

-- Add a spy close to Gremlin, make Strafer head to it, and a group of Hyenas kill Strafer to secure the spy's exit
function StraferNspy()

   -- Compute the line on which to place the spy and Strafer
   local jppos = jump.pos( jump.get( system.cur(), "Gremlin" ) )
   local unitline = (targpos - jppos) * (1/vec2.dist(targpos-jppos))
   local strpos = jppos + unitline * 15000
   local spypos = jppos + unitline * 12000

   -- First, teleport Strafer far away from any backup
   alpha[2]:rm()
   alpha[2] = pilot.add( "Hyena", fw.fct_dhc(), strpos, _("Lieutenant Strafer") )
   alpha[2]:setVisplayer()
   alpha[2]:control()

   -- Then put the fleeing spy
   spy = pilot.add( "Schroedinger", fw.fct_warlords(), spypos )
   spy:setVisplayer()
   spy:control()
   spy:hyperspace( system.get("Gremlin") )
   spy:setNoDeath()
   spy:setNoDisable()
   alpha[2]:attack( spy )

   -- Remove all cargo (to control their speed)
   alpha[2]:cargoRm( "all" )
   spy:cargoRm( "all" )
end

-- Many enemies jump and kill Strafer
function deathOfStrafer()
   cinema.on{ gui=true }
   camera.set( alpha[2], false, 20000 )

   tk.msg( _("Something is happening at the station"), _([[You start to head to the station, but you hear a flurry of messages coming from the NightClaws squadron. A Schroedinger has managed to take off, unnoticed, from the High Command station, presumably carrying classified information. It managed to sneak through the blockade. The squadrons have been taken by surprise, but Strafer is catching up.]]) )

   local fwarlords = fw.fct_warlords()
   attackers = {}
   for i = 1, 10 do
      attackers[i] = pilot.add( "Hyena", fwarlords, system.get("Gremlin") )
      attackers[i]:control()
      attackers[i]:attack( alpha[2] )
   end

   hook.pilot( alpha[2], "exploded", "straferDied" )
end

-- Strafer just died: now, there will be action for the player
function straferDied()
   cinema.off()
   camera.set( nil, true )
   hook.timer( 1.0, "spawnKillers" )

   for i = 1, #attackers do
      attackers[i]:taskClear()
      attackers[i]:hyperspace(system.get("Gremlin"))
   end
   for i = 1, #alpha do
      if alpha[i]:exists() then
         alpha[i]:taskClear()
         alpha[i]:hyperspace(system.get("Gremlin"))
      end
   end
end

-- Killers go after the player around Laarss
function spawnKillers()
   misn.osdDestroy()
   misn.osdCreate( _("The Meeting"), {_("Eliminate the hostile fighters")} )

   local fwarlords = fw.fct_warlords()
   local killers = {}
   killers[1] = pilot.add( "Hyena", fwarlords, haltpla, _("Curiatius"), {ai="baddie_norun"} )
   killers[2] = pilot.add( "Shark", fwarlords, haltpla, _("Curiatius"), {ai="baddie_norun"} )
   killers[3] = pilot.add( "Lancelot", fwarlords, haltpla, _("Curiatius"), {ai="baddie_norun"} )

   mem.deadkillers = 0
   for i = 1, #killers do
      hook.pilot( killers[i], "exploded", "killerDied" )
   end

   tk.msg( _("Now, it's your turn"), fmt.f(_([[Your sensors suddenly pick up three hostile signals coming from Laars, just as Captain Leblanc sends her message: "To all A-NightClaw pilots: follow and intercept the hostiles." You report the three hostiles coming at you and she answers: "{player}, take care of those three. Don't let any of them escape."]]), {player=player.name()}) )

   hook.timer( 1.0, "message", {pilot = killers[1], msg = fmt.f(_("You fell in our trap, {player}!"), {player=player.name()})} )
   hook.timer( 3.0, "message", {pilot = killers[2], msg = _("You're so dead!")} )
   hook.timer( 5.0, "message", {pilot = killers[1], msg = _("Without any support!")} )
   hook.timer( 7.0, "message", {pilot = killers[3], msg = _("Folks, please do less chatting and more killing.")} )
   hook.timer( 9.0, "message", {pilot = killers[1], msg = _("Aye-aye, boss!")} )
   hook.timer( 11.0, "message", {pilot = killers[2], msg = _("Copy that!")} )
   hook.timer( 13.0, "message", {pilot = killers[1], msg = _("I'm as silent as a carp.")} )
   hook.timer( 15.0, "message", {pilot = killers[2], msg = _("Same for me!")} )
end

-- A killer died
function killerDied()
   mem.deadkillers = mem.deadkillers + 1
   if mem.deadkillers >= 3 then -- Watch out that 3 matches #killers
      mem.stage = 4
      tk.msg( _("Rid of them!"), _("As the last enemy ship explodes, you watch to your sensor screen, and notice that the alpha squadron has left the system. The fleet leader orders you to land on Dvaer Prime.") )
      misn.osdDestroy()
      misn.osdCreate( _("The Meeting"), {fmt.f(_("Land on {pnt}"), {pnt=destpla})} )
   end
end
