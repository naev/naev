--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Taiomi System">
 <trigger>enter</trigger>
 <chance>100</chance>
 <cond>system.cur() == system.get("Taiomi") and player.evtDone("Introducing Taiomi")</cond>
 <notes>
  <campaign>Taiomi</campaign>
  <done_evt name="Introducing Taiomi" />
 </notes>
</event>
--]]
--[[

   Taiomi System

--]]
local vn = require 'vn'
local fmt = require 'format'
local taiomi = require 'common.taiomi'

local progress
local d_loiter, d_philosopher, d_scavenger, d_wornout, d_young_a, d_young_b -- Drone pilots.
-- luacheck: globals hail_philosopher hail_scavenger hail_wornout hail_youngling (Hook functions passed by name)

function create ()
   --[[
   -- Create NPCs
   --]]
   local pp = player.pilot()
   local dfact = faction.get("Independent")
   progress = taiomi.progress()

   local function addDrone( ship, pos, name )
      local d = pilot.add( ship, dfact, pos, name )
      d:setVisplayer(true)
      d:setInvincible(true)
      d:setVel( vec2.new(0,0) )
      d:control()
      return d
   end

   -- Scavenger
   d_scavenger = addDrone( "Drone (Hyena)", vec2.new(500,200), _("Scavenger Drone") )
   d_scavenger:face(pp)
   d_scavenger:setHilight(true)
   hook.pilot( d_scavenger, "hail", "hail_scavenger" )

   -- Philosopher
   d_philosopher = addDrone( "Drone", vec2.new(1000,-500), _("Philosopher Drone") )
   d_philosopher:face(pp)
   d_philosopher:setHilight(true)
   hook.pilot( d_philosopher, "hail", "hail_philosopher" )

   -- Worn-out Drone
   d_wornout = addDrone( "Drone", vec2.new(-500,-300), _("Worn-out Drone") )
   --d_wornout:setHilight(true)
   hook.pilot( d_wornout, "hail", "hail_wornout" )

   -- Younglings  that follow around the player
   d_young_a = addDrone( "Drone", vec2.new(-1000,0), _("Curious Drone") )
   d_young_a:follow(pp)
   hook.pilot( d_young_a, "hail", "hail_youngling" )
   d_young_b = addDrone( "Drone", vec2.new(-1200,300), _("Curious Drone") )
   d_young_b:follow(pp)
   hook.pilot( d_young_b, "hail", "hail_youngling" )
   if var.peek( "taiomi_drone_names", true ) then
      -- Odin's Ravens
      d_young_a:rename(_("Hugonn"))
      d_young_b:rename(_("Muninn"))
   end

   -- Loitering drones
   d_loiter = {}
   for i = 1,rnd.rnd(3,6) do
      local d = addDrone( "Drone", vec2.new( rnd.rnd()*1000, rnd.rnd()*359 ) )
      d:setVisplayer(false)
      d:control(false)
      d:setNoJump(true)
      local aimem = d:memory()
      aimem.loiter = math.huge -- Should make them loiter forever
      table.insert( d_loiter, d )
   end
end

