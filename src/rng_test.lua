-- luacheck: globals rnd

for i=1,10000 do
   local r = rnd.rnd()
   assert( r >= 0 and r <= 1, "rnd.rnd()" )
end

for _,vals in ipairs{ {5,10}, {9,3}, {-3,5}, } do --{5,nil} } do
   local l,h = vals[1], vals[2]
   local t = {}
   for i=1,10000 do
      local r = rnd.rnd(l, h)
      t[r] = (t[r] or 0) + 1
   end
   -- Flip order if necessary for testing
   if h~=nil and l>h then
      l,h = h,l
   elseif h==nil then
      h = l
      l = 0
   end
   for k,v in pairs(t) do
      assert( k>=l and k<=h, "rnd.rnd(l,h) valid keys", l, h )
   end
   for i=l,h do
      assert( t[i] > 0, "rnd.rnd(l,h) all keys", l, h )
   end
end

local function uniform_wrap()
   return rnd.uniform( -4, 4 )
end
for _,vals in ipairs{
   {1, rnd.sigma},
   {2, rnd.twosigma},
   {3, rnd.threesigma},
   {4, uniform_wrap},
} do
   local R,F = vals[1], vals[2]
   local min, max = math.huge, -math.huge
   -- With 10k samples statistically it passes most of the time
   for i=1,10000 do
      local r = F()
      min = math.min( min, r )
      max = math.max( max, r )
   end
   print( min, max, R )
   assert( min<-R*0.95 and max>R*0.95 and min>=-R and max<=R, "rnd.sigma", R )
end

local function table_nogen( N )
   return N
end
local function table_gen( N )
   local t = {}
   for i=1,N do
      t[i] = i
   end
   return t
end
for _,F in ipairs{ table_nogen, table_gen } do
   for i=1,100 do
      local N = 1000
      local t = rnd.permutation( F(N) )
      local match = 0
      for j=1,N do
         if t[j]==j then
            match = match+1
         end
      end
      assert( match<100, "permutation matches" )
      table.sort(t)
      match = 0
      for j=1,N do
         if t[j]==j then
            match = match+1
         end
      end
      assert( match==N, "sorted permutation matches" )
   end
end

print("test OK")
