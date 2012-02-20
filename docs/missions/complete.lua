--[[      Test Mission

         Heavily annotated mission with the sole purpose of learning the ropes of mission creation.

         Plot: Talk to someone in a bar, accept the mission, load cargo, fly to another planet, get payed.

         Relevant files:
                        dat/missions/neutral/testmission.lua
                        dat/mission.xml

 You may want to experiment with this mission. Feel free to modify it.
 In order to play it you need to copy this file to dat/missions/neutral/
 and it the following to dat/mission.xml:

 <mission name="Test Mission">
  <lua>neutral/testmission</lua>
  <avail>
   <chance>100</chance>
   <location>Bar</location>
  </avail>
 </mission>

 It will cause the mission to appear at any Bar, with a chance of 100%.
 Note that you could also take it on multiple times, which is probably not what you
 want with a real mission. Have a look at the other missions in mission.xml to get
 an idea how appearance can be handled and feel free to ask at:
 irc.freenode.net #naev
 
 In fact it's a damn good idea to just ask there and talk with the friendly chaps about your ideas.

]]--



-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow 
else -- default English text

--[[

   This section is used to define lots of variables holding all the various texts.
   All the text should be inside this section.
   The variables can have any names and can also be stored in tables like this:
   title = {}
   title[1] = "My first title"
   title[2] = "another title"

   If you want to use direct speech in your texts you'll need quotation marks.
   Since quotation marks are already used to delimit strings,
   quotation marks that should be visible to the user would need to be escaped
   using a leading backslash:
   
   dialog1 = "\"Whatever you say, Sir.\", he said."

   To avoid this inconvenience you can enclose Strings in double square brackets,
   as seen in some of the examples below.
   
]]--

npc_name = "A Nerd"
bar_desc = "A loner is sitting lonely at a lonely table."
title = "Testmission"
cargoname = "Parcels"
misn_desc_pre_accept = [[He lifts his head from the table as you approach. You can hardly make out his eyes through the thick glasses.
"H..H..Hi", he stutters. "A..Are you a freelancer? I have a v..very important task for a freelancer.
I can pay you, qu..quite a lot, %s credits."
You wonder just how important a task must be to be worth this much money.
"I need you to del..deliver ... cargo ... to %s. I can't trust the merchant guild with this."
You wonder what the ominous cargo might be, but you dare not to ask. Do you accept?]] 
not_enough_cargospace = "Your cargo hold doesn't have enough free space."
misn_desc = "Deliver the cargo safely to %s in the %s system."
reward_desc = "%s credits on delivery."
post_accept = {}
post_accept[1] = [["Good. v..very good. But please be careful. Many would like to get their d..d..dirty fingers on that cargo."
He looks over his thick glasses with a serious glare in his eyes and says: "Don't mess up."]]
post_accept[2] = "Deliver the cargo to %s in the %s system. %s credits await you."
misn_accomplished = [[Another nerd has impatiently awaited your arrival. "Ah, my Comics, finally!"
Congratulations for a job well done.
%s credits have been transfered to your acccount.]]

end



--[[

After the texts follow the functions.
There are bascially two types, those defined inside this file and the API functions.
The API functions can be found here: http://bobbens.dyndns.org/naev-lua/index.html
You can usually identify them by their appearance for example:
misn.accept ()
system.cur ()
misn.factions ()

Notice the leading name followed by a dot and the functions name.
API functions can be called from your mission without prior import or anything like that,
just call them using the correct syntax. You'll see a few examples later on.

All other functions obviously have to be defined prior to use.

misn and evt provide very similar things, you should use misn for missions and evt for events respectively.



There are a couple of special functions that might be called from outside:



function create()                                                                                                                                            
end

 Create the mission - OBLIGATORY for every mission
   This is the script entry point.
   You have to define this function and set a few basic parameters which
   will be used by the bar, the mission computer and your board computer.
   For missions that are started in other ways, this function is simply
   the entry point, and you can use it however you wish.



function accept()
end

 Accepts the mission - OBLIGATORY for mission computer and bar
   Will be called when the user clicks 'Approach' at the bar or 'Accept' at the mission computer.

   

function abort()
end

 Specifies what happens when the user aborts the mission. Optional.
 If it's not used the mission will simply be destroyed.
 In other words, it won't be finished and will reappear.
 
]]--




