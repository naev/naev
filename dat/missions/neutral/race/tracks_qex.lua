
-- Peninsula and Smiling Man are in relative coordinates,
-- but Kex tour is in absolute coordinates and therefore uses waypoints.
local wp=system.get("Qex"):waypoints()

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
         },{
            vec2.new( 874, 134 ),
            vec2.new( -70, -2 ),
            vec2.new( -34, -33 ),
            vec2.new( 809, 208 ),
         },{
            vec2.new( 809, 208 ),
            vec2.new( 34, 39 ),
            vec2.new( 118, 57 ),
            vec2.new( 725, 270 ),
         },{
            vec2.new( 725, 270 ),
            vec2.new( -118, -57 ),
            vec2.new( 39, -42 ),
            vec2.new( 470, 250 ),
         },{
            vec2.new( 470, 250 ),
            vec2.new( -39, 42 ),
            vec2.new( -91, 17 ),
            vec2.new( 549, 338 ),
         },{
            vec2.new( 549, 338 ),
            vec2.new( 147, -31 ),
            vec2.new( -144, 53 ),
            vec2.new( 735, 395 ),
         },{
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
         },{
            vec2.new( 896, 165 ),
            vec2.new( -69, 1 ),
            vec2.new( 61, 55 ),
            vec2.new( 733, 177 ),
         },{
            vec2.new( 733, 177 ),
            vec2.new( -61, -55 ),
            vec2.new( 86, 80 ),
            vec2.new( 551, 176 ),
         },{
            vec2.new( 551, 176 ),
            vec2.new( -86, -80 ),
            vec2.new( -169, -106 ),
            vec2.new( 544, 322 ),
         },{
            vec2.new( 544, 322 ),
            vec2.new( 73, 42 ),
            vec2.new( -154, 98 ),
            vec2.new( 723, 379 ),
         },{
            vec2.new( 723, 379 ),
            vec2.new( 154, -99 ),
            vec2.new( 288, -3 ),
            vec2.new( 687, 465 ),
         },{
            vec2.new( 687, 465 ),
            vec2.new( -181, -1 ),
            vec2.new( 36, -183 ),
            vec2.new( 402, 398 ),
         },{
            vec2.new( 402, 398 ),
            vec2.new( -15, 66 ),
            vec2.new( -81, 55 ),
            vec2.new( 515, 506 ),
         },{
            vec2.new( 515, 506 ),
            vec2.new( 81, -56 ),
            vec2.new( -105, 94 ),
            vec2.new( 702, 532 ),
         },{
            vec2.new( 702, 532 ),
            vec2.new( 105, -94 ),
            vec2.new( -83, 111 ),
            vec2.new( 896, 503 ),
         },{
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
            wp["track_tour_6_3"], -- start near Qex IV
            wp["track_tour_1_1"],
            wp["track_tour_1_2"],
            wp["track_tour_1_3"],
         }, {
            wp["track_tour_1_3"],
            wp["track_tour_2_1"],
            wp["track_tour_2_2"],
            wp["track_tour_2_3"], -- passes by Qex V
         }, {
            wp["track_tour_2_3"],
            wp["track_tour_3_1"],
            wp["track_tour_3_2"],
            wp["track_tour_3_3"], -- circle Qex II
         }, {
            wp["track_tour_3_3"],
            wp["track_tour_4_1"],
            wp["track_tour_4_2"],
            wp["track_tour_4_3"],
         }, {
            wp["track_tour_4_3"],
            wp["track_tour_5_1"],
            wp["track_tour_5_2"],
            wp["track_tour_5_3"], -- Through asteroid field
         }, {
            wp["track_tour_5_3"],
            wp["track_tour_6_1"],
            wp["track_tour_6_2"],
            wp["track_tour_6_3"],
         }
      },
   },
}
