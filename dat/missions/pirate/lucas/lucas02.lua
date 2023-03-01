--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Lucas 02">
 <unique />
 <chance>60</chance>
 <location>Bar</location>
 <cond>
   -- Must be 5 cycles after finishing Lucas 01
   local t = var.peek("lucas01_done")
   if t and time.get() &lt; t + time.new( 0, 5, 0 ) then
      return false
   end

   -- Should be "nearish" to Shangris Station, but not the station
   local scur = spob.cur()
   local ss, sys = spob.getS("Shangris Station")
   if scur==ss or sys:jumpDist() &gt; 9 then
      return false
   end

   -- Not be restricted and be generic faction
   local t = scur:tags()
   if t.refugee or t.poor or  t.restricted or not scur:faction():tags().generic then
      return false
   end

   return true
 </cond>
 <done>Lucas 01</done>
 <notes>
  <tier>1</tier>
  <campaign>Lucas</campaign>
 </notes>
</mission>
--]]
--[[
   Lucas 02
--]]
local fmt = require "format"
local vn = require "vn"
local lcs = require "common.lucas"
local vni = require "vnimage"
local pir = require "common.pirate"
local comm = require "common.comm"

local title = _([[Leaving Society]])
local reward = 600e3

-- Mission stages
-- 0: just started
-- 1: talked to pirate
-- 2: looted / took from pirate and found jump (or already known)
mem.stage = 0

local first_spob, first_sys = spob.getS("Shangris Station")
local last_spob, last_sys = spob.getS("Qorellia")

function create ()
   misn.setNPC( lcs.lucas.name, lcs.lucas.portrait, _([[Lucas looks more worn out than before.]]) )
end

function discover_spob ()
   if last_spob:known() then
      misn.osdCreate(title, {
         fmt.f(_([[Take Lucas to {spob} ({sys} system})]]),
            {spob=last_spob, sys=last_sys}),
      })
      misn.markerRm() -- just in case remove all
      mem.mrk = misn.markerAdd( last_spob )
   end
end

