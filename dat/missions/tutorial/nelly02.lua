--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 2">
 <unique />
 <priority>1</priority>
 <chance>100</chance>
 <cond>require("common.pirate").systemPresence() &lt;= 0</cond>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <done>Helping Nelly Out 1</done>
  <chapter>[01]</chapter>
 <notes>
  <campaign>Tutorial Nelly</campaign>
 </notes>
</mission>
--]]
--[[
   Nelly Tutorial Campaign

   Second mission is designed to teach about:
   1. Disabling
   2. Comm and bribing
   3. Safe lanes
   4. Stealth

   Mission Details:
   0. Have to disable out of control ship.
   1. Land back on planet.
   2. Have to go pick up repair part at nearby system.
   3. Attacked by pirate, taught how to bribe.
   4. Reach planet, get part.
   5. Flying back, avoided ex at jump point.
--]]
local tutnel= require "common.tut_nelly"
local tut   = require "common.tutorial"
local vn    = require 'vn'
local vntk  = require 'vntk'
local fmt   = require "format"
local lmisn = require "lmisn"

--[[
   Mission States:
  -2: Don't own a disabling weapon
  -1: Own but don't have equipped a disabling weapon
   0: Accepted and going to disable ship
   1: Disabled ship and returning to planet
   2: Going to pick up repair pirate
   3. Dealt with pirate
   4: Got part
   5: Past nosy pacifier
   6: Flying back
--]]
mem.misn_state = nil
local enemies, rampant, rampant_pos, rampant_pos_idx, spotter, spotter_pos -- Non-persistent state

local reward_amount = tutnel.reward.nelly02

local function has_disable( o )
   local dmg, dis = o:weapstats()
   dmg = dmg or 0
   dis = dis or 0
   return dis > dmg
end

