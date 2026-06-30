local head = {
   _("The Free Trader Network.")
}
local greeting = {
   _("No chains attached."),
   _("The one true independent news source."),
}
local articles = {
   -- Just defaults to Generic for now...
}

return function ()
   return "Free Trader", head, greeting, articles
end
