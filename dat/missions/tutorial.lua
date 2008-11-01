--[[

   The beginner player tutorial.

   Does simple stuff like teach the player to fly around or use his communications system.

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   title[1] = "Tutorial"
   text = {}
   text[1] = "Would you like to run the Tutorial to learn how to play NAEV?"
   misn_title = "NAEV Tutorial"
   misn_reward = "Knowledge of how to play the game."
   misn_desc = "New Player Tutorial to learn how survive in the universe."
end

      
function create()

   if tk.yesno( title[1], text[1] )
      then
      misn.accept()

      -- Set basic mission information.
      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc )
   end

end