local talked
function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local lucas = vn.newCharacter( lcs.vn_lucas() )
   vn.transition( lcs.lucas.transition )

   -- Change message first time you talk
   if talked then
      vn.na(_([[Will you help Lucas become a pirate?]]))
   else
      talked = true
      vn.na(_([[You approach Lucas who seems to be more on the edge than before.]]))
      lucas(_([[He seems to be mumbling to himself.
"I don't know why I trusted them…"]]))
      vn.menu{
         {_([["You OK?"]]), "cont01"},
         {_([[…]]), "cont01"},
      }

      vn.label("cont01")
      lucas(fmt.f(_([[It takes a while for them to notice your presence, but when they do, they give a half-hearted grin.
"Hey {playername}."]]),
         {playername=player.name()}))
      lucas(_([["Aw, screw it. I messed up. I put my faith in the system and got rammed over. They teach us like trash, useless garbage."]]))
      lucas(_([["I thought we was at fault, as if we had done something wrong and had to be punished. However, it was not we who did the wronging."]]))
      vn.menu{
         {p_("lucas02", [["What happened?"]]), "cont02"},
         {_([[…]]), "cont02"},
      }

      vn.label("cont02")
      lucas(_([["My family is dead. All of them. Kaput. Gone."]]))
      lucas(_([["My sister and mother succumbed at Maanen's Moon, and my father, my poor father, was refused any damn treatments in this so called civilized society."]]))
      lucas(_([["The bureaucrats refuse treating illegal nebula refugees. They even wanted to take him back to Maanen's bloody Moon! Heartless bastards."]]))
      lucas(_([["When he passed away in my arms, I lost all hope. I thought I had no option but to repent and follow my family. But then I realize, why are we at fault? Is it not the role of civilization to look for the betterment of mankind?"]]))
      lucas(_([["I decided to turn to anger, and want to take my revenge and carve my own path."]]))
      lucas(_([["This may surprise you, but I've decided to become a pirate."]]))

      vn.menu{
         {_([["A pirate!?"]]), "cont03_wut"},
         {_([["Sounds great!"]]), "cont03_arr"},
         {_([[…]]), "cont03_wut"},
      }

      vn.label("cont03_arr")
      lucas(_([["Wait, you are not surprised?"
The lean in and whisper to you.
"Don't tell me you are a… Wait no, it's probably better to not know now."]]))
      vn.jump("cont03")

      vn.label("cont03_wut")
      lucas(_([["Pirates are thought to be brutes unfit for society, however, that is just the lower rungs. The pirate clans are actually fully functional societies, and less arbitrary and oppressive than the Empire and the Houses."]]))
      vn.jump("cont03")

      vn.label("cont03")
      lucas(_([["Will you help me become a pirate?"]]))
   end
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Refuse]]), "refuse"},
   }

   vn.label("refuse")
   lucas(_([[They go back to their state of nervous anxiety and hope.]]))
   vn.done( lcs.lucas.transition )

   vn.label("accept")
   lucas(_([[They seem a bit taken aback.
"I'm surprised you accepted so easily."]]))
   lucas(fmt.f(_([["I've been looking around, and apparently there's a nearby clan known as the Raven Clan, which has many ex-Nebula refugees. I need you to take me to their base, which is rumoured to be called {spob}, and isn't easily accessible."]]),
      {spob=last_spob}))
   vn.func( function ()
      accepted = true
      if last_spob:known() then
         mem.spob_known = true
         vn.jump("pirate_known")
      end
   end )
   lucas(fmt.f(_([["I've been told some pirates like to hang around {spob} in the {sys} system. We should head over there to see if we can find some lead."]]),
      {spob=first_spob, sys=first_sys}))
   vn.na(_([[Lucas eagerly gets on your ship, ready to start a new life. You have a feeling it's not going to be that easy though.]]))
   vn.done( lcs.lucas.transition )

   vn.label("pirate_known")
   lucas(fmt.f(_([["Wait, you know it already? You've been there?!? This sure makes things much easier. Please take me to {spob}!"]]),
      {spob=last_spob}))
   vn.na(fmt.f(_([[Lucas eagerly gets on your ship and is ready to start their new life on {spob}. You have a feeling it's not going to be that easy though.]]),
      {spob=last_spob}))

   vn.done( lcs.lucas.transition )
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.setTitle( title )
   misn.setDesc(_([[You promised to help Lucas, the ex-Nebula refugee who lost his family, become a pirate.]]))
   misn.setReward(fmt.credits(reward))
   hook.land("land")

   local c = commodity.new( N_("Lucas"), N_("An individual who has given up on so-called 'civilized society' and wishes to become a pirate.") )
   mem.cargo = misn.cargoAdd( c, 0 )

   -- If known we skip most of the mission
   if last_spob:known() then
      mem.stage = 2
      discover_spob()
      return
   end

   misn.osdCreate( title, {
      fmt.f(_([[Try to find hints about {target} at {spob} ({sys} system)]]),
         {spob=first_spob, sys=first_sys, target=last_spob}),
   })
   mem.mrk = misn.markerAdd( first_spob )
end

local pir_portrait, pir_image
local pir_name = _("Shady Individual")
function land ()
   local spb = spob.cur()
   if mem.stage==0 and spb==first_spob then
      pir_image, pir_portrait = vni.pirate()
      mem.npc_pir = misn.npcAdd( "approach_pirate", pir_name, pir_portrait,
         fmt.f(_("You see a shady individual, maybe they know something about {spob}?"),
            {spob=last_spob}))

   elseif mem.stage==2 and spb==last_spob then
      vn.clear()
      vn.scene()
      local lucas = vn.newCharacter( lcs.vn_lucas() )
      vn.transition( lcs.lucas.transition )

      if not mem.spob_known then
         vn.na(fmt.f(_([[You land on {spob}, which strikes you as a surprisingly normal-looking world. If you didn't know that it was the center of the operations of the Raven Pirate Clan, you would have thought it was just a regular trade world.]]),
            {spob=last_spob}))
      end
      vn.na(_([[You get off your ship with Lucas, whose eyes are sparkling with hopes and dreams.]]))
      lucas(fmt.f(_([["{spob} at last! I can almost taste the freedom!"]]),
         {spob=last_spob}))
      lucas(_([["Thank you for all your help getting me here! Here, let me give you some credits for your troubles."]]))
      vn.sfxVictory()
      vn.na(fmt.reward(reward))
      vn.na(_([[Lucas seems euphoric to get away from the troubles of his past and begin anew. The question that remains unanswered is whether pirate society is truly more free than other societies, or is the problem at core human nature itself.]]))
      vn.na(_([[You wish him the best and he tells you to look for him next time you are near Raven space.]]))
      vn.done( lcs.lucas.transition )

      vn.run()

      misn.finish(true)
   end
end

function approach_pirate ()
   vn.clear()
   vn.scene()
   local p = vn.newCharacter( pir_name, {image=pir_image} )
   vn.transition()

   vn.na(fmt.f(_([[You approach the shady character, who doesn't seem very keen on your attention, and begin to make some idle conversation in hopes of drawing out information about {spob}.]]),
      {spob=last_spob}))
   p(_([[The character is less than cooperative, and seems more interested in getting rid of you than providing any help. It seems to be an exercise in futility.]]))
   p(_([[Eventually they start ignoring you and go away. Given the amount of people and security on the station, there is not really anything you can do.]]))
   vn.disappear(p)
   vn.na(_([[You look around, but it doesn't look like the people here are going to be the most helpful bunch, you'll have to find another way to get the information.]]))
   local lucas = lcs.vn_lucas()
   vn.appear( lucas, lcs.lucas.transition )
   vn.na(_([[You meet up with Lucas, who was also trying to get the information independently, albeit to the same result.]]))
   lucas(_([[He lets out a large sigh.
"It doesn't look like we're getting anywhere."]]))
   lucas(_([[His eyes suddenly light up.
"Maybe it's a long shot, but the best way to become a pirate is probably to pirate, is it not?"]]))
   lucas(_([["We should try to get on a pirate ship and parley as the pirates do. I don't think Marauder ships will know anything, so we should try to board a true pirate ship."]]))
   vn.na(_([[With the next plan of action decided, Lucas heads back to your ship with his head full of dreams of piracy.]]))

   vn.done( lcs.lucas.transition )
   vn.run()

   misn.markerRm( mem.mrk )
   mem.mrk = nil
   mem.stage = 1
   hook.board( "board_pirate" )
   hook.hail( "hail_pirate" )

   misn.npcRm( mem.npc_pir )

   misn.osdCreate( title, {
      fmt.f(_([[Obtain information about {spob} from boarding pirate ships]]),
         {spob=last_spob})
   })
end

local function valid_pirate( p )
   local m = p:memory()
   if not m.natural then
      return false
   end

   local pf = p:faction()
   if not pir.factionIsPirate( pf ) then
      return false
   end

   -- Not marauders
   if pf == faction.get("Marauder") then
      return false
   end

   return true
end

local function got_info ()
   jump.get( "Pas", "Effetey" ):setKnown(true)
   misn.osdCreate( title, {
      fmt.f(_([[Explore Qorel Tunnel and find your way to {spob}.]]),
         {spob=last_spob}),
   })
   mem.stage = 2
   discover_spob()
   hook.discover( "discover_spob" )
end

function hail_pirate( p )
   if not valid_pirate(p) then
      return
   end

   local m = p:memory()
   if m._lucas02_comm then
      return -- Already set up
   end
   comm.customComm( p, function ()
      if mem.stage>=2 then
         return nil -- Done already
      end
      return fmt.f(_("Ask about {spob}"),{spob=last_spob})
   end, function ( lvn, vnp )
      lvn.func( function ()
         if p:hostile() then
            lvn.jump("lucas02_hostile")
         end
      end )
      local payamount = 500e3
      vnp(fmt.f(_("You think us pirates will just tell you anything for money? Well, ye're damn right. I'll tell you for a measly {amount}. What do you say?"),
         {amount=fmt.credits(payamount)}))
      lvn.menu{
         {_([[Pay]]), "lucas02_pay"},
         {_([[Refuse]]), "menu"},
      }

      lvn.label("lucas02_broke")
      vnp(_([[#rYou do not enough enough credits.#0
"You think I'm going to accept less? Scram poor bum."]]))
      lvn.jump("menu")

      lvn.label("lucas02_pay")
      lvn.func( function ()
         if player.credits() < payamount then
            lvn.jump("lucas02_broke")
            return
         end
         player.pay(-payamount)
         mem.stage = 2
         var.push("lucas02_gotinfo",true)
      end )
      vnp(_([["Pleasure doing business with ya."]]))
      lvn.jump("menu")

      lvn.label("lucas02_hostile")
      vnp(_([["Like I'm going to tell you! You better plead for your life, punk!"]]))
   end )
   m._lucas02_comm = true

   -- Should run just after the comm stuff is done
   hook.timer(-1, "board_done")
end

function board_done ()
   if var.peek("lucas02_gotinfo") then
      got_info()
      var.pop("lucas02_gotinfo")
   end
end

function board_pirate( p )
   if not valid_pirate(p) then
      return
   end

   vn.clear()
   vn.scene()
   vn.transition()

   vn.sfxEerie()
   vn.na(fmt.f(_([[You board the ship with Lucas and through methods you are not particularly proud of, are able to obtain information of a secret jmup likely leading to the Qorel tunnel. You are told that {spob} is somewhere there.]]),
      {spob=last_spob}))

   vn.run()

   got_info()
end
