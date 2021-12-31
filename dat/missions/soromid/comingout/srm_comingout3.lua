--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="A Friend's Aid">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <done>Coming of Age</done>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Soromid</faction>
  <cond>var.peek("comingout_time") == nil or time.get() &gt;= time.fromnumber(var.peek("comingout_time")) + time.create(0, 20, 0)</cond>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   A Friend's Aid

--]]
local lmisn = require "lmisn"
local fmt = require "format"
local car = require "common.cargo"
local srm = require "common.soromid"

-- luacheck: globals chelsea chelsea_attacked chelsea_death chelsea_jump chelsea_land fass fthug jumpin jumpNext jumpout land spawnChelseaShip spawnThug takeoff thug_removed thug_timer (shared with derived mission srm_comingout5)
-- luacheck: globals chelsea_distress_timer (Hook functions passed by name)

mem.misn_title = _("A Friend's Aid")
mem.misn_desc = _("Chelsea needs you to escort her to {pnt}.")

function create ()
   mem.misplanet, mem.missys, mem.njumps, mem.tdist, mem.cargo, mem.avgrisk = car.calculateRoute( 3 )
   if mem.misplanet == nil or mem.missys == nil or mem.avgrisk > 0 then
      misn.finish( false )
   end

   mem.credits = 500e3
   mem.started = false

   misn.setNPC( _("Chelsea"), "soromid/unique/chelsea.webp", _("You see Chelsea looking contemplative.") )
end


function accept ()
   local txt
   if mem.started then
      txt = _([["I could still use your help with that mission! Could you help me out?"]])
   else
      txt = fmt.f( _([[Chelsea gleefully waves you over. "It's nice to see you again!" she says. The two of you chat a bit about her venture into the piloting business; all is well from the sound of it. "Say," she says, "someone offered me a really interesting mission recently, but I had to decline because my ship isn't really up to task. If you could just escort my ship through that mission I could share the pay with you! Your half would be {credits}. How about it?"]]), {credits=fmt.credits(mem.credits)} )
   end
   mem.started = true

   if tk.yesno( _("A Friend's Aid"), txt ) then
      tk.msg( _("A Friend's Aid"), fmt.f( _([["Awesome! I really appreciate it!
    "So the mission is in theory a pretty simple one: I need to deliver some cargo to {pnt} in the {sys} system. Trouble is apparently I'll be getting in the middle of some sort of trade dispute with a shady Soromid company that's bribed the local Soromid pilots. Needless to say we can expect to be attacked by some thugs and the Soromid military isn't likely to be of much help.
    "That's where you come in. I just need you to follow me along, make sure I finish jumping or landing before you do, and if we encounter any hostilities, help me shoot them down. Shouldn't be too too hard as long as you've got a decent ship. I'll meet you out in space!"]]), {pnt=mem.misplanet, sys=mem.missys} ) )

      misn.accept()

      misn.setTitle( mem.misn_title )
      misn.setDesc( fmt.f( mem.misn_desc, {pnt=mem.misplanet} ) )
      misn.setReward( fmt.credits( mem.credits ) )
      mem.marker = misn.markerAdd( mem.misplanet, "low" )

      misn.osdCreate( mem.misn_title, {
         fmt.f(_("Escort Chelsea to {pnt} in the {sys} system."), {pnt=mem.misplanet, sys=mem.missys} ),
      } )

      mem.startplanet = spob.cur()

      hook.takeoff( "takeoff" )
      hook.jumpout( "jumpout" )
      hook.jumpin( "jumpin" )
      hook.land( "land" )
   else
      tk.msg( _("A Friend's Aid"), _([["Ah, OK. Let me know if you change your mind."]]) )
      misn.finish()
   end
end


function spawnChelseaShip( param )
   fass = faction.dynAdd( "Independent", "Comingout_associates", _("Mercenary") )

   chelsea = pilot.add( "Llama", fass, param, _("Chelsea") )
   chelsea:outfitRm( "all" )
   chelsea:outfitRm( "cores" )
   chelsea:outfitAdd( "Unicorp PT-68 Core System" )
   chelsea:outfitAdd( "Unicorp Hawk 350 Engine" )
   chelsea:outfitAdd( "Unicorp D-4 Light Plating" )
   chelsea:outfitAdd( "Laser Turret MK1", 2 )
   chelsea:outfitAdd( "Small Shield Booster" )
   chelsea:outfitAdd( "Small Cargo Pod", 2 )

   chelsea:setHealth( 100, 100 )
   chelsea:setEnergy( 100 )
   chelsea:setTemp( 0 )
   chelsea:setFuel( true )

   chelsea:cargoAdd( "Industrial Goods", chelsea:cargoFree() )

   chelsea:setFriendly()
   chelsea:setHilight()
   chelsea:setVisible()
   chelsea:setInvincPlayer()

   hook.pilot( chelsea, "death", "chelsea_death" )
   hook.pilot( chelsea, "jump", "chelsea_jump" )
   hook.pilot( chelsea, "land", "chelsea_land" )
   hook.pilot( chelsea, "attacked", "chelsea_attacked" )

   mem.chelsea_jumped = false
