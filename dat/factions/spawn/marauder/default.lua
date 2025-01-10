return function ( t, _max, _params )
   -- Reweights
   t.squad.w = t.squad.w - 100
   t.capship.w = t.capship.w - 200
end, 10
