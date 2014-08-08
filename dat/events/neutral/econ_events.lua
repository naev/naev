
lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english

   events = {

      {
         "famine",  --event name/type
         25,         --event takes 25 STP
         {"Food", 2.0} --commodity and it's new price
      },

      {
         "bumper harvest",
         25,
         {"Food", .5} --food price now at half the price it was
      },

      {
         "worker's strike",
         25,
         {"Industrial Goods", 1.5},
         {"Ore", 1.5}
      },

      {
         "cat convention",
         25,
         {"Luxury Goods", 1.5},
         {"Medicine",1.25}
      },

      {
         "disease outbreak",
         25,
         {"Medicine",2},
         {"Food",1.3}
      }
   }

end

commodities={"Food", "Medicine", "Luxury Goods","Industrial Goods", "Ore"}
factions = {"Empire","Sirius","Frontier","Soromid","Dvaered","Independent"}

original={}   --original prices

function create()

   make_event()

end


   --make the news event for the selected event and system
function make_article(sys, event)

   title = "System "..sys:name().."  experienced "..event[1]
   body = "System "..sys:name().." recently experienced a "..event[1]..", changing "

      --say how the prices have changed
   for i=1, #event-2 do
      comm_name = event[i+2][1]
      body = body.."the price of "..comm_name..string.format(" from %.0f credits per ton to %.0f",math.abs(original[i]),econ.getPrice(comm_name, sys))
      if i<#event-3 then
         body = body..", "
      elseif i==#event-3 then
         body = body..", and "
      elseif i==#event-2 then
         body = body.."."
      end
   end

   body = body.." This event is expected to end in "..event[2].." STP, at date "..(time.create(0,event[2],0)+time.get()):str()
      --make the article
   article = news.add("Generic", title, body, time.get() + time.create( 0, event[2], 0))
   article:bind("economic event")

end


   --make a single economic event
function make_event()

      --get the event
   event_num = math.random(#events)
   event = events[event_num]

      --get a system with >=500 presence and with no current events
   economic_articles = news.get("economic events")
   got_sys=false
   for i=0,20 do
      sys=system.get(true)
      syspresences = system.presences(sys)
      sum=0
      for _,v in ipairs(factions) do
         if syspresences[v] then
            sum=sum+syspresences[v]
         end
      end
      system_nottaken=true
      for i=1,#economic_articles do --check this system wasn't already taken
         if string.match( article:title(), sys:name()) then
            system_nottaken=false
            break
         end
      end
      if system_nottaken and sum > 500 then
         got_sys=true
         break
      end
   end
   if not got_sys then
      return
   end

      --get the original prices
   for i=1,#event-2 do
      comm = event[i+2]
      comm_name = event[i+2][1]
      if econ.isSystemPriceSet(comm[1], sys) then
         original[i] = econ.getPrice(comm_name, sys)
      else
         original[i] = -econ.getPrice(comm_name, sys) --negative to indicate unset price
      end
   end

      --put in new prices
   for i=1,#event-2 do
      comm = event[i+2]
      price = econ.getPrice(comm[1], sys)
      price = price*comm[2]
      econ.setSystemPrice(comm[1], sys, price)
   end

      --update the prices, and make the article
   econ.updatePrices()

   make_article(sys, event)

      --set up the event ending
      --put all the information we'll need into a string
   str=" system:"..sys:name()..","
   for i=1,#event-2 do
      comm_name = event[i+2][1]
      str=str..string.format("%sorigprice:%f,",comm_name, original[i] )
   end

   hook.date( time.create(0, event[2], 0), "end_event", str )
   evt.save(true)

end


--end the event, and return the values to their original values
function end_event(str)
      --first, get all the information we'll need
   sys=nil
   num_changed_comms=0
   weighted=0
   comms={}

   tmp    = str:match("system:[a-zA-Z' ]*,")
   sysname = tmp:match(":[a-zA-Z' ]*"):sub(2)
   sys    = system.get(sysname)
   for i=1,#commodities do
      comm_entry = str:match(commodities[i].."origprice:[\-0-9]*")
      if (comm_entry) then
         comm_price = string.match(comm_entry, "[\-0-9]+")
         comm_price = tonumber(comm_price)
         comms[#comms+1] = {commodities[i], comm_price}
      end
   end

      --reset prices to their original states
   for i=1, #comms do
      comm=comms[i]
      if comm[2]<=0 then
         econ.unsetSystemPrice(comm[1], sys)
      else
         econ.setSystemPrice(comm[1], sys, comm[2])
      end
   end

   evt.finish()

end

