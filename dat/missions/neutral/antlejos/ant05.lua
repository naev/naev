--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 5">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Land</location>
 <spob>Antlejos V</spob>
 <done>Terraforming Antlejos 4</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Defend ships bringing volunteers from the PUAAA
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"

local reward = ant.rewards.ant05

local mainpnt, mainsys = spob.getS("Antlejos V")
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
   vn.na(_("Verner is waiting for you as your ship touches ground."))
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
   v(fmt.f(_([["Great! The supply ships will be jumping in from {jumpsys}. Make sure to guard them and try to ensure they all make it safe and sound here. Best of luck, we are counting on you!"]]),
      {jumpsys=nextsys}))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Terraforming Antlejos") )
   misn.setDesc(fmt.f(_("Defend the supply ships coming to {pnt} from the PUAAA."),{pnt=mainpnt}))
   misn.setReward(reward)
   misn.osdCreate(_("Terraforming Antlejos V"), {
      fmt.f(_("Go to the jump point from {sys} and wait for the supply ships"),{sys=nextsys}),
      _("Guard the supply ships from the PUAAA"),
      fmt.f(_("Return to {pnt}"),{pnt=mainpnt}),
   })
   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==2 and spob.cur() == mainpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      if supplydied <= 0 then
         v(_([["That was incredible! All the ships made it through in one piece. You're a real lifesaver!"]]))
      elseif supplydied < 3 then
         v(_([["Almost all the ships made it! Against those odds I think we did very good."]]))
      else
         v(_([["I guess that was better than nothing. With so few supplies making it through it seems like terraforming will take longer than I was hoping for."]]))
      end
      v(_([["With these supplies, I think we will be able to start the next important step on scaling up the terraforming operation. We're almost there, but we'll still need more assistance. The mission terminal should be up and working, and you should be able to find new supply requests there if you want to help. We still need all the hands we can get!"]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[5] )

      -- Give supplies based on how many ships were able to land
      ant.supplied( supplylanded * 125 ) -- Should get just 1000 if all make it

      player.pay( reward )
      ant.log(fmt.f(_("You guarded supply ships from the PUAAA delivering cargo to {pnt}."),{pnt=mainpnt}))
      ant.dateupdate()
      misn.finish(true)
   else
      lmisn.fail(_("You were supposed to guard the supply ships!"))
   end
end

local puaaa, fsup
local protestors, supplyships
local function add_protestor( shipname, fromrear )
   -- They will just go to Antlejos V and attack any hostiles on their way, including the player.
   local ent = (fromrear and rearpoint) or entrypoint
   local p = pilot.add( shipname, puaaa, ent, _("Protestor"), {ai="baddiepos"} )
   p:setHostile()
   local m = p:memory()
   m.guardpos = mainpnt:pos()
   table.insert( protestors, p )
end

local function add_supplyship( shipname )
   local ent = entrypoint
   local p = pilot.add( shipname, fsup, ent, _("Supply Ship"), {ai="independent"} )
   p:setFriendly()
   p:setVisplayer(true)
   p:setHilight(true)
   p:setInvincPlayer(true)
   p:control(true)
   p:land( mainpnt )
   hook.pilot( p, "land", "supplyland" )
   hook.pilot( p, "death", "supplydeath" )
   hook.pilot( p, "attacked", "supplyattacked" )
   table.insert( supplyships, p )
end

function supplyland ()
   supplylanded = supplylanded + 1
end

function supplydeath ()
   supplydied = supplydied + 1
end

local last_spammed
function supplyattacked( p )
   last_spammed = last_spammed or 0
   local t = naev.ticks()
   if (t-last_spammed) > 10 then
      p:broadcast(_("Supply ship under attack! Help requested!"))
      last_spammed = t
   end
end

function enter ()
   if mem.state==2 then
      -- Nothing to do here
      return
   end
   -- Wrong system
   if system.cur() ~= mainsys then
      lmisn.fail(fmt.f(_("You were not supposed to leave {sys}!"),{sys=mainsys}))
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   sysmrk = system.markerAdd( entrypoint:pos(), _("Supply Ship Entry Point") )
   hook.timer( 3, "approaching" )
end

function approaching ()
   if player.pos():dist( entrypoint:pos() ) > 3000 then
      hook.timer( 3, "approaching" )
      return
   end

   mem.state = 1
   misn.osdActive(2)
   player.msg(_("The supply ships will be incoming shortly!"))

   -- Initialize ship stuff
   protestors = {}
   supplyships = {}
   puaaa = ant.puaaa()
   fsup = faction.dynAdd( nil, "ant_supp", _("Supplier") )
   puaaa:dynEnemy( fsup )

   supplylanded = 0
   supplydied = 0

   hook.timer( 10, "supply1" )
   hook.timer( 40, "supply2" )
   hook.timer( 90, "supply3" )
   hook.timer( 150, "supply4" )
   hook.timer( 170, "supply5" )
   hook.timer( 170, "heartbeat")

   hook.timer( 20, "protestor1" )
   hook.timer( 60, "protestor2" )
   hook.timer( 120, "protestor3" )
   hook.timer( 190, "protestor4" )

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
   player.msg(_("The last supply ships have entered the system!"))
   system.markerRm( sysmrk )
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
end
function protestor4 ()
   add_protestor( "Lancelot" )
   add_protestor( "Shark" )
   add_protestor( "Shark" )
end

local protest_lines = ant.protest_lines
local protest_id
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
      local p = protestors[ rnd.rnd(1,#protestors) ]
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
      lmisn.fail(_("No supply ships made it through!"))
   end

   -- Tell the player to land
   misn.osdActive(3)
   mem.state = 2
   misn.markerMove( mem.mrk, mainpnt )
end
