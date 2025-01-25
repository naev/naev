local head = {
   _("Pride of House O'rez"),
}
local greeting = {
   _("Latest news on the war front."),
}
local articles = {
   {
      head = N_([[More Fabrications and Lies by House Yetmer]]),
      body = _([[Light has been shed on a new sham campaign by House Yetmer, attempting to cover up the fact they are losing the war. The traitors are making false claims regarding bloodlines, which will soon be squashed by the House O'rez elite lawyer team working hard at Jade Court.]]),
   },
}

return function ()
   return "O'rez", head, greeting, articles
end
