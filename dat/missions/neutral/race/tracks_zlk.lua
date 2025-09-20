
local control_1={
   ["ZR_01_01"]=vec2.new( 942, 466 ),
   ["ZR_01_02"]=vec2.new( 830, 176 ),
   ["ZR_01_03"]=vec2.new( 711, 291 ),
   ["ZR_01_04"]=vec2.new( 772, 409 ),
   ["ZR_01_05"]=vec2.new( 830, 577 ),
   ["ZR_01_06"]=vec2.new( 651, 698 ),
   ["ZR_01_07"]=vec2.new( 530, 585 ),
   ["ZR_01_08"]=vec2.new( 651, 466 ),
   ["ZR_01_09"]=vec2.new( 709, 528 ),
   ["ZR_01_10"]=vec2.new( 884, 579 ),
   ["ZR_01_11"]=vec2.new( 942, 471 ),  -- almost ZR_01_01
}

local control_2={
   ["ZR_02_01"]=vec2.new( 1062, 350 ),
   -- TODO: finish
}

return {
   {
      name = _("ZR-01"),
      scale = 15,
      centre = true,
      track = {
         {
            control_1["ZR_01_01"],
            vec2.new( 3, -288 ),
            vec2.new( 132, -1 ),
            control_1["ZR_01_02"],
         }, {
            control_1["ZR_01_02"],
            vec2.new( -132, 1 ),
            vec2.new( -2, -113 ),
            control_1["ZR_01_03"],
         }, {
            control_1["ZR_01_03"],
            vec2.new( 2, 113 ),
            vec2.new( -56, 1 ),
            control_1["ZR_01_04"],
         }, {
            control_1["ZR_01_04"],
            vec2.new( 56, -1 ),
            vec2.new( -7, -172 ),
            control_1["ZR_01_05"],
         }, {
            control_1["ZR_01_05"],
            vec2.new( -5, 178 ),
            vec2.new( 144, -11 ),
            control_1["ZR_01_06"],
         }, {
            control_1["ZR_01_06"],
            vec2.new( -123, 1 ),
            vec2.new( 1, 116 ),
            control_1["ZR_01_07"],
         }, {
            control_1["ZR_01_07"],
            vec2.new( -1, -116 ),
            vec2.new( -114, -3 ),
            control_1["ZR_01_08"],
         }, {
            control_1["ZR_01_08"],
            vec2.new( 59, -1 ),
            vec2.new( 2, -63 ),
            control_1["ZR_01_09"],
         }, {
            control_1["ZR_01_09"],
            vec2.new( 2, 51 ),
            vec2.new( -170, 0 ),
            control_1["ZR_01_10"],
         }, {
            control_1["ZR_01_10"],
            vec2.new( 62, -1 ),
            vec2.new( 4, 108 ),
            control_1["ZR_01_11"],
         },
      },
   }, {
      name = _("ZR-02"),
      scale = 15,
      centre = true,
      track = {
         {
            control_2["ZR_02_01"],
            vec2.new( 3, -112 ),
            vec2.new( 117, -4 ),
            vec2.new( 945, 236 ),
         }, {
            vec2.new( 945, 236 ),
            vec2.new( -117, 4 ),
            vec2.new( 2, 60 ),
            vec2.new( 825, 171 ),
         }, {
            vec2.new( 825, 171 ),
            vec2.new( 4, -54 ),
            vec2.new( 120, -4 ),
            vec2.new( 706, 118 ),
         }, {
            vec2.new( 706, 118 ),
            vec2.new( -240, -1 ),
            vec2.new( 3, -108 ),
            vec2.new( 471, 231 ),
         }, {
            vec2.new( 471, 231 ),
            vec2.new( -3, 108 ),
            vec2.new( -113, -1 ),
            vec2.new( 587, 350 ),
         }, {
            vec2.new( 587, 350 ),
            vec2.new( 113, 1 ),
            vec2.new( 2, -113 ),
            vec2.new( 709, 466 ),
         }, {
            vec2.new( 709, 466 ),
            vec2.new( 1, 175 ),
            vec2.new( 120, 3 ),
            vec2.new( 590, 639 ),
         }, {
            vec2.new( 590, 639 ),
            vec2.new( -120, -3 ),
            vec2.new( 2, 121 ),
            vec2.new( 474, 521 ),
         }, {
            vec2.new( 474, 521 ),
            vec2.new( 0, -50 ),
            vec2.new( -116, -2 ),
            vec2.new( 589, 470 ),
         }, {
            vec2.new( 589, 470 ),
            vec2.new( 116, 2 ),
            vec2.new( -118, -2 ),
            vec2.new( 946, 468 ),
         }, {
            vec2.new( 946, 468 ),
            vec2.new( 118, 2 ),
            vec2.new( -3, 112 ),
            vec2.new( 1062, 350 ),
         },
      },
   },
}
