--[[Insane Businessman ]]--



-- For Translations
lang = naev.lang()
if lang == "es" then
elseif lang == "de" then 
else

--[[Text of event]]--

title = "Mr. Crumb's Ship"
msg = [[You board Crumb's ship and overpower his defenses. As you search the ship you uncover a locked safe. Using your trusty blaster, you shoot the safe open and find one million credits. "Not a bad day's work," you say. It seems it pays to associate with billionaires. ]]

end

function create ()

   startsys = system.cur()

   if not evt.claim ( {startsys} ) then
      evt.finish()
   end

   spawn_pilot()

   hook.land("land")
   hook.jumpin("jumpout")
end

function land()
   evt.finish()
end

function jumpout()
   evt.finish()
end

function spawn_pilot()
   pilot = pilot.addRaw( "Gawain", "mercenary" , true, "Dummy")[1]
   pilot:rename("Mr. Crumb")
   pilot:control()
   pilot:runaway( player.pilot(), false )
   pilot:setHilight( true )
   hook.pilot( pilot, "jump", "pilot_jump" )
   hook.pilot( pilot, "death", "pilot_death" )
   hook.pilot( pilot, "board", "pilot_board")
end

function pilot_jump()
   evt.finish()
end

function pilot_death()
   evt.finish(true)
end

function pilot_board()
   player.unboard()
   tk.msg(title, msg)
   player.pay(1000000)
   evt.finish(true)
end
