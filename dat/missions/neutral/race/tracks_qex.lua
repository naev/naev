return {
   {
      name = _("Peninsula"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 939, 394 ),
            vec2.new( 6, -128 ),
            vec2.new( 70, 2 ),
            vec2.new( 874, 134 ),
         },
         {
            vec2.new( 874, 134 ),
            vec2.new( -70, -2 ),
            vec2.new( -34, -33 ),
            vec2.new( 809, 208 ),
         },
         {
            vec2.new( 809, 208 ),
            vec2.new( 34, 39 ),
            vec2.new( 118, 57 ),
            vec2.new( 725, 270 ),
         },
         {
            vec2.new( 725, 270 ),
            vec2.new( -118, -57 ),
            vec2.new( 39, -42 ),
            vec2.new( 470, 250 ),
         },
         {
            vec2.new( 470, 250 ),
            vec2.new( -39, 42 ),
            vec2.new( -91, 17 ),
            vec2.new( 549, 338 ),
         },
         {
            vec2.new( 549, 338 ),
            vec2.new( 147, -31 ),
            vec2.new( -144, 53 ),
            vec2.new( 735, 395 ),
         },
         {
            vec2.new( 735, 395 ),
            vec2.new( 82, -37 ),
            vec2.new( -15, 226 ),
            vec2.new( 939, 394 ),
         },
      },
   }, {
      name = _("Smiling Man"),
      scale = 15,
      center = true,
      track = {
         {
            vec2.new( 942, 381 ),
            vec2.new( 5, -104 ),
            vec2.new( 69, -1 ),
            vec2.new( 896, 165 ),
         },
         {
            vec2.new( 896, 165 ),
            vec2.new( -69, 1 ),
            vec2.new( 61, 55 ),
            vec2.new( 733, 177 ),
         },
         {
            vec2.new( 733, 177 ),
            vec2.new( -61, -55 ),
            vec2.new( 86, 80 ),
            vec2.new( 551, 176 ),
         },
         {
            vec2.new( 551, 176 ),
            vec2.new( -86, -80 ),
            vec2.new( -169, -106 ),
            vec2.new( 544, 322 ),
         },
         {
            vec2.new( 544, 322 ),
            vec2.new( 73, 42 ),
            vec2.new( -154, 98 ),
            vec2.new( 723, 379 ),
         },
         {
            vec2.new( 723, 379 ),
            vec2.new( 154, -99 ),
            vec2.new( 288, -3 ),
            vec2.new( 687, 465 ),
         },
         {
            vec2.new( 687, 465 ),
            vec2.new( -181, -1 ),
            vec2.new( 36, -183 ),
            vec2.new( 402, 398 ),
         },
         {
            vec2.new( 402, 398 ),
            vec2.new( -15, 66 ),
            vec2.new( -81, 55 ),
            vec2.new( 515, 506 ),
         },
         {
            vec2.new( 515, 506 ),
            vec2.new( 81, -56 ),
            vec2.new( -105, 94 ),
            vec2.new( 702, 532 ),
         },
         {
            vec2.new( 702, 532 ),
            vec2.new( 105, -94 ),
            vec2.new( -83, 111 ),
            vec2.new( 896, 503 ),
         },
         {
            vec2.new( 896, 503 ),
            vec2.new( 21, -30 ),
            vec2.new( 0, 63 ),
            vec2.new( 942, 381 ),
         },
      },
   }, {
      name = _("Qex Tour"),
      track = {
         {
            vec2.new(  5e3,  2e3 ), -- start near Qex IV
            vec2.new( -1e3,  3e3 ),
            vec2.new(  1e3, -1e3 ),
            vec2.new(  2e3,  2e3 ),
         }, {
            vec2.new(  2e3,  2e3 ),
            vec2.new( -1e3,  1e3 ),
            vec2.new(  2e3,    0 ),
            vec2.new( -1e3,  5e3 ), -- passes by Qex V
         }, {
            vec2.new( -1e3,  5e3 ),
            vec2.new( -6e3,    0 ),
            vec2.new(  1e3,  1e3 ),
            vec2.new( -8e3,  2e3 ), -- circle Qex II
         }, {
            vec2.new( -8e3,  2e3 ),
            vec2.new( -1e3, -1e3 ),
            vec2.new( -1e3,  1e3 ),
            vec2.new( -8e3,  4e3 ),
         }, {
            vec2.new( -8e3,  4e3 ),
            vec2.new(  1e3, -1e3 ),
            vec2.new(    0,  2e3 ),
            vec2.new( -1e3,  2e3 ), -- Through asteroid field
         }, {
            vec2.new( -1e3,  2e3 ),
            vec2.new(    0, -4e3 ),
            vec2.new(  1e3, -3e3 ),
            vec2.new(  5e3,  2e3 ),
         }
      },
   },
}