function create ()
   -- Save current system to return to
   mem.retpnt, mem.retsys = spob.cur()
   -- Need commodity exchange and mission computer
   local rs = mem.retpnt:services()
   if rs.commodity == nil or rs.missions == nil then
      misn.finish()
   end

   -- Find destination system that sells ion cannons
   local pntfilter = function( p )
      -- Must be claimable
      local s = p:system()
      if not naev.claimTest( s ) then
         return false
      end
      -- Sells Outfits
      if not p:services().outfits then
         return false
      end
      return true
   end
   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 1, 1, "Independent", false, pntfilter )
   if not mem.destpnt then
      misn.finish()
   end
   if not misn.claim{ mem.retsys, mem.destsys, "nelly" } then
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see Nelly motioning to you at a table.") )

   misn.setTitle( _("Helping Nelly Out… Again") )
   misn.setDesc( _("Help Nelly fix their ship.") )
   misn.setReward(reward_amount)
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   nel(_([[Nelly brightens up when you get near.
"Hey, I thought was able to get my ship up and running, but before I was able to get in and do a test run, the thing went haywire and now it's spinning around in circles! You have an Ion Cannon, do you? Could you help me disable my ship and get it back?"]]))
   vn.menu{
      {_("Help them with their ship"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[They look dejected.
"I guess I'll have to see if there is any other way to solve my problem."]]))
   vn.done( tutnel.nelly.transition )

   vn.label("accept")
   vn.func( function () doaccept = true end )
   nel(fmt.f(_([["Thanks for the help again. So while I was preparing to take off on my ship, I heard a weird noise outside, and when I went to check out, the autonav locked me out and my ship took off without anyone in it! Now it's flying around in circles outside of {pnt}!"]]),{pnt=mem.retpnt}))

   local pp = player.pilot()
   local has_dis = false
   local has_dis_owned = false
   local owned = {}
   for k,o in ipairs(pp:outfitsList()) do
      has_dis = has_dis or has_disable(o)
   end
   for k,o in ipairs(player.outfits()) do
      local hd = has_disable(o)
      has_dis_owned = has_dis_owned or hd
      if hd then
         table.insert( owned, o )
      end
   end
   local outfit_tobuy = outfit.get("Ion Cannon")
   local function pntfilter( p )
      -- Sells Outfits
      if p:services().outfits == nil then
         return false
      end
      -- Sells a particular outfit
      for k,o in ipairs(p:outfitsSold()) do
         if o == outfit_tobuy then
            return true
         end
      end
      return false
   end
   local pnts = lmisn.getSpobAtDistance( system.cur(), 0, 3, "Independent", false, pntfilter )
   table.sort( pnts, function( a, b )
      return a:system():jumpDist() < b:system():jumpDist()
   end )
   local nearplanet = pnts[1]

   if has_dis then
      nel(_([["It looks like you already have some disabling weapons equipped. Make sure they are set in the info window as either a primary or secondary weapon in your active weapon set or as an instant fire weapon set and let's go get my ship back!"]]))
   elseif has_dis_owned then
      nel(fmt.f(_([["It looks like you own some disabling weapons but don't have them equipped. Why don't you try to equip #o{outfitname}#0 before we head out? We want to disable my ship, not destroy it!"]]),{outfitname=owned[rnd.rnd(1,#owned)]}))
      local s = spob.cur():services()
      if not s.outfits and not s.shipyard then
         nel(fmt.f(_([["It looks like this planet doesn't have neither an #ooutfitter#0 nor a #oshipyard#0 so you won't be able to change your current equipment. Try to head off to a nearby planet with either an #ooutfitter#0 or a #oshipyard#0 such as #o{nearplanet}#0. You can check what services are available when you select the planet, or from the map."]]),{nearplanet=nearplanet}))
      end
   else
      nel(fmt.f(_([["It looks like you don't have any disabling weapons. Remember, you have to disable my ship and not destroy it! I think the nearby #o{nearplanet}#0 should have #o{outfitname}#0 for sale. You should buy and equip one before trying to disable my ship!"]]),{nearplanet=nearplanet, outfitname=outfit_tobuy}))
   end

   vn.done( tutnel.nelly.transition )
   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   mem.misn_state = 0
   mem.misn_marker = misn.markerAdd( mem.retsys )

   local osdtxt = {}
   if has_dis_owned then
      table.insert( osdtxt, _("Equip a weapon with disable damage") )
      mem.misn_state = -1
   elseif not has_dis then
      table.insert( osdtxt, _("Buy and equip a weapon with disable damage") )
      mem.misn_state = -2
   end
   table.insert( osdtxt, fmt.f(_("Disable and board Nelly's ship in {sys}"), {sys=mem.retsys}) )
   misn.osdCreate( _("Helping Nelly Out"), osdtxt )

   if mem.misn_state < 0 then
      mem.hk_equip = hook.equip( "equip" )
   end
   hook.enter("enter")
   hook.land("land")
end

local function info_msg( msg )
   vntk.msg( tutnel.nelly.name, msg )
end

function equip ()
   local pp = player.pilot()
   for k,o in ipairs(pp:outfitsList()) do
      if has_disable(o) then
         info_msg(fmt.f(_([["You have equipped a #o{outfitname}#0 with disable damage. Looks like you'll be able to safely disable my rampant ship!"]]),{outfitname=o}))
         mem.misn_state = 0
         misn.osdActive(2)
         hook.rm( mem.hk_equip )
         mem.hk_equip = nil
         return
      end
   end
end

local function reset_osd ()
   misn.osdCreate( _("Helping Nelly Out"), {
      fmt.f(_("Go to {pnt} in {sys}"),{pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Return to {pnt} in {sys}"),{pnt=mem.retpnt, sys=mem.retsys}),
   } )
   if mem.misn_state >= 4 then
      misn.osdActive(2)
   end
end

function enter ()
   local scur = system.cur()
   if mem.misn_state <= 0  and scur == mem.retsys then
      rampant_pos = mem.retpnt:pos() + vec2.newP( 2000, rnd.angle() )
      rampant = pilot.add( "Llama", "Dummy", rampant_pos, _("Llaminator MK2") )
      rampant:intrinsicSet( "speed", -50 )
      rampant:intrinsicSet( "thrust", -50 )
      rampant:intrinsicSet( "turn", -50 )
      rampant:intrinsicSet( "shield_regen_mod", -90 )
      rampant:intrinsicSet( "stress_dissipation", -90 )
      rampant:setVisplayer()
      rampant:setHilight()
      local aimem = rampant:memory()
      aimem.comm_no = _("No response.")
      aimem.distress = false
      hook.pilot( rampant, "disable", "disable" )
      hook.pilot( rampant, "board", "board" )
      hook.pilot( rampant, "death", "death" )
      rampant:control(true)
      --hook.pilot( rampant, "idle", "idle" )
      idle( rampant )

   elseif mem.misn_state == 2 and scur == mem.retsys then
      mem.jump_dest = jump.get( mem.retsys, mem.destsys )
      local fpir = faction.dynAdd( "Pirate", "nelly_pirate", _("Pirate"), {clear_enemies=true, clear_allies=true} )
      hook.timer( 1, "timer_pirate", fpir )
      if not mem.hk_reset_osd then
         mem.hk_reset_osd = hook.enter( "reset_osd_hook" )
      end

   elseif mem.misn_state == 4 and scur == mem.destsys then
      mem.jump_dest = jump.get( mem.destsys, mem.retsys )

      local s = player.pilot():stats().ew_stealth
      local m, a = mem.jump_dest:pos():polar()
      spotter_pos = vec2.newP( m - 1.5 * s, a )
      spotter = pilot.add( "Pacifier", "Mercenary", spotter_pos, _("Noisy Pacifier") )
      spotter:setVisplayer()
      spotter:setHilight()
      spotter:setInvincible()
      spotter:control()
      spotter:brake()

      mem.hk_timer_spotter = hook.timer( 9, "timer_spotter" )
      hook.timer( 15, "timer_spotter_start" )
      mem.misn_state = 5
   elseif mem.misn_state==5 then
      reset_osd()
   end
end

function idle ()
   if mem.misn_state > 0 then return end
   if not rampant or not rampant:exists() then return end
   local radius = 200
   local samples = 18
   rampant_pos_idx = rampant_pos_idx or 0
   rampant_pos_idx = math.fmod( rampant_pos_idx, samples ) + 1
   local pos = rampant_pos + vec2.newP( radius, 2*math.pi * rampant_pos_idx / samples )
   rampant:taskClear()
   rampant:moveto( pos, false, false )

   hook.timer( 1, "idle" )
end

function disable ()
   player.msg(fmt.f(_([[Nelly: "You disabled it! Now get on top of the ship and board it with {boardkey}!"]]),{boardkey=tut.getKey("board")}),true)
end

function board ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   vn.na(_("You board Nelly's ship and quickly go to the control panel to turn off the autonav."))
   nel(fmt.f(_([["I hate when this happens. Err, I mean, this is the first time something like this has happened to me! Let me bring the ship back to {pnt} and meet me at the spaceport bar for your reward."]]),{pnt=mem.retpnt}))
   vn.done( tutnel.nelly.transition )
   vn.run()

   -- Update objectives
   misn.osdCreate( _("Helping Nelly Out"), {
      fmt.f(_("Return to {pnt}"),{pnt=mem.retpnt})
   } )

   mem.misn_state = 1

   -- Have the ship go back
   local a, s = rampant:health()
   rampant:setHealth( a, s )
   rampant:setInvincible(true)
   rampant:intrinsicReset() -- Faster again
   rampant:setFriendly(true)
   rampant:control(true)
   rampant:taskClear()
   rampant:land( mem.retpnt )

   player.unboard()
end

function death ()
   vntk.msg(_([[#rMISSION FAILED: you weren't supposed to destroy Nelly's ship!#0
Although the mission has been aborted, you can still repeat it from the beginning by finding Nelly at a spaceport bar to try again.]]))
   misn.finish(false)
end

function land ()
   local cpnt = spob.cur()
   if cpnt == mem.retpnt and mem.misn_state==1 then
      mem.npc_nel = misn.npcAdd( "approach_nelly", tutnel.nelly.name, tutnel.nelly.portrait, _("Nelly is motioning you to come join her at the table.") )

   elseif cpnt == mem.destpnt and (mem.misn_state==2 or mem.misn_state==3) then
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      vn.na(_("You land and quickly Nelly goes over to the outfitter and seems to get into some sort of argument with the person in charge. After a bit you see they exchange something and she comes back with a grin on her face."))
      nel(fmt.f(_([["Got the parts! Cheaper than I expected too. Hopefully this will bring an end to my ship troubles. Let's go back to #o{pnt}#0 in #o{sys}#0!"]]), {pnt=mem.retpnt, sys=mem.retsys}))
      vn.done( tutnel.nelly.transition )
      vn.run()

      local c = commodity.new( N_("Jumpdrive Repair Parts"), N_("Spare parts that can be used to break a ship's broken jumpdrive.") )
      misn.cargoAdd( c, 0 )
      misn.markerMove( mem.misn_marker, mem.retpnt )
      misn.osdActive(2)
      mem.misn_state = 4

   elseif cpnt == mem.retpnt and mem.misn_state >= 4 then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      nel(_([["We finally made it. Nothing ever comes really easy does it? Ah, before I forget, let me reward you for your troubles."]]))
      vn.sfxVictory()
      vn.na(fmt.reward(reward_amount))
      vn.func( function () -- Rewards
         player.pay( reward_amount )
      end )
      nel(_([["I'm going to get my ship fixed and hopefully next time we meet it'll be in space. Have fun!"
She runs off to where here ship is stored with the repair parts in hand.]]))
      vn.done( tutnel.nelly.transition )
      vn.run()

      tutnel.log(_("You helped Nelly repair her ship."))

      misn.finish(true)
   end
end

function approach_nelly ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   vn.na(_("As you approach her you can see she is a bit flustered."))
   nel(fmt.f(_([["I got my ship checked up again, and it looks like it wasn't just an autonav malfunction, but the jumpdrive is toast. I need a spare part, but they don't seem to have any here, and told me I'll have to wait at least a hectaperiod if I want it delivered. However, I was able to find that they seem to have replacements on #o{destpnt}#0 in #o{destsys}#0."]]),{destpnt=mem.destpnt, destsys=mem.destsys}))
   nel(fmt.f(_([[Before you can answer she grabs your arm and pulls you towards where your ship is docked.
"Come on, let's head to #o{destpnt}#0 to grab a spare part so I can finally get my ship off the ground."]]),{destpnt=mem.destpnt}))
   vn.na(_("Looks like you'll have to do another round trip if you want to get paid."))
   vn.done( tutnel.nelly.transition )
   vn.run()

   misn.osdCreate( _("Helping Nelly Out"), {
      fmt.f(_("Go to {pnt} in {sys}"),{pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Return to {pnt} in {sys}"),{pnt=mem.retpnt, sys=mem.retsys}),
   } )
   misn.markerMove( mem.misn_marker, mem.destpnt )

   misn.npcRm( mem.npc_nel )
   mem.misn_state = 2
end

function timer_pirate( fpir )
   local pp = player.pilot()
   local d = mem.jump_dest:pos():dist( pp:pos() )
   if d < 5000 then
      -- Spawn pirates
      enemies = {}
      for i=1,3 do
         local p = pilot.add( "Pirate Hyena", fpir, mem.jump_dest )
         if i>1 then
            p:setLeader( enemies[1] )
         end
         p:setHostile()
         p:setHilight()
         p:setVisplayer()
         p:intrinsicSet( "fwd_damage", -75 )
         local aimem = p:memory()
         aimem.allowbribe = true
         aimem.bribe_base = 100
         aimem.vulnignore = true -- Ignore vulnerability
         table.insert( enemies, p )
      end
      player.msg(fmt.f(_([[Nelly: "Wait, are those pirates coming our way?"]])),true)
      player.autonavReset(5)
      hook.timer( 3, "timer_pirate_nelly" )
      hook.timer( 3, "timer_pirate_checkbribe" )
      mem.nelly_spam = 2
      return
   end
   hook.timer( 1, "timer_pirate", fpir )
end

function timer_pirate_nelly ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   nel(_([["It looks like we've been spotted by a trio of Pirate Hyenas. Normally I would say run, but I don't think we'll be able to outrun them. I think that bribing them may be the only way out."]]))
   nel(fmt.f(_([["If you target a pirate and hail them with {hailkey}, you should have an option to bribe them and their friends if they are hostile. Although it can be expensive, it beats getting blown to bits. Try targetting the nearest enemy with {targetkey}, hailing them with {hailkey}, and bribing them!"]]),{targetkey=tut.getKey("target_hostile"),hailkey=tut.getKey("hail")}))
   if player.credits() < 1000 then
      nel(fmt.f(_([["It looks like you won't have enough money to bribe them. Here, take #g{credits}#0, that should be enough hopefully."]]),{credits=fmt.credits(1e3)}))
      vn.func( function() player.pay(1000) end )
   end
   vn.done( tutnel.nelly.transition )
   vn.run()

   local osdtitle, osdelem = misn.osdGet()
   table.insert( osdelem, 1, _("Hail and bribe the pirates") )
   misn.osdCreate( osdtitle, osdelem )

   player.setSpeed( 2/3 )
   hook.timer( 15, "reset_speed" )
end

function reset_speed ()
   player.setSpeed()
end

function timer_pirate_checkbribe ()
   local allbribed = true
   local somebribed = false
   local n = 0
   for k,p in ipairs(enemies) do
      if p:exists() then
         if not p:flags("bribed") then
            allbribed = false
         else
            somebribed = true
            p:setHilight(false)
         end
         n = n+1
      end
   end

   if allbribed then
      player.msg(_([[Nelly: "Now we should be able to get out of here safely."]]),true)
      player.setSpeed()
      return
   end

   mem.nelly_spam = math.fmod(mem.nelly_spam,3)+1
   if mem.nelly_spam == 1 then
      local msg
      if n <= 0 then
         msg = _([[Nelly: "I guess that's another way of doing it."]])
         player.setSpeed()
      elseif somebribed then
         msg = _([[Nelly: "You only bribed some pilots, try to bribe them all!"]])
      else
         msg = fmt.f(_([[Nelly: "Quickly! Target the hostile pirates with {targetkey} and bribe them by hailing them with {hailkey}!"]]),
            {targetkey=tut.getKey("target_hostile"),hailkey=tut.getKey("hail")})
      end
      player.msg( msg, true )
   end

   if n > 0 then
      hook.timer( 3, "timer_pirate_checkbribe" )
   end
end

function reset_osd_hook ()
   reset_osd()
   hook.rm( mem.hk_reset_osd )
   mem.hk_reset_osd = nil
end

local spotter_msglist = {
   _([[I know you're out there Nelly. Please come forward voluntarily.]]),
   _([[Stop hiding Nelly, you know this is futile.]]),
   _([[Let us face this like adults, Nelly.]]),
   _([[Nelly, if you come out now I would hold this against you.]]),
   _([[You're seriously going to make me have to scan the entire universe to find you Nelly?]]),
   _([[Just come on out and let's get this over with, Nelly.]]),
}
function timer_spotter ()
   if not spotter:exists() then return end

   mem.spotter_msg = mem.spotter_msg or 0
   mem.spotter_msg = math.fmod( mem.spotter_msg, #spotter_msglist )+1

   spotter:broadcast( spotter_msglist[ mem.spotter_msg ], true )

   mem.hk_timer_spotter = hook.timer( 15, "timer_spotter" )
end

function timer_spotter_start ()
   if not spotter:exists() then return end

   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   nel(_([[Nelly seems to get a bit nervous when seeing the pacifier appear on-screen.
"Wait, that looks like Robin. Shit, I never paid her back did I?"
She frowns.
"Looks like we're going to have to avoid her for now. You know how electronic warfare works?"]]))
   vn.menu{
      {_("Learn about Electronic Warfare"), "learn"},
      {_("Already know"), "nolearn" },
   }

   vn.label("learn")
   nel(_([["I'll try to be brief. So ship sensors are based on detecting gravitational anomalies, and thus the mass of a ship plays a critical role in being detected. Smaller ships like yachts or interceptors are much harder to detect than carriers or battleship."]]))
   nel(p_("Nelly", [["Each ship has three important electronic warfare statistics:
- #oDetection#0 determines the distance at which a ship appears on the radar.
- #oEvasion#0 determines the distance at which a ship is fully detected, that is, ship type and faction are visible. It also plays a role in how missiles and weapons track the ship.
- #oStealth#0 determines the distance at which the ship is undetected when in stealth mode"]]))
   nel(fmt.f(_([["You can activate stealth mode with {stealthkey} when far enough away from other ships. When stealthed, your ship will be completely invisible to all ships. However, if a ship gets within the #ostealth#0 distance of your ship, it will slowly uncover you."]]),{stealthkey=tut.getKey("stealth")}))
   nel(_([["Besides making your ship invisible to other ships, #ostealth#0 slows down your ship heavily to mask your gravitational presence. This also has the effect of letting you jump out from jumpoints further away."]]))
   nel(_([["When not in stealth, ships can target your ship to perform a scan. This can uncover unwanted information, such as illegal cargo or outfits. The time to scan depends on the mass of the ship. If you don't want to be scanned, I recommend you to rely on stealth as much as possible."]]))
   nel(fmt.f(_([["To avoid getting spotted by {plt}, you should first get away from nearby ships and stealth with {stealthkey}. Then avoid other ships using the overlay map you can open with {overlaykey}, where the detection radius will be shown in red circles. You should then be able to fly around {plt} and get to the jump point. It shouldn't be hard, but be careful not to get close to them!"]]),{stealthkey=tut.getKey("stealth"),overlaykey=tut.getKey("overlay"),plt=spotter}))
   vn.done( tutnel.nelly.transition )

   vn.label("nolearn")
   nel(_([["You sure you know how to avoid them? It can be a bit tricky."]]))
   vn.menu{
      {_("Learn about Electronic Warfare"), "learn"},
      {_([["Call me Dr. Electronic Warfare"]]), "neverlearn"},
   }

   vn.label("neverlearn")
   nel(fmt.f(_([["Great! Avoid getting scanned by them and let's head off to {pnt} in {sys}!"]]),{pnt=mem.destpnt,sys=mem.destsys}))

   vn.done( tutnel.nelly.transition )
   vn.run()

   local osdtitle, osdelem = misn.osdGet()
   table.insert( osdelem, 2, fmt.f(_("Avoid Nelly's ex-colleague by using stealth with {stealthkey}"), {stealthkey=tut.getKey("stealth")}) )
   misn.osdCreate( osdtitle, osdelem )
   misn.osdActive(2)

   hook.timer( 1, "spotter_spot" )
end

function spotter_spot ()
   if not spotter:exists() then return end

   local pp    = player.pilot()
   local _detected, scanned = spotter:inrange( pp )
   local iss   = pp:flags("stealth")
   -- Spotter lost track of them
   if mem.spotter_scanning and (iss or not scanned) then
      mem.spotter_scanning = false
      spotter:taskClear()
      spotter:moveto( spotter_pos )
      player.msg(_([[Nelly: "Phew, it seems like they lost track of us."]]),true)

   elseif mem.spotter_scanning and spotter:scandone() then
      spotter:control(false)
      spotter:setHostile(true)
      spotter:comm(_("You won't get away this time Nelly!"))
      hook.rm( mem.hk_timer_spotter )
      mem.hk_timer_spotter = nil
      return

   elseif scanned and not mem.spotter_scanning then
      mem.spotter_scanning = true
      spotter:taskClear()
      spotter:pushtask( "scan", pp )
      player.msg(fmt.f(_([[Nelly: "They found us and are scanning us. Quickly try to stealth with {stealthkey}!"]]),{stealthkey=tut.getKey("stealth")}),true)
      player.autonavReset( 5 )

   end

   hook.timer( 1, "spotter_spot" )
end