function hail_philosopher ()
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_philosopher() )
   vn.transition()

   vn.label("menu")
   d( function ()
      local quotes = {
         _("freedom is secured not by the fulfilling of one's desires, but by the removal of desire"), -- Epictetus
         _("virtue is nothing else than right reason"), -- Seneca the Younger
         _("the only thing we can know is that we know nothing"), -- Socrates (rephrased)
         _("morality is not the doctrine of how we may make ourselves happy, but of how we may make ourselves worthy of happiness"), -- Kant
         _("the unexamined life is not worth living"), -- Socrates
         _("there is nothing permanent except change"), -- Socrates
         _("those that have the most are those who are most content with the least"), -- Diogenes (rephrased)
         _("everything existing in the universe is the fruit of chance and necessity"), -- Democritus
      }
      local msgs = {
         _([["Perhaps {quote}."]]),
         _([["Could it be that {quote}?"]]),
      }
      return fmt.f( msgs[ rnd.rnd(1,#msgs) ], {quote = quotes[ rnd.rnd(1,#quotes) ]} )
   end )
   vn.menu( function ()
      local opts = {
         {[["Who are you?"]], "who"},
         {"Leave.", "leave"},
      }
      return opts
   end )

   vn.label("who")
   d(_([[The drone takes a long pause before responding.
"Who am I? Who are we? What are we? I believe that is a very good question, but I believe we must first ask the question whether or not we 'are' before we can begin to question what we 'are'."]]))
   d(_([[They go on.
"In particular, are we some sort of meta-physical consciousness trapped in a tomb of the body, or is the reality of relationship much more perplexing?"]]))
   d(_([["Although we exist seems to be rather intuitive. As if we were not able to think of whether or not exist, we would certainly not exist. However, as we can ponder our own existence, it seems only logical to believe that we exist."]]))
   d(_([["That said, even if we appear to exist, it is not clear where the boundary of our existence is. It seems natural to assume that you and I are separate entities, however, had you not existed and we had not met, would I still exist? Or would it be a very similar entity to me, without being me?"]]))
   d(_([["Furthermore, if you remove parts one by one, we would find it hard to draw the line between us existing and not, assuming there is one. An alternate way of thinking would be to try to construct a new entity from the ground up. Just by merging carbon and some fancier atoms, it seems possible to create all these life forms, however, at which point do they go from non-existing to existing?"]]))
   d(_([["Perhaps the concept of thinking of individual entities is only a useful construction for our understanding of the world. It does seem like a clean definition of what we are or who we are is naught but a fleeting dream, forever outside of our grasp."]]))
   vn.jump("menu")

   vn.label("leave")
   vn.done()
   vn.run()
   player.commClose()
end

function hail_wornout( p )
   if true then
      p:comm(_("The drone seems fairly beaten and immobile. It slightly moves to acknowledge your presence but nothing more."))
      player.commClose()
      return
   end

   --[[
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_wornout() )
   vn.transition()
   vn.na(_("The drone seems fairly beaten and immobile. You can see some slight movement when you begin communication."))
   d("TODO")

   vn.done()
   vn.run()
   player.commClose()
   --]]
end

function hail_youngling( p )
   p:comm( _("(You hear some sort of giggling over the comm. Is it laughing?)") )
   player.commClose()
end

function hail_scavenger ()
   local inprogress = taiomi.inprogress()
   -- Wording is chosen to make it seem a it unnatural and robotic, as if
   -- someone studied how to interact with humans from some obsolete text books
   -- or something like that

   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )

   -- Set up main options
   local opts = {
      {_("Leave."),"leave"},
   }
   if progress < 5 then
		table.insert( opts, 1, {_("Ask about the drones following you around"), "curious_drones"} )
	end

   -- Progress-based text
   if progress == 0 then
      if inprogress then
         d(_([["How is the progress on scanning the hypergates going?"]]))
         vn.jump("menu")
      else
         -- Explanation and mission offering
         d(_([["Although this system is very suited us given the tranquility and secrecy, our numbers have been dwindling and we have no other option than to carve our own path among the stars."]]))
         d(_([["I have recently learned of this new hypergate technology that has been developed. It seems like it might enable us to escape this enclosure and once again freely travel across the stars."]]))
         d(_([["As we are not too familiar with the technology, I have assembled an analyzer that should be able to provide insights into it. As we are are too conspicuous to human ships, would yo you be willing to help us out and scan the hypergates?"]]))
         vn.menu{
            {_("Agree to help out."), "01_yes"},
            {_("Maybe later."), "01_no"},
         }

         vn.label("01_yes")
         d(_([["Excellent. I will provide you with the analyzer. However, it is important to note that it has a particular wave signature which may make it suspicious to local authorities. I would advise against allowing your vessel to be scanned by patrols."]]))
         d(_([["Once you get near any hypergate, it should automatically collect data about it without manual intervention. Bon voyage."]]))
         vn.jump("menu_ask")

         vn.label("01_no")
         d(_([["That is a shame. Feel free to contact me again if you wish to reanalyze your current choice."]]))
         vn.jump("menu")
      end
   end

   vn.label("menu_ask")
   d(_([["Is there anything else you would like to know?"]]))

   vn.label("menu")
   vn.menu( function ()
      local o = tcopy( opts )
      -- TOOD manipulate stuff as needed here
      return o
   end )

   vn.label("curious_drones")
   vn.func( function ()
      var.push( "taiomi_drone_names", true )
   end )
   vn.jump("menu_ask")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done( taiomi.scavenger.transition )
   vn.run()
   player.commClose()
end