end


function spawnThug( param )
   fthug = faction.dynAdd( "Mercenary", "Comingout_thugs", _("Thugs") )
   fthug:dynEnemy(fass)

   local shiptypes = { "Hyena", "Hyena", "Hyena", "Shark", "Lancelot" }
   local shiptype = shiptypes[ rnd.rnd( 1, #shiptypes ) ]
   local thug = pilot.add( shiptype, fthug, param, fmt.f(_("Thug {ship}"), {ship=_(shiptype)} ), {ai="baddie"} )

   thug:setHostile()

   hook.pilot( thug, "death", "thug_removed" )
   hook.pilot( thug, "jump", "thug_removed" )
   hook.pilot( thug, "land", "thug_removed" )
end


function jumpNext ()
   if chelsea ~= nil and chelsea:exists() then
      chelsea:taskClear()
      chelsea:control()
      if system.cur() == mem.missys then
         chelsea:land( mem.misplanet )
      else
         chelsea:hyperspace( lmisn.getNextSystem( system.cur(), mem.missys ) )
      end
   end
end


function takeoff ()
   spawnChelseaShip( mem.startplanet )
   jumpNext()
   spawnThug()
end


function jumpout ()
   mem.lastsys = system.cur()
end


function jumpin ()
   if mem.chelsea_jumped and system.cur() == lmisn.getNextSystem( mem.lastsys, mem.missys ) then
      spawnChelseaShip( mem.lastsys )
      jumpNext()
      hook.timer( 5.0, "thug_timer" )
   else
      lmisn.fail( _("You have abandoned the mission.") )
   end
end


function land ()
   if spob.cur() == mem.misplanet then
      tk.msg( _("Another Happy Landing"), fmt.f( _([[You successfully land and dock alongside Chelsea and she approaches the worker for the cargo delivery. The worker gives her a weird look, but collects the cargo with the help of some robotic drones and hands her a credit chip. When you get back to your ships, Chelsea transfers the sum of {credits} to your account, and you idly chat with her for a while.
    "Anyway, I should probably get going now," she says. "But I really appreciated the help there! Get in touch with me again sometime. We make a great team!" You agree, and you both go your separate ways once again.]]), {credits=fmt.credits(mem.credits)} ) )
      player.pay( mem.credits )
      srm.addComingOutLog( _([[You helped escort Chelsea through a dangerous cargo delivery mission where you had to protect her from the thugs of a shady company. She said that she would like to get back in touch with you again sometime for another mission.]]) )
      misn.finish( true )
   else
      tk.msg( _("Mission Failed"), _("You have lost contact with Chelsea and therefore failed the mission.") )
      misn.finish( false )
   end
end


function thug_timer ()
   spawnThug()
   if system.cur() == mem.missys then
      spawnThug( mem.lastsys )
   end
end


function chelsea_death ()
   lmisn.fail( _("A rift in the space-time continuum causes you to have never met Chelsea in that bar.") )
end


function chelsea_jump( _p, jump_point )
   if jump_point:dest() == lmisn.getNextSystem( system.cur(), mem.missys ) then
      player.msg( fmt.f( _("Chelsea has jumped to {sys}."), {sys=jump_point:dest()} ) )
      mem.chelsea_jumped = true
   else
      lmisn.fail( _("Chelsea has abandoned the mission.") )
   end
end


function chelsea_land( _p, planet )
   if planet == mem.misplanet then
      player.msg( fmt.f( _("Chelsea has landed on {pnt}."), {pnt=planet} ) )
      mem.chelsea_jumped = true
   else
      lmisn.fail( _("Chelsea has abandoned the mission.") )
   end
end


function chelsea_attacked ()
   if chelsea ~= nil and chelsea:exists() then
      chelsea:control( false )
      hook.rm( mem.distress_timer_hook )
      mem.distress_timer_hook = hook.timer( 1.0, "chelsea_distress_timer" )
   end
end


function chelsea_distress_timer ()
   jumpNext()
end


function thug_removed ()
   spawnThug()
   hook.rm( mem.distress_timer_hook )
   jumpNext()
end
