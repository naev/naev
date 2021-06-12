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
local taiomi = require 'campaigns.taiomi'

function create ()
   --[[
   -- Create NPCs
   --]]
   local pp = player.pilot()
   local dfact = "Independent"

   local function addDrone( ship, pos, name )
      local d = pilot.add( ship, dfact, pos )
      if name then
         d:rename( name )
      end
      d:setVisplayer(true)
      d:setInvincible(true)
      d:setVel( vec2.new(0,0) )
      d:control()
      return d
   end

   -- Scavenger
   d_scavenger = addDrone( "Drone (Hyena)", vec2.new(500,200), "Scavenger Drone" )
   d_scavenger:face(pp)
   d_scavenger:setHilight(true)
   hook.pilot( d_scavenger, "hail", "hail_scavenger" )

   -- Philosopher
   d_philosopher = addDrone( "Drone", vec2.new(1000,-500), "Philosopher Drone" )
   d_philosopher:face(pp)
   d_philosopher:setHilight(true)
   hook.pilot( d_philosopher, "hail", "hail_philosopher" )

   -- Worn-out Drone
   d_wornout = addDrone( "Drone", vec2.new(-500,-300), "Worn-out Drone" )
   d_wornout:setHilight(true)
   hook.pilot( d_wornout, "hail", "hail_wornout" )

   -- Younglings  that follow around the player
   d_young_a = addDrone( "Drone", vec2.new(-1000,0), "Curious Drone" )
   d_young_a:follow(pp)
   hook.pilot( d_young_a, "hail", "hail_youngling" )
   d_young_b = addDrone( "Drone", vec2.new(-1200,300), "Curious Drone" )
   d_young_b:follow(pp)
   hook.pilot( d_young_b, "hail", "hail_youngling" )

   -- Loitering drones
   d_loiter = {}
   for i = 1,rnd.rnd(3,6) do
      local d = addDrone( "Drone", vec2.new( rnd.rnd()*1000, rnd.rnd()*359 ) )
      d:setVisplayer(false)
      d:control(false)
      d:setNoJump(true)
      local mem = d:memory()
      mem.loiter = math.huge -- Should make them loiter forever
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
         _([["Perhaps %s."]]),
         _([["Could it be that %s?"]]),
      }
      return string.format( msgs[ rnd.rnd(1,#msgs) ], quotes[ rnd.rnd(1,#quotes) ])
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
   d(_([["Although we exist seems to be rather intuitive. As if we did were not able to think of whether or not exist, we would certainly not exist. However, as we can ponder our own existence, it seems only logical to believe that we exist."]]))
   d(_([["That said, even if we appear to exist, it is not clear where the boundary of our existence is. It seems natural to assume that you and I are separate entities, however, had you not existed and we had not met, would I still exist? Or would it be a very similar entity to me, without being me?"]]))
   d(_([["Furthermore, if you remove parts one by one, we would find it hard to draw the line between us existing and not, assuming there is one. An alternate way of thinking would be to try to construct a new entity from the ground up. Just by merging carbon and some fancier atoms, it seems possible to create all these life forms, however, at which point do they go from non-existing to existing?"]]))
   d(_([["Perhaps the concept of thinking of individual entities us only a useful construction for our understanding of the world. It does seem like a clean definition of what we are or who we are is naught but a fleeting dream, forever outside of our grasp."]]))
   vn.jump("menu")

   vn.label("leave")
   vn.done()
   vn.run()
   player.commClose()
end

function hail_scavenger ()
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition()
   d("TODO")

   vn.done()
   vn.run()
   player.commClose()
end

function hail_wornout ()
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_wornout() )
   vn.transition()
   vn.na("The drone seems fairly beaten and immobile. You can see some slight movement when you begin communication.")
   d("TODO")

   vn.done()
   vn.run()
   player.commClose()
end

function hail_youngling( p )
   p:comm( _("(You hear some sort of giggling over the comm. Is it laughing?)") )
   player.commClose()
end
