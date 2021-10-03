--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 2">
 <flags>
  <unique />
 </flags>
 <avail>
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
 </avail>
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
local neu   = require "common.neutral"
local pir   = require "common.pirate"
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
   5: Flying back
--]]
misn_state = nil

misn_title = _("Helping Nelly Out")
reward_amount = 60e3

local function has_disable( o )
   local dmg, dis = o:weapStats()
   return dis > 0
end

function create ()
   if not var.peek("testing") then misn.finish() end
   -- Save current system to return to
   retpnt, retsys = planet.cur()
   if not misn.claim( retsys ) then
      misn.finish()
   end
   -- Need commodity exchange and mission computer
   local rs = retpnt:services()
   if rs.commodity == nil or rs.missions == nil then
      misn.finish()
   end

   -- Find destination system that sells ion cannons
   local pntfilter = function( p )
      -- Sells Outfits
      if p:services().outfits == nil then
         return false
      end
      return false
   end
   destpnt, destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 1, 1, "Independent", false, pntfilter )
   if not destpnt then
      warn("No destpnt found")
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see a Nelly motioning to you at the table.") )

   misn.setTitle( _("Helping Nelly Out… Again") )
   misn.setDesc( _("Help Nelly fix their ship.") )
   misn.setReward( fmt.credits(reward_amount) )
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )

   nel(_([[Nelly brightens up when you get near.
"Hey, I thought was able to get my ship up and running, but before I was able to get in and do a test run, the thing went haywire and now it's spinning around in circles. You have an Ion Cannon, do you? Could you help me disable my ship and get it back?"]]))
   vn.menu{
      {_("Help them with their ship"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[They look dejected.
"I guess I'll have to see if there is any other way to solve my problem."]]))
   vn.done()


   vn.label("accept")
   vn.func( function () doaccept = true end )
   nel(fmt.f(_([["Thanks for the help again. So while I was preparing to take off on my ship, I heard a weird noise outside, and when I went to check out, the autonav locked me out and my ship took off without anyone in it! Now it's flying around in circles outside of {pntname}!"]]),{pntname=retpnt:name()}))

   local pp = player.pilot()
   local hasoutfit = false
   local has_dis = false
   local has_dis_owned = false
   local owned = {}
   for k,o in ipairs(pp:outfits()) do
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
   local pnts = lmisn.getPlanetAtDistance( system.cur(), 0, 3, "Independent", false, pntfilter )
   table.sort( pnts, function( a, b )
      return a:system():jumpDist() < b:system():jumpDist()
   end )
   local nearplanet = pnts[1]

   if has_dis then
      nel(_([["It looks like you already have some disabling weapons equipped. Make sure they are set in the info window as either a primary or secondary weapon in your active weapon set or as an instant fire weapon set and let's go get my ship back!"]]))
   elseif has_dis_owned then
      nel(fmt.f(_([["It looks like you own some disabling weapons but don't have them equipped. Why don't you try to equip #o{outfitname} before we head out? We want to disable my ship, not destroy it!"]]),{outfitname=owned[rnd.rnd(1,#owned)]:name()}))
      local s = planet.cur():services()
      if not s.outfits and not s.shipyard then
         nel(fmt.f(_([["It looks like this planet doesn't have neither an #ooutfitter#0 nor a #oshipyard#0 so you won't be able to change your current equipment. Try to head off to a nearby planet with either an #ooutfitter#0 or a #oshipyard#0 such as #o{nearplanet}#0. You can check what services are available when you select the planet, or from the map."]]),{nearplanet=nearplanet:name()}))
      end
   else
      nel(fmt.f(_([["It looks like you don't have any disabling weapons. Remember, you have to disable my ship and not destroy it! I think the nearby #o{nearplanet}#0 should have #o{outfitname}#0 for sale. You should buy and equip one before trying to disable my ship!"]]),{nearplanet=nearplanet:name(), outfit_tobuy:name()}))
   end

   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   misn_state = 0
   misn_marker = misn.markerAdd( retsys )

   local osdtxt = {}
   if has_dis_owned then
      table.insert( osdtxt, _("Equip a weapon with disable damage") )
      misn_state = -1
   elseif not has_dis then
      table.insert( osdtxt, _("Buy and equip a weapon with disable damage") )
      misn_state = -2
   end
   table.insert( osdtxt, fmt.f(_("Disable and board Nelly's ship in {sysname}"), {sysname=destsys:name()}) )
   misn.osdCreate( misntitle, osdtxt )

   if misn_state < 0 then
      hk_equip = hook.equip( "equip" )
   end
   hook.enter("enter")
   hook.land("land")
end

function info_msg( msg )
   vntk.msg( tutnel.nelly.name, msg )
end

function hk_equip ()
   local pp = player.pilot()
   for k,o in ipairs(pp:outfits()) do
      if has_disable(o) then
         info_msg(fmt.f(_([["You have equipped a #o{outfitname}#0 with disable damage. Looks like you'll be able to safely disable my rampant ship!"]]),{outfitname=o:name()}))
         misn_state = 0
         misn.osdActive(2)
         hook.rm( hk_equip )
         hk_equip = nil
         return
      end
   end
end

function enter ()
   local scur = system.cur()
   if misn.state <= 0  and scur == retsys then
      rampant_pos = scur:pos() + rnd.rnd( 2000, rnd.rnd()*360 )
      rampant = pilot.add( "Llama", "Independent", rampant_pos )
      rampant:rename(_("Llaminator MK2"))
      rampant:intrinsicSet( "speed", -50 )
      rampant:intrinsicSet( "thrust", -50 )
      rampant:intrinsicSet( "turn", -50 )
      rampant:intrinsicSet( "shield_regen", -100 )
      rampant:intrinsicSet( "stress_dissipation", -100 )
      hook.pilot( rampant, "disable", "disable" )
      hook.pilot( rampant, "board", "board" )
      hook.pilot( rampant, "death", "death" )
      rampant:control(true)
      hook.pilot( rampant, "idle", "idle" )

   elseif misn_state == 2 and scur == retsys then
      jump_dest = jump.get( retsys, destsys )
      fpir = faction.dynAdd( "Pirate", "Pirate", _("Pirate"), {clear_enemies=true, clear_allies=true} )
      hook.timer( 1, "timer_pirate" )
      if not hk_reset_osd then
         hk_reset_osd = hook.enter( "reset_osd_hook" )
      end

   elseif misn_state == 4 and scur == destsys then
      jump_dest = jump.get( destsys, retsys )

      local s = player.pilot():stats().ew_stealth
      local m, a = jump_dest:pos():polar()
      local pos = vec2.newp( m - 1.5 * s, a )
      spotter = pilot.add( "Pacifier", "Mercenary", pos )
      spotter:intrinsicSet( "ew_hide", -50 )
      -- TODO ex stealth stuff

   end
end

function idle ()
   local radius = 200
   local samples = 10
   rampant_pos_idx = rampant_pos_idx or 0
   rampant_pos_idx = math.fmod( rampant_pos_idx, samples ) + 1
   local pos = rampant_pos + vec2.newP( radius, rnd.rnd() * samples / rampant_pos_idx * 360 )
   rampant:moveto( pos, false )
end

function disable ()
   player.pilot():comm(fmt.f(_([[Nelly: "You disabled it! Now get on top of the ship and board it with {boardkey}!"]]),{tut.getKey("board")}))
end

function board ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.na(_("You board Nelly's ship and quickly go to the control panel to turn off the autonav."))
   nel(fmt.f(_([["I hate when this happes. Err, I mean, this is the first time something like this has happened to me! Let me bring the ship back to {pntname} and meet me at the spaceport bar for your reward."]]),{pntname=retpnt:name()}))
   vn.run()

   -- Update objectives
   misn.osdCreate( misntitle, {
      fmt.f(_("Return to {pntname}"),{pntname=retpnt:name()})
   } )

   misn_state = 1

   -- Have the ship go back
   rampant:setHealth( 100 )
   rampant:setInvincible(true)
   rampant:intrinsicReset() -- Faster again
   rampant:control(true)
   rampant:land( retpnt )

   player.unboard()
end

function death ()
   vntk.msg(_([[#rMISSION FAILED: you weren't supposed to destroy Nelly's ship!#0
Although the mission has been aborted, you can still repeat it from the beginning by finding Nelly at a spaceport bar to try again.]]))
   misn.finish(false)
end

function land ()
   local cpnt = planet.cur()
   if cpnt == retpnt and misn_state==1 then
      npc_nel = misn.npcAdd( "approach_nelly", tutnel.nelly.nam, tut.nelly.portrait, _("Nelly is motioning you to come join her at the table.") )

   elseif cpnt == destpnt and (misn_state==2 or misn_state==3) then
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.na(_("You land and quickly Nelly goes over to the outfitter and seems to get into some sort of argument with the person in charge. After a bit you see they exchange something and she comes back with a grin on her face."))
      nel(fmt.f(_([["Got the parts! Cheaper than I expected to. Hopefully this will bring an end to my ship troubles. Let's go back to #o{pntname}#0 in #o{sysname}#0!"]]), {pntname=destpnt:name(), sysname=destsys:name()}))
      vn.run()

      local c = misn.cargoNew( N_("Jumpdrive Repair Parts"), N_("Spare parts that can be used to break a ship's broken jumpdrive.") )
      misn.cargoAdd( c, 0 )
      misn_marker = misn.markerMove( retsys )
      misn.osdActive(2)
      misn_state = 4

   elseif cpnt == retpnt and misn_state >= 4 then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      nel(_([["We finally made it. Nothing ever comes really easy does it? Ah, before I forget, let me reward you for your troubles."]]))
      vn.sfxVictory()
      vn.na(fmt.f(_("You have received #g{credits}#0."), {credits=fmt.credits(reward_amount)}))
      vn.func( function () -- Rewards
         player.pay( reward_amount )
      end )
      nel(_([["I'm going to get my ship fixed and hopefully next time we meet it'll be in space. Have fun!"
She runs off to where here ship is stored with the repair parts in hand.]]))
      vn.run()

      tutnel.log(_("You helped Nelly repair her ship."))

      misn.finish(true)
   end
end

function approach_nelly ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.na(_("As you approach her you can see she is a bit flustered."))
   nel(fmt.f(_([["I got my ship checked up again, and it looks like it wasn't just an autonav malfunction, but the jumpdrive is toast. I need a spare part, but they don't seem to have any here, and told me I'll have to wait at least a hectaperiod if I want it delivered. However, I was able to find that they seem to have replacements on #o{destpnt}#0 in #o{destsys}#0."]]),{destpnt=destpnt:name(), destsys=destsys:name()}))
   nel(fmt.f(_([[Before you can answer she grabs your arm and pulls you towards where your ship is docked.
"Come on, let's head to #o{destpnt}#0 to grab a spare part so I can finally get my ship off the ground."]]),{destpnt=destpnt:name()}))
   vn.na(_("Looks like you'll have to do another round trip if you want to get paid."))
   vn.run()

   misn.osdCreate( misntitle, {
      fmt.f(_("Go to {pntname} in {sysname}"),{pntname=destpnt:name(), sysname=destsys:name()}),
      fmt.f(_("Return to {pntname} in {sysname}"),{pntname=retpnt:name(), sysname=retsys:name()}),
   } )
   misn_marker = misn.markerMove( destsys )

   misn.npcRm( npc_nel )
   misn_state = 2
end

function timer_pirate ()
   local pp = player.pilot()

   if d < 5000 then
      -- Spawn pirates
      enemies = {}
      for i=1,3 do
         local p = pirate.add( "Hyena", fpir, jump_dest, _("Pirate Hyena") )
         if i>1 then
            p:setLeader( enemies[1] )
         end
         p:setHostile()
         p:setHilight()
         p:setVisplayer()
         p:control()
         p:attack( pp )
         p:intrinsicSet( "fwd_damage", -75 )
         local mem = p:memory()
         mem.allowbribe = true
         mem.bribe_base = 100
         table.insert( enemies, p )
      end
      pp:comm(fmt.f(_([[Nelly: "Wait, are those pirates coming our way?"]])))
      hook.timer( 5, "timer_pirate_nelly" )
      hook.timer( 3, "timer_pirate_checkbribe" )
      nelly_spam = 2
      return
   end
   hook.timer( 1, "timer_pirate" )
end

function timer_pirate_nelly ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   nel(_([["It looks like we've been spotted by a trio of Pirate Hyenas, normally I would say run, but I don't think we'll be able to outrun them. I think that bribing them may be the only way out."]]))
   nel(fmt.f(_([["If you target a pirate and hail them with {hailkey}, you should have an option to bribe them and their friends if they are hostile. Although it can be expensive, it beats getting blown to bits. Try targetting the nearest enemy with {targetkey}, hailing them with {hailkey}, and bribing them!"]]),{targetkey=tut.getKey("target_hostile"),hailkey=tut.getKey("hail")}))
   vn.run()

   local osdtitle, osdelem, osdactive = misn.osdGet()
   table.insert( osdelem, 1, _("Hail and bribe the pirates") )
   misn.osdCreate( osdtitle, osdelem )
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
         end
         n = n+1
      end
   end

   local pp = player.pilot()
   if allbribed then
      pp:comm(_([[Nelly: "Now we should be able to get out of here safely."]]))
      return
   end

   nelly_spam = math.fmod(nelly_spam,3)+1
   if nelly_spam == 1 then
      local msg
      if n <= 0 then
         msg = _([[Nelly: "I guess that's another way of doing it."]])
      elseif somebribed then
         msg = _([[Nelly: "You only bribed some pilots, try to bribe them all!"]])
      else
         msg = fmt.f(_([[Nelly: "Quickly! Target the hostile pirates with {targetkey} and bribe them by hailing them with {hailkey}!"]]),{targetkey=tut.getKey("target_hostile"),hailkey=tut.getKey("hail")})
      end
      pp:comm( msg )
   end

   hook.timer( 3, "timer_pirate_checkbribe" )
end

function reset_osd ()
   misn.osdCreate( misntitle, {
      fmt.f(_("Go to {pntname} in {sysname}"),{pntname=destpnt:name(), sysname=destsys:name()}),
      fmt.f(_("Return to {pntname} in {sysname}"),{pntname=retpnt:name(), sysname=retsys:name()}),
   } )
   if misn_state >= 4 then
      misn.osdActive(2)
   end
end

function reset_osd_hook ()
   reset_osd()
   hook.rm( hk_reset_osd )
   hk_reset_osd = nil
end
