return {
   name = "Black Cat",
   cond = function ()
      return (system.cur():presence("Wild Ones") > 0)
   end,
}
