--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Generic News">
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Event for creating news
--]]
local fmt = require "format"
local lmisn = require "lmisn"
local lf = require "love.filesystem"

-- TODO probably better to change the system to try to enforce an average number of articles or something instead of random time since last article and random chance
local MIN_ARTICLES   = 3 -- Minimum number of articles to try to enforce
local ARTICLE_CHANCE = 0.25 -- Chance a new article can appear

local add_article, add_econ_article, add_header -- forward-declared functions

-- List to treat special factions diffferently
local override_list = {
   -- Treat pirate clans the same (at least for now)
   ["Wild Ones"]     = "Pirate",
   ["Raven Clan"]    = "Pirate",
   ["Dreamer Clan"]  = "Pirate",
   ["Black Lotus"]   = "Pirate",
}

local header_table = {}
local greeting_table = {}
local articles = {}

local econ_articles = {
   {
      head = _("Unfortunate Merchant Goes Bankrupt"),
      body = _([[A merchant was forced into bankruptcy due to a badly timed trade of {cargo} on {pnt}. "I thought {credits} per tonne was a good deal, but it turns out I should have waited," the merchant said.]])
   },
   {
      head = _("Shipping Company Goes Out of Business"),
      body = _([[A small shipping business failed just this decaperiod. While it was already failing, what finally put the company under was a poorly-timed trade of {cargo} on {pnt} for {credits} per tonne. "It was poor executive decision," one analyst asserts. "Patience is key when trading, and it's clear that the owner of this company didn't have enough of that."]])
   },
   {
      head = _("Interview with an Economist"),
      body = _([[One of the galaxy's foremost experts on economics gives an interview explaining our modern economy. "We actually have a pretty good understanding of how the economy works. For example, we were able to predict what the price of {cargo} on {pnt} would reach very accurately; the actual price reached was {credits} per tonne, which we were only off by about 15%. Over time, we hope to lower that margin of error to as little as 2%."]])
   },
   {
      head = _("Economist Describes Sinusoidal Economic Theory"),
      body = _([[A little-known economist discusses a controversial economic theory. "When you look at the trends, it resembles a sine wave. For instance, the price of {cargo} on {pnt} is now {credits} per tonne, and it seems to return to that price with some regularity. We are working on developing a model to predict these curves more accurately." Other economists disagree, however, attributing these economists' results to chance.]])
   },
   {
      head = _("Young Pilot Buys Their First Commodity"),
      body = _([[A young pilot has bought some {cargo} as a way of breaking into the freelance piloting business. Born and raised on {pnt}, where they bought their first commodity, they spoke with enthusiasm for the new career. "You know, it's real exciting! Even on my home planet the price of {credits} per tonne isn't static, but when you look all around, there's so much price variation, so much potential for profit! I'm excited to finally get started."]])
   },
   {
      head = _("Corporate Scandal Rips Through the Galaxy"),
      body = _([[Economists are attributing the price of {cargo} on {pnt} to a scandal involving WarpTron Industries. Debates have ensued regarding whether or not the price, seen to be {credits} per tonne, will go up, down, or remain the same this time.]])
   },
   {
      head = _("Commodity Trading Likened to Gambling"),
      body = _([[In a controversial statement, one activist has likened commodity trading to gambling. "It's legalized gambling, plain and simple! Right now the price of {cargo} on {pnt} is {credits} per tonne, for example, but everyone knows the price fluctuates. Tomorrow it could be lower, or it could be higher. Who knows? Frankly, it is my firm opinion that this 'commodity trading' is self-destructive and needs to stop."]])
   },
   {
      head = _("Leadership Decision Disrupts Prices"),
      body = _([[The price of {cargo} was jeopardized on {pnt} today when the local government passed a controversial law, bringing it to {credits} per tonne. Protests have erupted demanding a repeal of the law so that the economy can stabilize.]])
   },
   {
      head = _("Five Cycle Old Child Starts Commodity Trading"),
      body = _([[A child no more than five cycles old has started commodity trading early, buying 1 tonne of {cargo}. A native of {pnt}, she explained that she has a keen interest in the economy and wishes to be a space trader some day. "I bought it for {credits}, but it goes up and down, so if you time it right, you can make more money! My mom is a trader too and I want to be just like her."]])
   },
}

-- Appends a table to a destination table
local function merger( dest, src, key )
   if not src then return end

   if not dest[key] then
      dest[key] = {}
   end

   for i,v in ipairs(src) do
      local k = key
      if type(v) == "table" and v.key then -- allow overwriting keys
         k = v.key
         if not dest[k] then
            dest[k] = {}
         end
      end
      table.insert( dest[k], v )
   end
end

-- Try to load all the modular news files
for k,v in ipairs(lf.getDirectoryItems("events/news")) do
   local key, head, greet, art = require( "events.news."..string.gsub(v,".lua","") )()
   merger( header_table, head, key )
   merger( greeting_table, greet, key )
   merger( articles, art, key )
end

-- Return an economy article based on the given commodity, planet object, and number of credits.
local function get_econ_article( cargo, pnt, credits )

   local i = rnd.rnd( 1, #econ_articles )
   local head = econ_articles[i].head
   local body = fmt.f( econ_articles[i].body, {cargo=cargo, pnt=pnt, credits=fmt.credits(credits)} )

   return head, body
end

function create()
   hook.load( "land" )
   hook.land( "land" )
end

-- create news
function land ()
   local p = spob.cur()
   local s = p:services()
   -- Needs to be inhabited and have a bar for there to be news
   if not s.inhabited or not s.bar or p:tags().nonews then
      -- Remove old headers (we only want one at a time)
      for i, article in ipairs( news.get( "header" ) ) do
         article:rm()
      end
      return
   end
   -- Needs a faction for there to be news
   local f = p:faction()
   if f == nil then return end
   local my_faction = f:nameRaw()

   local t = override_list[my_faction]
   if t then
      my_faction = t
   end

   add_header( my_faction )
   local n = #news.get( my_faction )
   if n < MIN_ARTICLES then
      for i=1,MIN_ARTICLES-n do
         add_article( my_faction, true )
      end
   end
   add_article( my_faction )
   add_econ_article( my_faction )
end


function add_header( my_faction )
   -- Remove old headers (we only want one at a time)
   for i, article in ipairs( news.get( "header" ) ) do
      article:rm()
   end

   if header_table[my_faction] == nil then
      warn( fmt.f( _([[News: Faction '{fct}' does not have entry in faction table!]]), {fct=my_faction}) )
      my_faction = 'Generic'
   end

   local function sample_one( t )
      if type(t) == "table" then
         return t[ rnd.rnd(1,#t) ]
      end
      return t
   end

   local cur_t = time.get()
   local head = sample_one( header_table[my_faction] )
   local body = sample_one( greeting_table[my_faction] )
   local a = news.add( my_faction, head, body, cur_t + time.new( 0, 0, 1 ), 0, -1 ) -- Highest priority
   a:bind( "header" )
end

function add_article( my_faction, force )
   if not force then
      local last_article = var.peek( "news_last_article" )
      if last_article ~= nil then
         local t = time.fromnumber( last_article )
         if time.get() - t < time.new( 0, 1, 5000 ) then
            return false
         end
      end

      if rnd.rnd() > ARTICLE_CHANCE then
         return false
      end
   end

   local function jointest( dest, src )
      for k,v in ipairs(src) do
         -- Make sure it doesn't already exist and passes test
         local tag = v.tag or v.head
         if #news.get(tag)==0 and (not v.test or v.test()) then
            table.insert( dest, v )
         end
      end
   end

   -- Find potential article list
   -- TODO add weighting?
   local alst = {}
   jointest( alst, articles[ my_faction ] )
   if my_faction ~= "Generic" and faction.get(my_faction):tags().generic then
      jointest( alst, articles[ "Generic" ] )
   end
   if alst == nil or #alst <= 0 then
      return false
   end

   -- Get the elemnt
   local elem  = alst[ rnd.rnd( 1, #alst ) ]
   local head  = elem.head
   local body  = elem.body
   local tag   = elem.tag or head
   local priority = elem.priority or 6 -- Slightly lower priority than default
   if type(body)=="function" then
      body = body()
      if body == nil then
         return false -- Skip
      end
   end

   -- Add the news for roughly 10 periods
   local exp = time.get() + time.new( 0, 10, 5000 * rnd.sigma() )
   local a = news.add( my_faction, _(head), body, exp, nil, priority )
   a:bind( tag )
   var.push( "news_last_article", time.get():tonumber() )

   return true
end


function add_econ_article( my_faction )
   local last_article = var.peek( "news_last_econ_article" )
   local t = nil
   local generic = faction.get(my_faction):tags().generic
   if last_article ~= nil then t = time.fromnumber( last_article ) end
   if (t == nil or time.get() - t > time.new( 0, 2, 0 ))
         and rnd.rnd() < 0.75 and generic then
      local planets = {}
      for i, s in ipairs( lmisn.getSysAtDistance( system.cur(), 2, 4 ) ) do
         for j, p in ipairs( s:spobs() ) do
            local f = p:faction()
            if f and f:tags().generic and #(p:commoditiesSold()) > 0 then
               planets[ #planets + 1 ] = p
            end
         end
      end
      if #planets > 0 then
         local p = planets[ rnd.rnd( 1, #planets ) ]
         local pd = time.get() - time.new(
               0, p:system():jumpDist() + rnd.rnd( 0, 1 ), 9000 * rnd.sigma() )
         local exp = time.get() + time.new( 0, 5, 5000 * rnd.sigma() )
         local commchoices = p:commoditiesSold()
         local commod = commchoices[ rnd.rnd( 1, #commchoices ) ]
         local price = commod:priceAtTime( p, pd )
         local head, body = get_econ_article( commod, p, price )
         news.add( "Generic", head, body, exp, pd, 6 ) -- Slightly lower priority
         p:recordCommodityPriceAtTime( pd )
         var.push( "news_last_econ_article", time.get():tonumber() )
      end
   end

   -- Remove old econ articles (we only want one at a time)
   for i, article in ipairs(news.get("econ")) do
      article:rm()
   end

   -- Ignore non-generic factions
   if not generic then
      return
   end

   local cur_t = time.get()
   local body = ""
   for i, sys in ipairs( lmisn.getSysAtDistance( system.cur(), 0, 1 ) ) do
      for j, plnt in ipairs( sys:spobs() ) do
         local commodities = plnt:commoditiesSold()
         if #commodities > 0 then
            body = body .. "\n\n" .. fmt.f( _("{pnt} in {sys}"), {pnt=plnt, sys=sys} )
            for k, comm in ipairs( commodities ) do
               body = body .. "\n" .. fmt.f( _("{cargo}: {price}"), {cargo=comm,
                     price=fmt.number(comm:priceAtTime(plnt, cur_t))} )
            end
            plnt:recordCommodityPriceAtTime( cur_t )
         end
      end
   end
   if body ~= "" then
      -- Create news, expires immediately when time advances (i.e.
      -- when you take off from the planet).
      -- Lowest priority
      local a = news.add( "Generic", _("Current Market Prices"), body, cur_t + time.new( 0, 0, 1 ), nil, 11 )
      a:bind( "econ" )
   end
end
