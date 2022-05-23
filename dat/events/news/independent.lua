local head = {
   _("Welcome to Universal News Feed.")
}
local greeting = {
   _("Interesting events from around the universe."),
   _("Fair and unbiased news."),
   _("All the headlines all the time."),
}
local articles = {
   -- Just defaults to Generic for now...
}

return function ()
   return "Independent", head, greeting, articles
end
