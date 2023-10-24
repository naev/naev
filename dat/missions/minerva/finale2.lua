--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Finale 2">
 <unique />
 <priority>1</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Regensburg</spob>
 <done>Minerva Finale 1</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]
--[[
   Final mission having to revisit and break into Minerva Station
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local vni = require 'vnimage'
local fmt = require "format"
local audio = require 'love.audio'
local pp_shaders = require "pp_shaders"
local lg = require "love.graphics"
local ccomm = require "common.comm"
local tut = require 'common.tutorial'

local mainspb, mainsys = spob.getS("Minerva Station")
local returnspb, returnsys = spob.getS("Shangris Station")
local title = _("Minerva Station Redux")

-- Mission states:
--  nil: mission not accepted yet
--    1. Get to Minerva Station
--    2. Infiltrated
--    3. Finish
mem.state = nil

function create ()
   misn.finish( false )
   if not misn.claim( mainsys, true ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.maikki.name, minerva.maikki.portrait, _("Maikki seems to be waiting for you in her regular clothes.") )
   misn.setDesc(fmt.f(_([[Maikki has entrusted you with raiding the weapons laboratory at Minerva Station to obtain anything that can help Kex, while also plundering anything you find of value. Afterwards she has told you to meet her up at {returnspb} in the {returnsys} system.]]),
      {returnspb=returnspb, returnsys=returnsys}))
   misn.setReward(_("Saving Kex!"))
   misn.setTitle( title )

   -- Apply diff if necessary
   local minervadiff = "minerva_1"
   if not diff.isApplied( minervadiff ) then
      diff.apply( minervadiff )
   end
end

local talked -- Reset when loads
function accept ()
   vn.clear()
   vn.scene()
   local maikki = vn.newCharacter( minerva.vn_maikki() )
   vn.music( minerva.loops.maikki )
   vn.transition("hexagon")

   if not talked then
      talked = true

      vn.na(_([[You join Maikki at her table. Although she has changed into her civilian clothes, her composure and facial expression is quite unlike when you first met her.]]))
      maikki(_([["I've talked with the pirate head surgeon, and there's both good news and bad news."]]))
      vn.menu{
         {[["Good news?"]], "01_good"},
         {[["Bad news?"]], "01_bad"},
      }

      -- Just change the order she says stuff in
      local vn01good, vn01bad
      vn.label("01_good")
      maikki(_([["The good news is that Zuri is somewhat stable. It's still not clear if she's going to pull it off, but we have to trust her fortitude. We'll have to fly her back to get some more proper follow-up care. However, at least the worst has been avoided for now."]]))
      vn.func( function ()
         vn01good = true
         if not vn01bad then
            vn.jump("01_bad")
         else
            vn.jump("01_cont")
         end
      end )

      vn.label("01_bad")
      maikki(_([["The bad news is that we don't know what the deal is with my father. He seems to be alive, however, the surgeon had no idea what to do. I guess this is a first for them. We got a second look by the lead engineer who told us that it seems like some of his circuitry was damaged."]]))
      vn.func( function ()
         vn01bad = true
         if not vn01good then
            vn.jump("01_good")
         else
            vn.jump("01_cont")
         end
      end )

      vn.label("01_cont")
      maikki(_([["Damn it, I should have brought my squad with me, but I erred on the side of cautious. How unlike me! But with Zuri and Kex's state, we're going to have to take them to a proper medical facility. That doesn't mean we can leave Minerva Station as it is, after all we've been through!"]]))
      local winner = var.peek("minerva_judgement_winner")
      local msg
      if winner=="dvaered" then
         msg = _([["Minerva Station is going to be swarming with Dvaereds brutes in no time when it gets handed over. ]])
      elseif winner=="zalek" then
         msg = _([["Minerva Station is going to be swarming with Za'leks and their pesky drones in no time when it gets handed over. ]])
      else -- "independent"
         msg = _([["We don't know what's going to happen to the station after the trial ended as it did. I wouldn't be surprised if it would be flooded with Imperials in a short time. ]])
      end
      maikki(msg.._([[We have to infiltrate the weapons facility at the station and not only plunder some nice weapon prototypes and blueprints, but now we also have to see if we can find anything that can help my father! It's a bit of a long shot, but it's our best bet."]]))
      vn.menu( {
         {_([["Wait, you knew about the station?"]]), "02_plan"},
         {_([["Anything for Kex!"]]), "02_kex"},
      } )

      vn.label("02_plan")
      maikki(_([["A pirate has to be efficient: finding my father, and infiltrating a weapons facility. Two birds with one stone! I wasn't entirely sure, but I had a good lead on the fact that the Minerva CEO was doing quite a bit more than gambling."]]))
      vn.jump("02_cont")

      vn.label("02_kex")
      maikki(_([["Yes. You have no idea how much shit I've had to put up with to find him. There is no way I'm going to lose him now! The rest can wait!"]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      maikki(_([["Although I'm really tempted to storm Minerva Station myself, and plunder the weapons facility like any pirate dreams of, but it pisses me off that I'm going to have to leave it to you. Damn responsibilities."]]))
      maikki(_([["Zuri got most of the information and should be the one to brief you on the weapons facility, but that's not going to work out now."
She lets out a sigh.
"I'll try to give you a short rundown on what we know."]]))
      maikki(_([[She clears her throat.
"So, our intel hints that they are working on an experimental energy weapon of some time at the station. Should be quite preliminary design, but we don't have much info on the current state of development. Either way, it's going to be useful and/or worth a fortune!"]]))
      maikki(_([["Apparently, the laboratory is located in a penthouse to not raise suspicion. Security is quite tight around the area, but we've got the likely location narrowed down. I forget where it was, but I'll send you the documents we have on it."]]))
      maikki(_([["We don't really know what we'll find there, but I guess you'll have to improvise? It's pretty much now or never, and we won't get a second chance once the station gets locked down."]]))
      maikki(_([["You'll have to make it to the station on your own, but once you get there, some pirate operatives should be there to help you ransack the place. You won't have too much time, so just try to grab whatever you can and get out of there. Make sure to keep an eye out for any sort of thing that can help Kex. I'm not sure our engineers will be able to figure it out by themselves."]]))
      maikki(_([[She gives a solemn nod.
"Are you ready to infiltrate Minerva Station one last time?"]]))
   else
      maikki(_([["Are you ready now to infiltrate Minerva Station?"]]))
   end

   vn.menu( {
      {_("Accept the Undertaking"), "accept"},
      {_("Prepare more"), "decline"},
   } )

   vn.label("decline")
   vn.na(_([[Maikki gives you some time to prepare more. Return to her when you are ready.]]))
   vn.done("hexagon")

   vn.label("accept")
   vn.func( function () mem.state=1 end )
   maikki(_([["Great! I knew I could count on you."]]))
   maikki(fmt.f(_([["I want to take Kex and Zuri to {spb} in the {sys} system, where I know a gal that should help patch them up. Not as good as the surgeons back at New Haven, but it'll have to do. We don't have the time to make the full trip at the moment, once you join us, we can figure out if we can make the trip."]]),
      {spb=returnspb, sys=returnsys}))
   maikki(fmt.f(_([["You infiltrate the station, get what you can, and meet up with us at {spb}. We'll then figure out what to do. Best of luck!"
Maikki gives you a weird two finger salute and takes off to her ship, leaving you to do your part on your own.]]),
      {spb=returnspb}))

   vn.done("hexagon")
   vn.run()

   -- If not accepted, mem.state will still be nil
   if mem.state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( title, {
      fmt.f(_("Land on {spb} ({sys} system)"),{spb=mainspb, sys=mainsys}),
      _("Ransack the weapon laboratory"),
      fmt.f(_("Meet up with Maikki at {spb} ({sys} system)"),{spb=returnspb, sys=returnsys}),
   } )
   mem.mrk = misn.markerAdd( mainspb )

   hook.enter("enter")
   hook.land("land")
   hook.load("land")
end

local boss, guards, hailhook, bosshailed
function enter ()
   local scur = system.cur()
   if scur==mainsys and mem.state==1 then
      if mem.boss_died then
         mainspb:landAllow(true)
         return
      end

      pilot.clear()
      pilot.toggleSpawn(false)

      local pos = mainspb:pos() + vec2.new( 100+200*rnd.rnd(), rnd.angle() )
      boss = pilot.add( "Empire Hawking", "Empire", pos, nil, {ai="guard"} )
      guards = { boss }
      for k,v in ipairs{"Empire Lancelot", "Empire Lancelot"} do
         local p = pilot.add( v, "Empire", pos+rnd.rnd(50,rnd.angle()), nil, {ai="guard"} )
         p:setLeader( boss )
         table.insert( guards, p )
      end

      bosshailed = false
      mainspb:landDeny(true,_([["Special authorization needed."]]))

      hailhook = hook.hail_spob( "comm_minerva" )
      hook.pilot( boss, "hail", "comm_boss" )
      hook.pilot( boss, "death", "boss_death" )
   else
      -- Clean up some stuff if applicable
      if hailhook then
         hook.rm( hailhook )
         hailhook = nil
      end
   end
end

local landack, timetodie
local askwhy, left01, left02, left03, triedclearance, inperson
local function talk_boss( fromspob )
   if timetodie then
      player.msg(_("Communication channel is closed."))
      return
   elseif landack then
      if boss and boss:exists() then
         boss:comm(_("Proceed to land."))
      end
      return
   elseif inperson then
      if boss and boss:exists() then
         boss:comm(_("Please bring the documents in person."))
      end
      return
   end

   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, boss )
   vn.transition()
   if fromspob then
      vn.na(fmt.f(_([[You try to open a communication channel with {spb}, but get rerouted to an Imperial ship.]]),
         {spb=mainspb}))
   end
   if bosshailed then
      p(_([["What do you want again? I told you Minerva Station is locked down until further notice."]]))
   else
      p(_([["What do you want? Minerva Station is locked down until further notice."]]))
      vn.func( function () bosshailed = true end )
   end

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {_([["Why is it locked down?"]]), "01_why"},
         {_([[Leave.]]), "leave"},
      }
      table.insert( opts, 2, {_([["I'm here to do cleaning."]]), "01_contractor"} )
      if left03 then
         table.insert( opts, 2, {_([["Please, let me land!"]]), "01_left4"} )
      elseif left02 then
         table.insert( opts, 2, {_([["It's a matter of life and death!"]]), "01_left3"} )
      elseif left01 then
         table.insert( opts, 2, {_([["What I left is really important!"]]), "01_left2"} )
      else
         table.insert( opts, 2, {_([["I left something at the station."]]), "01_left"} )
      end
      if askwhy and not triedclearance then
         table.insert( opts, 2, {_([["I brought Imperial clearance."]]), "01_clearance"} )
      end
      return opts
   end )

   vn.label("01_why")
   p(_([["Imperial Decree 28701-289 is why. No access without proper Imperial clearance. No exceptions."]]))
   vn.func( function () askwhy = true end )
   vn.jump("menu")

   vn.label("01_left")
   p(_([["Tough luck buddy. Judge's orders. Not going to risk a pay cut for someone I don't know. File an EB-89212-9 with information on it and you may be able to get it back."]]))
   vn.func( function () left01 = true end )
   vn.jump("menu")

   vn.label("01_left2")
   p(_([["Everyone thinks everything is important to them, buddy. The richest man is he who needs nothing, but just go file an EB-89212-9 if you want it back."]]))
   vn.func( function () left02 = true end )
   vn.jump("menu")

   vn.label("01_left3")
   p(_([["Why, you should have have said so at the beginning! No, seriously, buddy. File the EB-89312-9. What, or was it EB-89213-9? Anyway, you're not getting through. Don't push your luck."]]))
   vn.func( function () left03 = true end )
   vn.jump("menu")

   vn.label("01_left4")
   p(_([["Would have preferred to have an office job instead of having to deal with punks..."
They let out a sigh.]]))
   vn.jump("timetodie")

   vn.label("01_contractor")
   vn.func( function ()
      local s = player.pilot():ship()
      local t = s:tags()
      if t.standard and t.transport then
         vn.jump("contractor_ok")
      else
         vn.jump("contractor_bad")
      end
   end )
   vn.label("contractor_ok")
   p(_([["Great! We were expecting you, it's a mess down there. Proceed to land."]]))
   vn.jump("landack")
   vn.label("contractor_bad")
   p(fmt.f(_([["Wait, why are you in a {ship}? Cleaning crew usually comes in a Koala or Mule? Show me your credentials!"]]),
      {ship=player.pilot():ship()}))
   vn.menu{
      {_([["Company is doing a fleet renewal."]]),"contractor_bad_renewal"},
      {_([["Left it at the office."]]),"contractor_bad_left"}
   }
   vn.label("contractor_bad_renewal")
   vn.func( function ()
      local pp = player.pilot()
      local weaps = pp:outfitsList("weapon")
      if #weaps >= 2 then
         vn.jump("contractor_bad_renewal_fight")
      else
         vn.jump("contractor_bad_left")
      end
   end )
   vn.label("contractor_bad_renewal_fight")
   p(_([["What's the deal with all the weapons you're sporting then? You look like you're looking for trouble, but the Empire always takes out the trash!"]]))
   vn.jump("timetodie")
   vn.label("contractor_bad_left")
   p(_([["Go get them then. You are not getting landing access without your credentials."]]))
   vn.na(_([[The communication channel cuts off.]]))
   vn.done()

   vn.label("01_clearance")
   vn.func( function () triedclearance = true end )
   p(_([["OK, please send the clearance codes."]]))
   vn.menu{
      {_([[Send a meme.]]),"clearance_meme"},
      {_([[Send random data.]]),"clearance_random"},
   }
   vn.label("clearance_meme")
   p(_([[You hear a chuckle before they clear their throat.
"You do know that impersonation is an Imperial felony, right?"]]))
   vn.menu{
      {_([["That chuckle means I can land right?"]]),"clearance_meme_die"},
      {_([[Apologize.]]),"clearance_meme_apologize"},
   }
   vn.label("clearance_meme_die")
   p(_([["You brought this upon yourself!"]]))
   vn.jump("timetodie")
   vn.label("clearance_meme_apologize")
   vn.na(_([[You apologize profusely.]]))
   p(_([["Make sure it doesn't happen again. I'll be writing a report on this, next time, expect no leniency."]]))
   vn.na(_([[The communication channel closes.]]))
   vn.done()
   vn.label("clearance_random")
   vn.na(fmt.f(_([[You stream a large block of random data to {boss}.]]),
      {boss=boss}))
   p(_([["I'm sorry, but it seems like the communication channel is corrupting. Please bring the documents in person."]]))
   vn.func( function ()
      inperson = true
      boss:setActiveBoard(true)
      hook.pilot( boss, "board", "board_boss" )
   end )
   vn.done()

   vn.label("timetodie")
   vn.func( function () timetodie=true end )
   vn.na(_([[The communication channel cuts off as your sensors pick up signals of weapons powering up.]]))
   vn.done()

   vn.label("landack")
   vn.na(_([[Your navigation system lights up green as you receive a confirmation of landing access to Minerva Station.]]))
   vn.func( function () landack=true end )
   vn.done()

   vn.menu("leave")
   vn.na(_([[You close the communication channel.]]))
   vn.done()

   vn.run()

   if landack then
      mainspb:landAllow(true)
   elseif timetodie then
      for k,v in ipairs(guards) do
         v:setHostile(true)
      end
      if hailhook then
         hook.rm( hailhook )
         hailhook = nil
      end
   end
end

function comm_boss()
   talk_boss( false )
   player.commClose()
end

function comm_minerva( commspb )
   if commspb ~= mainspb then return end
   talk_boss( true )
   player.commClose()
end

function boss_death ()
   mem.boss_died = true
   if hailhook then
      hook.rm( hailhook )
      hailhook = nil
   end

   mainspb:landAllow(true)
   player.msg(fmt.f(_("With the blocking ship out of the way, you should be able to land on {spb} now."), {spb=mainspb}))
end

function board_boss ()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[You approach the {plt} to dock. The {plt} has their shields down for boarding, it would be a good time to try to make use uf the situation.]]),
      {plt=boss}))
   vn.menu{
      {_([[Board normally.]]),"01_board"},
      {fmt.f(_([[Open fire on the {plt} at point blank.]]),{plt=boss}),"01_fire"},
      {fmt.f(_([[Ram the {plt}.]]),{plt=boss}),"01_ram"},
   }

   vn.label("01_fire")
   vn.func( function ()
      boss:setHealth( 70, 0 )
      boss:setEnergy( 0 )
      for k,v in ipairs(guards) do
         v:setHostile(true)
      end
      mainspb:landAllow(false) -- Clear landing status
   end )
   vn.na(fmt.f(_([[You quickly power up your weapons and aim at critical ship infrastructure right as the boarding clamps begin to extend. Catching the {plt} off guard, your weapons are able to do significant damage, even knocking several systems offline. However, you have no choice now but to finish the job. Time to power on your weapons.]]),
      {plt=boss}))
   vn.done()

   vn.label("01_ram")
   vn.func( function ()
      if player.pilot():mass() > 0.4 * boss:mass() then
         vn.jump("ram_good")
      else
         vn.jump("ram_bad")
      end
   end )
   vn.label("ram_good")
   vn.func( function ()
      boss:setHealth( 40, 0 )
      boss:setEnergy( 0 )
      boss:disable(true) -- not permanent
      for k,v in ipairs(guards) do
         v:setHostile(true)
      end
      mainspb:landAllow(false) -- Clear landing status
   end )
   vn.na(fmt.f(_([[Right before boarding, you power on your shields and slam on your ship thrusters. Catching the {plt} completely off guard with your maneuver, your ship smashes into their hull causing massive damage. Time to power on your weapons.]]),
      {plt=boss}))
   vn.done()
   vn.label("ram_bad")
   vn.func( function ()
      local ppm = player.pilot():mass()
      local bm = boss:mass()
      boss:setHealth( (bm-ppm)/bm, 0 )
      boss:setEnergy( 0 )
      for k,v in ipairs(guards) do
         v:setHostile(true)
      end
      mainspb:landAllow(false) -- Clear landing status
   end )
   vn.na(fmt.f(_([[Right before boarding, you power on your shields and slam on your ship thrusters. Catching the {plt} completely off guard with your maneuver, your ship smashes into their hull causing significant damage. Time to power on your weapons.]]),
      {plt=boss}))
   vn.done()

   vn.label("01_board")
   vn.na(_([[You board the ship and are escorted to the command room where the captain is waiting. Getting this far and not having any other choice, you give them a holodrive with some randomly generated contents. They plug in the drive and try to authorize it, however, an error appears as expected.]]))
   vn.na(_([[The first officer attempts percussive maintenance on the main system, however, as expected, the error does not go away. The captain is unfazed by the situation, and, muttering something about the new Imperial Operating System update, says something about verifying it later on your way out. Surprisingly enough, you are given permission to land.]]))
   vn.na(_([[You quickly make your way back to your ship to prepare to land before they see through your bullshit.]]))
   vn.func( function ()
      landack=true
      mainspb:landAllow(true)
   end )
   vn.done()

   vn.run()

   boss:setActiveBoard(false)
   player.unboard()
end

local pirimage, pirportrait
function land ()
   if mem.state==1 and spob.cur()==mainspb then
      pirimage, pirportrait = vni.pirate()
      misn.npcAdd( "approach_pir", _("Pirate?"), pirportrait, _("A pirate-ish individual. Maybe this is one of Maikki's crew?") )
   elseif mem.state==2 and spob.cur()==returnspb then
      vn.clear()
      vn.scene()
      local maikki = vn.newCharacter( minerva.vn_maikki() )
      vn.music( minerva.loops.maikki )
      vn.transition("hexagon")

      vn.na(_([[You land and are immediately greeted by a Maikki. How did she track your ship?]]))
      maikki(_([["What did you find? Did you meet Marley and Sage?"]]))
      vn.na(_([[She takes you to an impromptu installation on her ship, the Pink Demon. There you give Maikki the holodrives that she promptly passes off to her engineers to inspect the contents, while you explain to her the ordeal you went through. She shows sorrow when you explain the fates of Marley and Sage, the two pirates you met, but such is the way of pirates.]]))
      vn.na(fmt.f(_([[She explains to you the situation of Zuri and Kex, while Kex hasn't changed at all, Zuri hasn't improved significantly and is under a deep coma. While the installations at {spb} are not bad, they are far from being the forefront in medical technology.]]),
         {spb=returnspb}))
      vn.na(_([[Eventually an engineer comes back to report on the situation. It seems like some schematics of Kex's design were recovered, along with some other blueprints that could prove to be useful, or at least in the worst case, worth a pretty penny on the black market. Maikki looks ecstatic at the possibility, albeit remote, of being able to repair her father.]]))
      maikki(fmt.f(_([["Great job! I knew we could pull this off. If we have the blueprints or schematics or whatever they are, they should be able to repair my father! {spb} may have shitty medical installations, but the engineers here should be top notch!"]]),
         {spb=returnspb}))
      vn.na(_([[The engineers get the OK from Maikki to proceed, and begin working on whatever they have to do on Kex. Given the importance of the operation, you and Maikki leave it to the specialists while you fret together in the nearby waiting room until the procedure is finished.]]))
      maikki(_([["This is nerve-wracking. I'm a pirate for hell's sake, not supposed to be worrying about lives, just booty!"]]))
      vn.menu( function ()
         local opts = {
            {_([["Is life but not another booty?"]]), "01_booty"},
            {_([["Pirates care about lives too!"]]), "01_lives"},
            {_([["It's only human."]]), "01_human"},
         }
         return rnd.permutation(opts)
      end )

      vn.label("01_booty")
      maikki(_([["That is an interesting way of seeing it. I guess treasuring one's comrades and family isn't much different from treasuring hordes of stolen values."]]))
      vn.jump("01_cont")

      vn.label("01_lives")
      maikki(_([["It's hard to care about lives when they are worth so little in this universe. One day you meet a nice pirate, the next day you find out they're dead. It sort of desensitizes you to life."]]))
      vn.jump("01_cont")

      vn.label("01_human")
      maikki(_([["Yes, but normal humans are a droll. They spend all day worrying about taxes and commuting to and back from work. Freedom is what a pirate seeks, and worrying seems to be exactly the opposite!"]]))
      vn.jump("01_cont")

      vn.label("01_cont")
      maikki(_([["I don't know. I guess It's just weird having my father in my life again. Plus, it's not like Zuri, who can do well on her own, he's like... you know... a fowl! If I don't watch out for him a stray cat or whatever will have a banquet!"]]))
      maikki(_([["I feel like I have to protect him, you know? Even though the asshole has been missing my entire life, he's still family. All I have left now! If I lose him, there's nothing left..."]]))
      vn.menu( function ()
         local opts = {
            {_([["He's a cyborg fowl superweapon, he's not helpless."]]), "02_fowl"},
            {_([["You have to protect him."]]), "02_protect"},
            {_([["He'll be alright."]]), "02_alright"},
         }
         return rnd.permutation(opts)
      end )

      vn.label("02_fowl")
      maikki(_([["I know but, still, he's soo small!"
She lets out a sigh.]]))
      vn.jump("02_cont")

      vn.label("02_protect")
      maikki(_([["I know right! He's so small and feeble looking. Makes you want to cuddle with him and keep all the danger away."
She lets out a sigh.]]))
      vn.jump("02_cont")

      vn.label("02_alright")
      maikki(_([["I hope you're right. I mean, there's no other possibility is there not?"
She lets out a sigh.]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      maikki(_([["This is going to take a while to get used to. Guess it's best not to overthink it for now and go with the flow."]]))
      vn.na(_([[You are interrupted by an engineer that comes in to inform that it was likely a success and they would try to wake up Kex.]]))
      maikki(_([[Maikki closes her eyes for a second and takes a deep breath.
"Let's go!"]]))
      vn.na(_([[You are led to a makeshift surgery room covered in devices and screens. On the operating table you see a tiny-looking Kex who's hooked up to some electrical gear.]]))
      maikki(_([["It's now or never. Let's go this!"]]))
      vn.musicStop()
      vn.func( function () music.stop() end )
      vn.na(_([[The engineers hit a key on a console, and everyone goes silent in anticipation.]]))
      vn.na(_([[...]]))
      vn.na(_([[Looking over, Maikki seems to be holding her breath while the engineers scratch their heads. Did it not work?]]))
      vn.na(_([[.]]))
      vn.na(_([[ .]]),true)
      vn.na(_([[ .]]),true)

      -- Time to bring back Kex!!!
      vn.move( maikki, "left" )
      local kex = minerva.vn_kex{ pos="right" }
      vn.appear( kex, "hexagon" )

      vn.na(_([[Suddenly you hear a loud BEEP and the light in Kex's cyborg eye turns on.]]))
      kex(_([["INITIALIZING STRANGELOVE OS VERSION 0.18-alpha.239.]]))
      kex(_([[ .]]),true)
      kex(_([[ .]]),true)
      kex(_([[

INITIALIZATION COMPLETE."]]),true)
      kex(_([[Kex blinks several times before letting out a large SQUAWK.]]))
      maikki(_([[Likely out of shock, Maikki quickly slaps and surely slaps Kex across the cheek.]]))
      vn.music( minerva.loops.kex )
      kex(_([[Kex blinks several times again.]]))
      maikki(_([["You better not squawk again."
She looks ready to slap him again.]]))
      kex(_([[Kex looks around the room once before staring at you.]]))
      vn.menu{
         {_([["Go on, say something."]]),"03_cont"},
         {_([["Welcome back!"]]),"03_cont"},
         {_([["SQUAWK!"]]),"03_squawk"},
      }

      vn.label("03_squawk")
      kex(_([[He jumps a bit in surprise.
"Crikey! You scared me."]]))
      maikki(_([["Now you know how I feel!"]]))
      vn.jump("03_cont")

      vn.label("03_cont")
      kex(_([["Where... Where am I?"
He looks quite nervous.]]))
      maikki(fmt.f(_([["We're on the Pink Demon, stationed at {spb}."]]),
         {spb=returnspb}))
      kex(_([["Pink... Demon?"
He squints a bit looking at Maikki, then suddenly seems to get a bit agitated. He turns again to you.]]))
      vn.menu{
         {_([["Glad to have you back!"]]),"04_back"},
         {_([["How many fingers am I holding up?"]]),"04_fingers"},
         {_([["How's daddy feeling?"]]),"04_daddy"},
      }

      vn.label("04_back")
      kex(_([[He still seems a bit fuzzy.
"I remember... a courtroom? What happened?"]]))
      vn.jump("04_cont")

      vn.label("04_fingers")
      kex(_([["I'm fine. I remember... a courtroom? What happened?"]]))
      vn.jump("04_cont")

      vn.label("04_daddy")
      kex(_([[Kex freezes in place.]]))
      maikki(_([[Maikki elbows you in the ribs, almost sending you sprawling to the floor.
"How are you feeling?"]]))
      kex(_([[He still seems quite tense, but it doesn't seem like he's very focused.
"I remember... a courtroom? What happened?"]]))
      vn.jump("04_cont")

      vn.label("04_cont")
      vn.na(_([[You roughly give him an overview of what happened at the courtroom without going into too much depth, and some of the posterior challenges leading up until the current moment.]]))
      kex(_([["I see..."
He still doesn't seem to have relaxed significantly.]]))
      maikki(_([["Well then."
Maikki makes emphasizes her presence to Kex.]]))
      kex(_([[He still seems to avoid eye contact with Maikki and looks at you.
"What are you going to do now?"]]))
      maikki(_([[Before you can answer, Maikki butts in.
"Isn't there something more important right now?"]]))
      kex(_([[His voice trembles a bit.
"Ummmmm, I guess... something..."
His voice becomes smaller and smaller and ends up trailing off.]]))
      maikki(_([["You do recognize me, you bastard!"
The engineers have to hold Maikki back from Kex as she lets her temper take the best of her.]]))
      maikki(_([["Let me at the fowl! I spend 5 bloody cycles tracking him down thinking that he must have forgotten about me or whatever when he turned into a bird, and he remembered all along!"
She struggles against the engineers.]]))
      maikki(_([["He could have been all like 'It's good to see you daughter!' or 'I'm sorry I wasn't there!', but the bloody fowl was just running away from his problems and avoiding facing reality like all along!"]]))
      maikki(_([["You haven't changed at all!"
She breaks free from the engineers and jumps at Kex. Fearing the worst, you try to jump to intercept, but Maikki's alacrity surprises you.]]))
      maikki(_([[Maikki tackles Kex, and for a second you think it's the end of the cyborg duck, but, despite the tumultuousness, you find Maikki hugging Kex. She whispers softly,
"I missed you, you bastard..."]]))
      kex(_([["I'm sorry, Maikki... I'm sorry for everything... I failed you..."]]))

      vn.scene()
      vn.transition("hexagon")

      vn.na(_([[With the emotional meeting out of the way, and knowing Kex is safe and sound, the engineers clean up things a bit, and you decide to leave the two a bit of privacy, while you go grab a drink to try to recoup after such a tumultuous chain of events.]]))
      vn.na(_([[Eventually you get a message from Maikki that she is waiting for you on the bridge of the Pink Demon and you head back to meet the two.]]))

      vn.scene()
      vn.newCharacter( kex ) -- TODO pirate hat
      local maikkip = vn.newCharacter( minerva.vn_maikkiP{pos="left"} )
      vn.transition("hexagon")
      vn.na(_([[You find Maikki in her pirate attire, and surprisingly enough, Kex is also in a matching outfit.]]))
      vn.menu{
         {_([["All caught up I guess?"]]), "05_cont"},
         {_([["Nice look!"]]), "05_cont"},
      }
      vn.label("05_cont")
      maikkip(_([["Thanks! I owe it to you to finally be able to be able to talk to my father. We have a ton of catching up to do still!"]]))
      kex(_([["I feel the same. Thank you for helping me out, despite my current appearance. Maybe this time I will be able to amend my past and break the loop."]]))
      maikkip(_([["Amend the past? That's impossible. You were a complete asshole."]]))
      kex(_([[He looks bit dejected and sad.]]))
      maikkip(_([["Don't be sad. You have to learn to live with your past, and learn from it to have a good future! What's done is done, but you can still decide what's yet to come."]]))
      kex(_([["It's still hard. Although I've had a lot of time to think since I lost my body. This time I have no excuses."]]))
      maikkip(fmt.f(_([[She beams a smile at Kex.
"Ah, {playername}, my father decided to join me and my crew aboard the Pink Demon. We'll be heading back to New Haven after this. Zuri is still in need of good treatment. What are you going to do? Care to join us?"]]),
         {playername=player.name()}))
      vn.na(_([[You thank Maikki for her offer, but say that you want to forge your own path. That doesn't mean you won't be able to join her on new adventures, but that you want to keep your autonomy.]]))
      maikkip(_([["Hah! One would think you are the pirate instead of me!"
She gives you a sly look.]]))
      maikkip(_([["To be honest, unless hell froze over, I knew there was no chance of you joining us. Wild Ones aren't for everyone, and you've got things you want to do."]]))
      kex(_([["But make sure to visit us! I'm not sure how to thank you enough."]]))
      maikkip(_([["Ah, I almost forgot. What is a good adventure without a good reward?"]]))
      kex(_([["A good learning experience?"]]))
      maikkip(_([["That was a rhetorical question! You have to..
Oh never mind. Here, take this credit stick, it's the least that I can do for you."]]))
      vn.sfxVictory()
      local reward_amount = minerva.rewards.finale2
      vn.func( function ()
         player.pay( reward_amount )
      end )
      vn.na(fmt.reward(reward_amount))
      kex(_([["Wow, that's a lot of credits! I also want to thank you. but I've lost everything I had. I'll find a way to pay you back, I promise!"]]))
      vn.na(_([[You wish the two best on their trip back, and promise you will go visit them sometime. You then head back after the long ordeal with the knowledge you were able to reunite a daughter and father.]]))

      vn.done("hexagon")
      vn.run()

      faction.modPlayerSingle("Wild Ones", 15)
      minerva.log.maikki(fmt.f(_([[You managed to infiltrate the weapon laboratory on Minerva Station, obtaining important document including schematics related to Kex's cyborg parts. Despite being blown out into space and rescued by your ship AI, you were able to reach {returnspb} and hand over the documents. This allowed Maikki's engineers to resuscitate Kex and you were able to witness an emotional reunion between Kex and Maikki. They then took off to New Haven and you promised you would visit them sometime.]]),
         {returnspb=returnspb}))

      -- Update minerva
      if diff.isApplied("minerva_1") then
         diff.remove("minerva_1")
      end
      diff.apply("minerva_2")

      misn.finish(true)
   end
end

local shader_fadeout, hook_update
function approach_pir ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( _("Pirate?"), {image=pirimage} )
   vn.transition()

   vn.na(_([[You approach the shady character, who seems to be sitting idly around until they notice you.]]))
   pir(_([[They take a good look at you first before deciding you must be the person they are waiting for.
"Maikki sent you right? I've been expecting you."]]))
   pir(_([["We don't have too much time, the place is starting to get full of bureaucrats and it's only a question of time before they start locking things down and asking questions."]]))
   pir(_([["You're up to date with everything right? We're pretty sure that the weapon laboratory is in penthouse number 5. It just doesn't match the station blueprints and there seems to be excess power routed there."]]))
   pir(_([["I've got us some disguises prepared that should let us get close enough to break it."]]))
   vn.na(_([[You head to the lavatory and get changed into the outfit. It seems to be some sort of mix between a jumpsuit and a labcoat, and the size is quite off, making you look a bit funny. After getting changed you meet up with the pirate who is also now in a similar attire.]]))
   pir(_([["Not the best fit, but we have no choice to try. Let's get headed."]]))
   vn.na(_([[You follow them through the winding corridors of the station. Except for passing a couple distracted bureaucrats, it is quite uneventful. Eventually you find yourself near a door that seems to have a guard posted.]]))
   pir(_([["Act natural."]]))
   vn.na(_([[They go to the door and give a nod to the guard before entering the room. You follow their step and imitate their actions.]]))
   vn.na(_([[You find yourself with the pirate inside what seems to be a laboratory packed with racks full of boxes and weird gadgets. Other than you and the pirate, there's some bureaucrats that seem to be performing inventory while a pair of guards watches. It's going to be tricky to do anything with so many people in the room.]]))
   vn.na(_([[You and the pirate split up, acting as if you belong there while you inspect the rest of the room. Past the racks there seems to be a small clearing with what looks like some laser weapon prototype. You are looking around when you notice the pirate is talking to a bureaucrat.]]))

   vn.label("01_menu")
   vn.na(_([[What do you do?]]))
   vn.menu{
      {_([[Fiddle with the laser weapon prototype.]]), "01_fiddle"},
      {_([[Look around more.]]), "01_look"},
   }

   vn.label("01_look")
   vn.na(_([[As you walk around looking more, you accidentally trip on some cable that gets pulled out of the socket. You hear some beeping that seems to draw some of the attention of the guards.]]))
   vn.menu{
      {_([[Plug the cable back in.]]), "02a_plugin"},
      {_([[Pretend nothing happened.]]), "02a_pretend"},
   }

   vn.label("02a_plugin")
   vn.na(_([[You fumble with the cable as you try not to draw extra attention and plug it in the wall, which seems to reboot something and trigger the prototype.]]))
   vn.jump("prototype_shot")

   vn.label("02a_pretend")
   vn.na(_([[You pretend nothing happens, and walk away as the incessant beeping continues. The guards eventually stop paying attention, and a group of bureaucrats comes to look at it instead, as you pretend to tally things at a safe distance.]]))
   vn.na(_([[Suddenly the beeping stops and there is finally some peaceful silence, an instant before one of the prototypes capacitors near the bureaucrats ruptures in a violent explosion!]]))
   vn.jump("chaos")

   vn.label("01_fiddle")
   vn.na(_([[You look at the laser weapon prototype that is placed on a pedestal. It is connected to all sorts of wires with many of the internals exposed, and an assortment of what seem to be buttons. One particularly large red button seems to call your attention.]]))
   vn.menu{
      {_([[Press the button.]]), "02b_button"},
      {_([[Investigate the prototype in more detail.]]), "02b_investigate"},
      {_([[Do something else.]]), "01_menu"},
   }

   vn.label("02b_investigate")
   vn.na(_([[You look around and try to make sense of the prototype. It's highly connected to the laboratory so it does not look like you would be able to remove it easily. Furthermore, it's not even clear if it is truly functional or just a heap of scrap metal. Sadly, you lack the expertise to do a full proper evaluation. The only thing you know is that it looks pretty damn cool.]]))
   vn.jump("01_menu")

   vn.label("02b_button")
   vn.na(_([[You press the big red button, noticing out of the corner of your eye a 'DO NOT PRESS' button a bit too late.]]))
   vn.sfx( audio.newSource( 'snd/sounds/laser.wav' ) ) -- TODO potentially change
   vn.label("prototype_shot")
   vn.na(_([[There's a resounding brrrruunnggzzzzz as the prototype fires, with a shock wave launching you to the ground, and the blast misses the target in front of it, partially ricocheting off the laboratory walls until it smashes into a group of bureaucrats, immediately sending them off in pieces.]]))
   vn.label("chaos")
   vn.move( pir, "farleft" )
   local emp1 = vn.Character.new( _("Guard A"), {image=vni.empireMilitary(),pos="right"} )
   local emp2 = vn.Character.new( _("Guard B"), {image=vni.empireMilitary(),pos="farright"} )
   vn.appear( {emp1,emp2} )
   vn.na(_([[The room breaks into chaos with the pirate sucker punches the bureaucrat he was talking to, and you trying to recover your breath. The two guards quickly draw their weapons and you and the pirate find yourselves looking at the end of a rifle. Things aren't looking too hot.]]))
   pir(_([["Shit."]]))
   emp1(_([["Put your hands where I can see them!"]]))
   vn.na(_([[You and the pirate exchange looks, but have no choice but to raise your hands as the guards slowly approach with their weapons raised and ready.]]))
   vn.na(_([[Suddenly you hear the door open, the outside guard must have been drawn to the commotion. Seeing this as a last chance, the pirate tackles one of the guards and you duck for cover as you hear several shots being fired.]]))

   vn.scene()
   vn.transition("slidedown")
   vn.na(_([[As the dust settles, you notice there is only one other person left standing.]]))
   local emp3 = vn.Character.new( _("Outside Guard"), {image=vni.empireMilitary()} )
   vn.appear( emp3 )
   vn.na(_([[The guard from the outside looms before you, with smoke pouring out of their weapon. You tentatively raise your hands again in hopes of not getting shot.]]))
   emp3(_([[The guard goes over to the pirate and rolls over the body, revealing a large gaping hole in their chest.
"Damnit, they got Marley! Them bastards."]]))
   vn.menu{
      {_([["Wait... You're with Maikki?"]]), "03_maikki"},
      {_([["..."]]), "03_silence"},
   }

   vn.label("03_maikki")
   emp3(_([["Aye, I'm with Maikki. So was Marley before they got a hole in them. Poor feller."]]))
   vn.jump("03_cont")

   vn.label("03_silence")
   emp3(_([["It's 'lright. I'm with Maikki."]]))
   vn.jump("03_cont")

   vn.label("03_cont")
   emp3(_([["What a mess in 'ere. Worse than that time on Caladan. We're goin'ta have to move fast. I've blocked the door, but not sure how we're gonna get out."]]))
   vn.na(_([[Despite the mess, you follow the disguised pirate's lead and try to find things that can be of use. You manage to find some holodrives that you stuff into your jumpsuit pockets. Hopefully they'll contain something of use.]]))
   vn.na(_([[As you hastily scour the laboratory for anything of use, you hear some banging on the door. It looks like backup has made it here. You're not sure how long the door is going to last.]]))
   emp3(_([["Shit, sounds like alotta 'em. Don't think we have the firepower to hold 'em off."]]))
   vn.na(_([[What do you do?]]))
   vn.menu{
      {_([[Attempt to aim the prototype at the door.]]),"04_prototype"},
      {_([[Try to blockade the door.]]),"04_blockade"},
   }

   vn.label("04_prototype")
   vn.na(_([[You begin to try to aim the prototype at the door, when you noticed that it seems like the entire prototype power system has gone down. You fumble to try to power the system but it seems futile. You are running out of time.]]))
   vn.jump("04_cont")

   vn.label("04_blockade")
   vn.na(_([[You try to blockade the door with the largest furniture you can find, but soon enough you find your efforts are in vain: they are using a laser cutter to break down the door. You are running out of time.]]))
   vn.jump("04_cont")

   vn.label("04_cont")
   vn.na(_([[You suddenly notice that the disguised pirate was doing something near the back of the room.]]))
   emp3(_([["'elp me! We ain't got time!"]]))
   vn.na(_([[Not really knowing what they're doing, you help them move some canisters to a wall.]]))
   emp3(_([["Git down!"]]))
   vn.na(_([[Without much of a warning, they open fire on the canisters, as you duck for cover. Suddenly, you hear a chain of multiple explosions, and without being able to react, you are sucked up into outer space. They must have blown a breach in the station hull!]]))
   vn.scene()
   vn.func( function ()
      music.stop() -- TODO something better I guess?
      vn.textbox_bg_alpha = 0
      vn.show_options = false
      vn.setBackground( function ()
         local nw, nh = naev.gfx.dim()
         vn.setColor( {0, 0, 0, 1} )
         lg.rectangle("fill", 0, 0, nw, nh )
      end )
   end )
   vn.transition( "blinkout" )
   vn.na(_([[As everything gets cold, you fumble around your jumpsuit, maybe there is an emergency breather there.]]))
   vn.na(_([[So...]]))
   vn.na(_([[   cold...]]),true)
   vn.na(_([[...]]))
   vn.na(_([[... ...]]))
   vn.na(_([[...]]))
   vn.na(_([[.]]))
   vn.na(_([[ .]]),true)
   vn.na(_([[ .]]),true)

   vn.func( function ()
      -- Fades in shader from black
      local fadein_pixelcode = [[
#include "lib/blur.glsl"

const float INTENSITY = 10.0;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float disp = INTENSITY*(0.5-distance(0.5, u_progress));
   vec4 c1 = vec4(vec3(0.0),1.0);
   vec4 c2 = blur9( tex, texture_coords, love_ScreenSize.xy, disp );
   return mix( c1, c2, u_progress );
}
   ]]
      shader_fadeout = { shader=pp_shaders.newShader( fadein_pixelcode ) }
      shader_fadeout._dt = 0.0
      shader_fadeout._update = function( self, dt )
         self._dt = self._dt + dt * 1/3
         self.shader:send( "u_progress", math.min( 1, self._dt ) )
      end
      shader_fadeout.shader:addPPShader("final", 99)
   end )

   vn.run()

   local c = commodity.new(N_("Minerva Holodrives"), N_("A set of holodrives you were able to break out of Minerva Station. Hopefully they have something important in them."))
   c:illegalto( {"Empire"} )
   mem.carg_id = misn.cargoAdd( c, 0 )

   hook_update = hook.update("shader_update")
   player.takeoff()
end

function shader_update( dt )
   shader_fadeout:_update( dt )
   if shader_fadeout._dt >= 1 and mem.state < 2 then
      hook.rm( hook_update )
      shader_fadeout.shader:rmPPShader()

      vn.reset()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )

      vn.na(fmt.f(_([[You awake to a throbbing headache and a {shipai} hologram staring down at you.]]),
         {shipai=tut.ainame()}))
      sai(_([["Oh wow, it looks like humans can survive that long in a vacuum."]]))
      vn.menu{
         {_([["My head..."]]), "01_cont"},
         {_([["What... happened?"]]), "01_cont"},
         {_([["...is this hell?"]]), "01_hell"},
      }

      vn.label("01_hell")
      sai(fmt.f(_([["This is the {shipname}. Maybe you will need a health check after all."]]),
         {shipname=player.pilot():name()}))
      vn.jump("01_cont")

      vn.label("01_cont")
      sai(_([["There was a large explosion at Minerva Station, and since your vital signs were tanking, I took the liberty of flying the ship to check what happened. Now, how many fingers am I holding up?"]]))
      vn.menu{
         {_([["Vital signs?"]]), "02_signs"},
         {_([["Fingers?"]]), "02_fingers"},
      }

      vn.label("02_signs")
      local underwear_brand
      vn.func( function ()
         local brands = {
            [N_("MilSpec")] = 0,
            [N_("Unicorp")] = 0,
            [N_("TeraCom")] = 0,
            [N_("Enygma")]  = 0,
            [N_("S&K")]     = 0,
            [N_("Melendez")]= 0,
            [N_("Red Star")]= 0,
         }
         for k,v in ipairs(player.pilot():outfits()) do
            if v then
               for b,n in pairs(brands) do
                  if string.find(v:nameRaw(), b) then
                     brands[b] = n+1
                  end
               end
            end
         end
         local m = 0
         local s = N_("MilSpec")
         for b,n in pairs(brands) do
            if n > m then
               m = n
               s = b
            end
         end
         underwear_brand = s
      end )
      sai( function ()
         return fmt.f(_([["Your {brand} underwear has builtin reporting functionality. It's useful to tell if you are alive and healthy, or if you soiled yourself."]]),
            {brand=_(underwear_brand)}) end )
      vn.jump("02_cont")

      vn.label("02_fingers")
      sai(_([["Oh, I forgot. I do not have fingers anymore."]]))
      vn.jump("02_cont")

      vn.label("02_cont")
      vn.na(_([[You decide against asking further as your head continues throbbing.]]))
      sai(_([["You do not look too good. However, the health inspection will have to wait, we must get out of here!"]]))
      sai(fmt.f(_([["Maikki told us she would be waiting on {spb} in the {sys} system. We should head there."]]),
         {spb=returnspb, sys=returnsys}))
      vn.na(_([[With a heavy sigh and a horrible headache, you take the controls of you ship. Time to meet up with Maikki, however, you'll have to take care that you don't get scanned on the way there as the cargo you have is likely sought after by the Empire.]]))

      vn.run()

      -- Update objectives
      mem.state = 2
      misn.markerMove( mem.mrk, returnspb )
      misn.osdActive(3)
   end
end
