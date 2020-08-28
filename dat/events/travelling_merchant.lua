--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Travelling Merchant">
 <trigger>enter</trigger>
 <chance>5</chance>
 <cond>system.cur():presence("Pirate") &gt; 0 and system.cur():presence("Pirate") &lt; 160 </cond>
</event>
--]]
--[[
   
   Travelling Merchant Event

Spawns a travelling merchant that can sell the player if interested.

--]]

trader_name = _("Mad Maroni")
store_name = string.format(_("%s's \"Fine\" Wares"), trader_name)
broadcastmsg = {
   string.format(_("%s's the name and selling fine shit is my game! Come get your outfits here!"), trader_name),
   _("Get your fiiiiiiiine outfits here! Guaranteed 3 space lice or less or your money back!"),
   _("Recommended by the Emperor's pet iguana's third cousin! High quality outfits sold here!"),
   _("Best outfits in the universe! So freaking good that 50% of my clients lose their hair from joy!"),
   _("Sweeet sweet space outfits! Muaha hahaha ha ha ha erk..."),
   _("...and that's how I was able to get a third liver haha. Oops is this on? Er, nevermind that. Outfits for sale!"),
}

first_hail_title = trader_name
first_hail_message = _('"Howdy Human! Er, I mean, Greetings! If you want to take a look at my wonderful, exquisite, propitious, meretricious, effulgent, ... wait, what was I talking about? Oh yes, please come see my wares on my ship. You are welcome to board anytime!"')

board_title = trader_name
board_message = _("You open the airlock and are immediately greeted by an intense humidity and heat, almost like a jungle. As you advance through the dimly lit ship you can see all types of mold and plants crowing in crevices in the wall. Wait, was that a small animal scurrying around? Eventually you reach the cargo hold that has been re-adapted as a sort of bazaar. It is a mess of different wares and most don't seem of much value, there might be some interesting find.")

function create ()
   -- Find planet
   local planets = planet.getAll()
   if planets == nil then
      local rad = system.cur():radius()
      spawn_pos = vec2.newP( rnd.rnd(0,rad*0.5), rnd.rnd(0,360) )
   else
      spawn_pos = planets[rnd.rnd(1,#planets)]:pos()
      spawn_pos = spawn_pos + vec2.newP( rnd.rnd(50, 150), rnd.rnd(0,360) )
   end

   -- Create pilot
   p = pilot.addRaw( "Mule", "trader", spawn_pos, 'Trader' )
   p:rename( trader_name )
   p:setFriendly()
   p:setInvincible()
   p:setVisplayer()
   p:setHilight(true)
   p:setActiveBoard(true)
   p:control()
   p:brake()

   -- Set up hooks
   timerdelay = 5000
   broadcastid = 1
   broadcastmsg = rnd.permutation( broadcastmsg )
   timer = hook.timer( timerdelay, "broadcast" )
   hailed_player = false
   hailhook = hook.pilot( p, "hail", "hail" )
   boardhook = hook.pilot( p, "board", "board" )
end

function broadcast ()
   if broadcastid > #broadcastmsg then broadcastid = 1 end
   p:broadcast( broadcastmsg[broadcastid], true )
   broadcastid = broadcastid+1
   timerdelay = timerdelay * 2
   timer = hook.timer( timerdelay, "broadcast" )

   if not hailed_player and not var.peek('travelling_trader_hailed') then
      p:hailPlayer()
      hailed_player = true
   end
end

function hail ()
   if not var.peek('travelling_trader_hailed') then
      var.push('travelling_trader_hailed', true)
      tk.msg( first_hail_title, first_hail_message )
      player.commClose()
   end
end

function board ()
   tk.msg( board_title, board_message )

   -- Always available outfits
   -- TODO add more
   outfits = { 'Air Freshener' }

   -- TODO add randomly chosen outfits, maybe conditioned on the current system or something?

   -- Give mission rewards the player might not have for a reason
   local mission_rewards = {
      { "Drinking Aristocrat",      "Swamp Bombing" },
      { "The Last Detail",          "Sandwich Holder" },
      { "Prince",                   "Ugly Statue" },
      { "Destroy the FLF base!",    "Star of Valor" },
      { "Disrupt a Dvaered Patrol", "Pentagram of Valor" },
      { "Nebula Satellite",         "Satellite Mock-up" },
      { "The one with the Runaway", "Toy Drone" },
      { "Deliver Love",             "Love Letter" },
      --{ "Racing Skills 2",          "Racing Trophy" }, -- This is redoable so no need to give it again
      { "Operation Cold Metal",     "Left Boot" },
   }
   for _,r in ipairs(mission_rewards) do
      local m = r[1]
      local o = r[2]
      if player.misnDone(m) and player.numOutfit(o)<1 then
         table.insert( outfits, o )
      end
   end

   -- Start the merchant and unboard.
   outfit.merchant( store_name, outfits )
   player.unboard()
end

