--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Land Tutorial Event">
 <trigger>land</trigger>
 <chance>100</chance>
</event>
--]]
--[[

   Tutorial Event

--]]

function create ()
   hook.ship_buy( "ship_buy" )
   hook.takeoff( "takeoff" )
end

function ship_buy( shp )
   if not var.peek( "tutorial_time_dilation" ) then
      if ship.get(shp):time_mod() > 1 then
         tk.msg( "", _([[The person who sells you the %s looks at your record and pauses. "Ah, I see you haven't owned a large ship before! Sorry to slow you down, but I just wanted to tell you some important things about your ship. I promise I'll be but a moment.
    "Firstly, you may notice that the ship you bought has a 'Time Constant' rating. See, when operating a larger ship, you have to expend more time and effort performing the basic operations of the ship, causing your perception of time to speed up. Time Constant is simply a measure of how fast you will perceive the passage of time compared to a typical small ship; for example, a Time Constant rating of 200%% means that time appears to pass twice as fast as typical small ships.
    "This, and the slower speed of your ship, may make it difficult to use forward-facing weapons as well as on smaller ships. For the largest classes - Destroyers and up - I would generally recommend use of turreted weapons, which will automatically aim at your opponent, rather than forward-facing weapons. That's of course up to you, though.
    "That's all! Sorry to be a bother. I wish you good luck in your travels!" You thank the salesperson and continue on your way.]]):format(shp) )
         var.push( "tutorial_time_dilation", true )
      end
   end
end

function takeoff ()
   evt.finish()
end
