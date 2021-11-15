--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 5">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Land</location>
  <planet>Antlejos V</planet>
  <done>Terraforming Antlejos 4</done>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Defend ships bringing volunteers from the PUAAA
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"
local fleet = require "fleet"

local reward = ant.rewards.ant05

local mainpnt, mainsys = planet.getS("Antlejos V")
local nextsys = system.get("Delta Polaris")
local rearsys = system.get("Klintus")
local entrypoint = jump.get( mainsys, nextsys )
local rearpoint  = jump.get( mainsys, rearsys )

local supplylanded, supplydied
local sysmrk

function create ()
   if ant.datecheck() then misn.finish() end

   if not misn.claim(mainsys) then misn.finish() end

   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_("Verner is waiting for you as your ship tuoches gruond."))
   v(fmt.f(_([["It seems like the PUAAA have started to get brave and are going to make things hard for us. I have some supply ships coming in from {sys}, and it looks like they will be facing hostilities before they get here. Would you be willing to guard through {cursys}? I would be able to pay you {creds} for your troubles."]]),
      {sys=nextsys, cursys=mainsys, creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([["Great! The supply ships will be jumping in from {jumpsys}. Make sure to guard them and try to ensure they all make it safe and sound here. Best of luck, we are counting on you!"]]),
      {jumpsys=nextsys})
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Terraforming Antlejos") )
   misn.setDesc(fmt.f(_("Defend the supply ships coming to {pnt} from the PUAAA."),{pnt=mainpnt}))
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate(_("Terraforming Antlejos V"), {
      _("Guard the supply ships from the PUAAA"),
      fmt.f(_("Return to {pnt}"),{pnt=mainpnt}),
   })
   mem.mrk = misn.markerAdd( mainsys )
   sysmrk = system.mrkAdd( entrypoint:pos(), _("Supply Ship Entry Point") )
   mem.state = 1

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==2 and planet.cur() == mainpnt then
      local rewardmod = 1

      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      if supplydied <= 0 then
         v(_([["That was incredible! All the ships made it through in one piece. You're a real life saver!"]]))
      else if supplydied < 3 then
         v(_([["Almost all the ships made it! Against those odds I think we did very good."]]))
      else
         v(_([["I guess that was better than nothing. With so few supplies making it through it seems like terraforming will take more than I was hoping for."]]))
      end
      v(_([["With these supplies, I think we will be able to start the next important step on scaling up the terraforming operation. We're almost there. I have to organize some things, but meet me up back here in a bit."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[5] )

      -- Give supplies based on how many ships were able to land
      ant.supplied( supplylanded * 500 )

      player.pay( reward )
      ant.log(fmt.f(_("You guarded supply ships from the PUAAA delivering cargo to {pnt}."),{pnt=mainpnt}))
      ant.dateupdate()
      misn.finish(true)
   else
      player.msg(_("#rMISSION FAILED: You were not supposed to guard the supply ships!"))
      misn.finish(false)
   end
end

local puaaa, fsup
local protestors, supplyships
local function add_protestor( shipname, fromrear )
   -- They will just go to Antlejos V and attack any hostiles on their way, including the player.
   local ent = (fromrear and rearpoint) or entrypoint
   local p = pilot.add( shipname, puaaa, ent, _("Protestor"), {ai="guard"} )
   p:setHostile()
   local m = p:memory()
   m.guardpos = mainpnt:pos()
   m.guarddodist = math.huge
   table.insert( protestors, p )
end

local function add_supplyship( shipname )
   local ent = (fromrear and rearpoint) or entrypoint
   local p = pilot.add( shipname, fsup, ent, _("Supply Ship") )
   p:setFriendly()
   p:setVisplayer(true)
   p:setHilight(true)
   p:setInvincPlayer(true)
   p:control(true)
   p:land( mainpnt, true )
   hook.pilot( p, "land", "supplyland" )
   hook.pilot( p, "death", "supplydeath" )
   table.insert( supplyships, p )
end

function supplyland ()
   supplylanded = supplylanded + 1
end

function supplydeath ()
   supplydied = supplydied + 1
end

function enter ()
   if mem.state==1 and system.cur() ~= returnsys then
      player.msg(fmt.f(_("#rMISSION FAILED: You were not supposed to leave {sys}!"),{sys=mainsys}))
      misn.finish(false)
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   -- Initialize ship stuff
   protestors = {}
   supplyships = {}
   puaaa = ant.puaaa()
   fsup = faction.dynAdd( nil, "ant_supp", _("Supplier") )
   puaaa:dynEnemy( fsup )

   supplylanded = 0
   supplydied = 0

   hook.timer( 10, "supply1" )
   hook.timer( 25, "supply2" )
   hook.timer( 50, "supply3" )
   hook.timer( 80, "supply4" )
   hook.timer( 100, "supply5" )
   hook.timer( 100, "heartbeat")

   hook.timer( 15, "protestor1" )
   hook.timer( 30, "protestor2" )
   hook.timer( 60, "protestor3" )
   hook.timer( 100, "protestor4" )

   hook.timer( 25, "protest" )
end

function supply1 ()
   add_supplyship( "Koala" )
end
function supply2 ()
   add_supplyship( "Mule" )
end
function supply3 ()
   add_supplyship( "Koala" )
   add_supplyship( "Koala" )
end
function supply4 ()
   add_supplyship( "Rhino" )
end
function supply5 ()
   add_supplyship( "Koala" )
   add_supplyship( "Koala" )
   add_supplyship( "Koala" )
   player.msg("The last supply ships have entered the system!")
end

function protestor1 ()
   add_protestor( "Hyena" )
   add_protestor( "Hyena" )
end
function protestor2 ()
   add_protestor( "Shark" )
   add_protestor( "Hyena" )
end
function protestor3 ()
   add_protestor( "Shark" )
   add_protestor( "Hyena" )
   add_protestor( "Hyena", true )
   add_protestor( "Hyena", true )
end
function protestor4 ()
   add_protestor( "Lancelot" )
   add_protestor( "Shark" )
   add_protestor( "Shark" )
end

local protest_lines = ant.protest_lines
local protest_id, attacked
function protest ()
   if protest_id == nil then
      protest_id = rnd.rnd(1,#protest_lines)
   end

   -- See surviving pilots
   local nprotestors = {}
   for _k,p in ipairs(protestors) do
      if p:exists() then
         table.insert( nprotestors, p )
      end
   end
   protestors = nprotestors
   if #protestors > 0 then
      -- Say some protest slogan
      p:broadcast( protest_lines[ protest_id ], true )
      protest_id = math.fmod(protest_id, #protest_lines)+1
   end

   -- Protest again
   hook.timer( 15, "protest" )
end

function heartbeat ()
   local nsupplyships = {}
   for _k,p in ipairs(supplyships) do
      if p:exists() then
         table.insert( nsupplyships, p )
      end
   end
   if #nsupplyships > 0 then
      hook.timer( 3, "heartbeat" )
      return
   end
     
   if supplylanded <= 0 then
      player.msg(_("#rMISSION FAILED: No supply ships made it through!"))
      misn.finish(false)
   end

   -- Tell the player to land
   misn.osdActive(2)
   mem.state = 2
   mem.mrk = misn.markerMove( mem.mrk, mainpnt )
   system.mrkRm( sysmrk )
end
