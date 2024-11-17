return function ( t, _max, _params )
   -- Reweights
   t.loner_strong.w = t.loner_strong.w - 100
   t.squad.w = t.squad.w - 100
   t.capship.w = t.capship.w - 200
end, 10