function create ()
   -- This will get called when the conditions in mission.xml are met (or when the mission is initiated from another script).
   -- It is used to set up mission variables.

   -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Gamma Polaris")
   targetworld = planet.get("Polaris Prime")

   -- IMPORTANT: system claiming
   -- Missions and events may "claim" one or more systems for prioritized use. When a mission has claimed a system, it acquires the "right" to temporarily modify that system.
   -- For example, all the pilots may be cleared out, or the spawn rates may be changed. Obviously, only one mission may do this at a time, or serious conflicts ensue.
   -- Therefore, you have to make your mission claim any systems you want to get privileged rights on.
   -- When a mission tries to claim a system that is already claimed, the mission MUST terminate.
   -- If you do not need to claim any systems, please make a comment at the beginning of the create() function that states so.
   if not misn.claim ( {targetworld_sys} ) then
      abort() -- Note: this assumes you have an abort() function in your script. You may also just use misn.finish() here.
   end

   -- Set a reward. This is just a useful variable, nothing special.
   reward = 10000
   

   -- Set stuff up for the bar.
   -- Give our NPC a name and a portrait.
   misn.setNPC( npc_name, "scientist" )
   -- Describe what the user should see when he clicks on the NPC in the bar.
   misn.setDesc( bar_desc )
end


function accept ()

   -- Show a yes/no dialog with a title and a text.
   -- If the answer is no the following block will be executed.
   -- In this case, misn.finish(), it ends the mission without changing its status.
   if not tk.yesno( title, string.format( misn_desc_pre_accept, reward, targetworld:name() ) ) then
      misn.finish()
   end

   -- Check for cargo space and in case there isn't enough free space end the mission. It will keep showing up in the bar.
   if player.pilot():cargoFree() <  3 then
      tk.msg( title, not_enough_cargospace )
      misn.finish()
   end

   -- Add special mission cargo, name and quantity.
   -- The cargoID is a plain normal variable that holds this information.
   -- It can later be used to remove the cargo again.
   cargoID = misn.cargoAdd( cargoname, 3 )

   -- Set up mission information for the onboard computer and OSD.
   -- The OSD Title takes up to 11 signs.
   misn.setTitle( title )

   -- Reward is only visible in the onboard computer.
   misn.setReward( string.format( reward_desc, reward ) )

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   misn.markerAdd( targetworld, "high")


   -- Add mission
   -- At this point the mission gets added to the players active missions.
   misn.accept()

   -- Create two windows that show up after the player has accepted.
   -- Useful to explain further details.
   tk.msg( title, post_accept[1] )
   tk.msg( title, string.format( post_accept[2], targetworld:name(), targetworld_sys:name(), reward ) )


   -- Set up hooks.
   -- These will be called when a certain situation occurs ingame.
   -- In this case whenever you land.
   -- See http://bobbens.dyndns.org/naev-lua/modules/hook.html for further hooks.
   -- "land" is just the name of the function that will be called by this hook.
   hook.land("land")
end

   -- The function specified in the above hook.
function land ()
   -- Are we at our destination?
   if planet.cur() == targetworld then
      -- If so, remove the mission cargo.
      misn.cargoRm( cargoID )
      -- Give the player his reward.
      player.pay( reward )

      -- Pop up a window that tells the player that he finished the mission and got his reward.
      tk.msg( title, string.format(misn_accomplished, reward) )

      -- Mark the mission as successfully finished.
      misn.finish(true)
   end
end

-- This will be called when the player aborts the mission in the onboard computer.
function abort ()
   -- Remove cargo.
   misn.cargoRm( cargoID )
   -- Mark mission as unsuccessfully finished. It won't show up again if this mission is marked unique in mission.xml.
   misn.finish( false )
end

-- Have fun creating your own missions!

