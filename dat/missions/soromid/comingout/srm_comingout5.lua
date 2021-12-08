--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Garbage Person">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <done>Visiting Family</done>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Soromid</faction>
 </avail>
 <notes>
  <campaign>Coming Out</campaign>
 </notes>
</mission>
--]]
--[[

   Waste Collector

--]]
local fmt = require "format"
require "missions.soromid.comingout.srm_comingout3"
local srm = require "common.soromid"

-- luacheck: globals chelsea chelsea_attacked chelsea_death chelsea_jump chelsea_land fass fthug jumpin jumpNext jumpout land spawnChelseaShip spawnThug takeoff thug_removed thug_timer (from base mission srm_comingout3)

mem.misn_title = _("Waste Collector")
mem.misn_desc = _("Chelsea needs an escort to {pnt} so they can get rid of the garbage now filling their ship.")

function create ()
   mem.misplanet, mem.missys = planet.getS( "The Stinker" )
   if mem.misplanet == nil or mem.missys == nil or system.cur():jumpDist(mem.missys) > 4 then
      misn.finish( false )
   end

   mem.credits = 500e3
   mem.started = false

   misn.setNPC( _("Chelsea"), "soromid/unique/chelsea.webp", _("Chelsea seems like they're stressed. Maybe you should see how they're doing?") )
end


function accept ()
   local txt
   if mem.started then
      txt = _([["I'm not having any luck coming up with a plan to get rid of all of this garbage without getting jumped by those thugs. Is there any chance you could reconsider being my escort? It would be a big help."]])
   else
      txt = fmt.f( _([[You walk over to Chelsea to greet them when you notice an unpleasant odor coming off of them. Chelsea notices you. "Ah! {player}! Uh, sorry about the smell. I don't know why the hell I did this, but I took a job from some guy here and now I'm stuck with it." You ask what kind of job it is. "Erm, I kind of agreed to take their trash from them." You grimace. "Yeah," Chelsea says, "it's gross. And what's worse, I'm in over my head. I've already taken the garbage and my new ship is packed to the brim with the stuff, but there's thugs outside that seem to be waiting for me." A look of rage appears on their face as they turn to the side. "You can probably guess who's responsible for that." The thought had crossed your mind as well.
    Chelsea turns back to you. "I know I ask a lot of you, but could you help me once again? I just need an escort to {pnt} so I can drop off this garbage there. I'll give you {credits} for the trouble. What do you say?"]]), {player=player.name(), pnt=mem.misplanet, credits=fmt.credits(mem.credits)} )
   end
   mem.started = true

   if tk.yesno( _("The Dirty Job"), txt ) then
      tk.msg( _("The Dirty Job"), _([["I appreciate it very much. I'll wait at the hangar until you're ready to take off. Get ready for a fight when we get out of the atmosphere; it's going to be a bumpy ride."]]) )

      misn.accept()

      misn.setTitle( mem.misn_title )
      misn.setDesc( fmt.f( mem.misn_desc, {pnt=mem.misplanet} ) )
      misn.setReward( fmt.credits( mem.credits ) )
      mem.marker = misn.markerAdd( mem.missys, "low" )

      misn.osdCreate( mem.misn_title, {
         fmt.f( _("Escort Chelsea to {pnt} in the {sys} system."), {pnt=mem.misplanet, sys=mem.missys} ),
      } )

      mem.startplanet = planet.cur()

      hook.takeoff( "takeoff" )
      hook.jumpout( "jumpout" )
      hook.jumpin( "jumpin" )
      hook.land( "land" )
   else
      tk.msg( _("The Dirty Job"), _([["OK, I understand. I guess I'll have to find some other way to get rid of all this garbage..."]]) )
      misn.finish()
   end
end


function spawnChelseaShip( param )
   if not fass then
      fass = faction.dynAdd( "Independent", "Comingout_associates", _("Mercenary") )
   end

   chelsea = pilot.add( "Rhino", fass, param, _("Chelsea") )

   chelsea:outfitRm( "all" )
   chelsea:outfitRm( "cores" )
   chelsea:outfitAdd( "Unicorp PT-310 Core System" )
   chelsea:outfitAdd( "Melendez Buffalo XL Engine" )
   chelsea:outfitAdd( "S&K Medium Cargo Hull" )
   chelsea:outfitAdd( "Heavy Ripper Turret", 2 )
   chelsea:outfitAdd( "Enygma Systems Turreted Fury Launcher", 2 )
   chelsea:outfitAdd( "Fury Missile", 80 )
   chelsea:outfitAdd( "Medium Shield Booster" )
   chelsea:outfitAdd( "Targeting Array" )
   chelsea:outfitAdd( "Droid Repair Crew" )
   chelsea:outfitAdd( "Medium Cargo Pod", 4 )

   chelsea:setHealth( 100, 100 )
   chelsea:setEnergy( 100 )
   chelsea:setTemp( 0 )
   chelsea:setFuel( true )

   local c = commodity.new( N_("Waste Containers"), N_("A bunch of waste containers leaking all sorts of indescribable liquids.") )
   chelsea:cargoAdd( c, chelsea:cargoFree() )

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
   if not fthug then
      fthug = faction.dynAdd( "Mercenary", "Comingout_thugs", _("Thugs") )
      fthug:dynEnemy(fass)
   end

   local shiptypes = { "Hyena", "Hyena", "Shark", "Lancelot", "Admonisher" }
   local shiptype = shiptypes[ rnd.rnd( 1, #shiptypes ) ]
   local thug = pilot.add( shiptype, fthug, param, fmt.f( _("Thug {ship}"), {ship=_(shiptype)} ), {ai="baddie"} )

   thug:setHostile()

   hook.pilot( thug, "death", "thug_removed" )
   hook.pilot( thug, "jump", "thug_removed" )
   hook.pilot( thug, "land", "thug_removed" )
end


function takeoff ()
   spawnChelseaShip( mem.startplanet )
   jumpNext()
   spawnThug()
   spawnThug()
end


function land ()
   if planet.cur() == mem.misplanet then
      tk.msg( _("The Unbearable Smell Now Ends"), _([[As you dock, you can't help but notice the foul smell of garbage all around you. The planet really does fit the name. You grimace as you watch workers unload what must be hundreds of tonnes of garbage from Chelsea's ship, some of which is leaking. Eventually Chelsea's ship is emptied and you and Chelsea are handed your credit chips for the job. You and Chelsea part ways, vowing to take a shower immediately while Chelsea vows to scrub the cargo hold of their ship clean.]]) )
      player.pay( mem.credits )

      local t = time.get():tonumber()
      var.push( "comingout_time", t )

      srm.addComingOutLog( _([[You helped Chelsea get rid of a load of garbage they naively agreed to take to The Stinker as a mission, defending them from thugs along the way.]]) )

      misn.finish( true )
   else
      tk.msg( _("Mission Failed"), _("You have lost contact with Chelsea and therefore failed the mission.") )
      misn.finish( false )
   end
end


function thug_timer ()
   spawnThug()
   spawnThug()
   if system.cur() == mem.missys then
      spawnThug( mem.lastsys )
   end
end
