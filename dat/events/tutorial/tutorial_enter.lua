--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Enter Tutorial Event">
 <trigger>enter</trigger>
 <chance>100</chance>
</event>
--]]
--[[

   Enter Tutorial Event

--]]

nebu_volat_wng = "#r" .. _("WARNING: NEBULA VOLATILITY DETECTED") .. "#0"

function create ()
   local sys = system.cur()
   local nebu_dens, nebu_volat = sys:nebula()
   if not var.peek( "tutorial_nebula_volatility" ) and nebu_volat > 0 then
      tk.msg( _("Volatility Rising"), string.format(
            _([[As you jump the system you notice a small alarm lights up in the control panel:
    %s
    Looking over the systems it seems like the nebula is unstable here and is beginning to damage the ship's shields. If the volatility gets any stronger, it could be fatal to the %s. Going deeper into the nebula could prove to be a very risky endeavour.]]), nebu_volat_wng, player.ship() ) )
      var.push( "tutorial_nebula_volatility", true )
   end
   evt.finish()
end

