--[[
Mission Name: Deliver Cake
Author: iwaschosen

Plot: on Zhiru you meet the same girl who received the love letters,her name is Paddy. Asks if you'd be willing to run another delivery and work with Michal on food biz. she says the man is an entrepreneur, and is trying to build a Food business
 on Zeo where he will sell her baked goodies etc. asks if you can take recipes and plans to him on Zeo. Fills you cargo hold with cake which you don’t like. You can sell cake or bring to Michal who will pay a lot of $ for the cake, player doesn’t know that he will get payed for cake he brings.
--]]

lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow
else -- default English text
-- Dialogue
  cargoname = "Recipes"
  title = "A Tasty Job"
  title2 = "Regrets.."
  npc_name = "Familiar Face"
  bar_desc = "A Familiar looking young woman is looking at you"
  firstcontact = [[The woman smiles and says "Are you not the pilot that delivered those sweet love letters to me? Yes you are! My name is Paddy, sorry I didn't introduce myself before. I was caught up in the moment.. Michal's letters are always very exciting" she blushes.
  "Anyway Michal is trying to start a restaurant on %s. Would you be interested in giving him a hand?"]]
  toobad = [["Oh, that's too bad. I really think he will be successful in his endeavors. Maybe next time you will be ready to help out" she says and goes to get herself a drink.]]
  reward = "A lot of cake"
  objectives = [["Great!" Paddy yells. "here are my recipes, bring them over to Michal on %s. I'll also have my friends load some cake for your journey"

  You begin to think of home made cake, and your stomach grumbles. Its been a while since you had some good cake.]]

  -- take off messages
  message1 = "You really regret having a full cargo hold. Some cake would be excellent right about now"
  message2 = [[As you finish your exit procedures you go back to your cargo hold and grab a slice of cake. You take a bite and spit it out immediately. It is beyond foul. The worst cake you have ever had, and your cargo hold is full of it. You sit back down and decide you could always sell it off at one of the commodity markets..]]

  -- Mission Computer text
  misn_desc = [[Deliver %s to Michal on %s in the %s system.]]
  reward_desc = [[Hopefully something other then cake!]]

  -- Mission Ending
  finish1 = [[Michal meets you as you are getting off the ship. "Ah! Paddy told me you would be coming!"
  you hand off the recipes and are about to wonder if you will be offered any payment. "I guess Paddy told you I'm starting a new venture..
  Anyway Im a little short on credits but heres %s for the troubles. I know it's not a lot, so I will also give you a 5 percent share in my new venture".
  He says. You accept the credits and begin to wonder what these shares could be worth. probably nothing but oh well. As Michal walks off he turns around and yells
  "Meet me in the bar in a few days if you want to talk more business!"]]

  finish2 =[[Michal meets you by your cargo hold. "Ah! Paddy told me you would be coming! and I see you brought some cake with you! Its my favourite!"
  You hand off the recipes and watch as workers begin to unload the cake, wondering how anybody could eat such vile cake, and why they are unloading your cake.
  "This stuff sells like crazy if you know where to sell it, I'll take care of it for you though. It sells for about 1000 a ton, I'll credit it to your account."
  As Michal walks off leaving you wondering why anyone would pay for such bad cake. Michal turns around and yells "Oh yea, You are also a 5 percent shareholder in
  my new venture now. Meet me in the bar in a few days if you want to talk more business!". As you check your account you notice you are %s credits richer.]]

  quit = "You jettison the vile cake and that wicked recipe. Vowing to never agree to take food as payment again"
  --Start Functions

  function create () --No system shall be claimed by mission
    cakes = "Food"
    startworld, startworld_sys = planet.cur()
    reward=5000
    targetworld_sys = system.get("Apez")
    targetworld = planet.get("Zeo")
    misn.setNPC(npc_name,"neutral/female1")
    misn.setDesc(bar_desc)
  end

  function accept()
    if not tk.yesno(title,string.format(firstcontact,targetworld:name()))then
      tk.msg(title,toobad)
      misn.finish()
    end
    tk.msg(title,string.format(objectives,targetworld:name()))
    amount = player.pilot():cargoFree()
    cargoID = misn.cargoAdd(cargoname,0)
    pilot.cargoAdd(player.pilot(),cakes,amount)
    misn.accept()
    --set up hooks
    takeoffhook = hook.enter("takeoff")
    landhook = hook.land("land")
    -- set up mission computer
    misn.setTitle(title)
    misn.setReward(reward_desc)
    misn.setDesc( string.format( misn_desc, cargoname,targetworld:name(), targetworld_sys:name() ) )
    misn.markerAdd(targetworld_sys, "low")
  end


  function takeoff()
    if amount==0 then
      message = message1
    else
      message = message2
    end
    tk.msg(title,message)
    hook.rm(takeoffhook)
  end

  function land()
    if planet.cur() == targetworld then
      cakeLeft = pilot.cargoHas(player.pilot(),cakes)
      if cakeLeft<amount then
        amount=cakeLeft
      end
      reward = reward + (1000*amount)
      if reward> 5000 then
        misn_accomplished = finish2
        pilot.cargoRm(player.pilot(),cakes,amount)
      else
        misn_accomplished = finish1
      end
      tk.msg(title,string.format(misn_accomplished,reward))
      player.pay(reward)
      misn.cargoRm(cargoID)
      misn.finish(true)
    end
  end

  function abort()
    tk.msg(title,quit)
    misn.cargoRm(cargoID)
    misn.cargoRm(cargoFood)
    hook.rm(takeoffhook)
    hook.rm(landhook)
  end
end
