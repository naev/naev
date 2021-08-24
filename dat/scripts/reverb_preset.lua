local efx_preset = {}
function efx_preset.generic ()
   return {
      type="reverb",
   }
end
function efx_preset.paddedcell ()
   return {
      type="reverb",
      highgain=0.001,
      density=0.1715,
      decaytime=0.17,
      decayhighratio=0.1,
      earlygain=0.25,
      earlydelay=0.001,
      lategain=1.2691,
      latedelay=0.002,
   }
end
function efx_preset.room ()
   return {
      type="reverb",
      highgain=0.5929,
      density=0.4287,
      decaytime=0.4,
      earlygain=0.1503,
      earlydelay=0.002,
      lategain=1.0629,
      latedelay=0.003,
   }
end
function efx_preset.bathroom ()
   return {
      type="reverb",
      highgain=0.2512,
      density=0.1715,
      decayhighratio=0.54,
      earlygain=0.6531,
      lategain=3.2734,
   }
end
function efx_preset.livingroom ()
   return {
      type="reverb",
      highgain=0.001,
      density=0.9766,
      decaytime=0.5,
      decayhighratio=0.1,
      earlygain=0.2051,
      earlydelay=0.003,
      lategain=0.2805,
      latedelay=0.004,
   }
end
function efx_preset.stoneroom ()
   return {
      type="reverb",
      highgain=0.7079,
      decaytime=2.31,
      decayhighratio=0.64,
      earlygain=0.4411,
      earlydelay=0.012,
      lategain=1.1003,
      latedelay=0.017,
   }
end
function efx_preset.auditorium ()
   return {
      type="reverb",
      highgain=0.5781,
      decaytime=4.32,
      decayhighratio=0.59,
      earlygain=0.4032,
      earlydelay=0.02,
      lategain=0.717,
      latedelay=0.03,
   }
end
function efx_preset.concerthall ()
   return {
      type="reverb",
      highgain=0.5623,
      decaytime=3.92,
      decayhighratio=0.7,
      earlygain=0.2427,
      earlydelay=0.02,
      lategain=0.9977,
      latedelay=0.029,
   }
end
function efx_preset.cave ()
   return {
      type="reverb",
      highgain=1,
      decaytime=2.91,
      decayhighratio=1.3,
      earlygain=0.5,
      earlydelay=0.015,
      lategain=0.7063,
      latedelay=0.022,
      highlimit=0,
   }
end
function efx_preset.arena ()
   return {
      type="reverb",
      highgain=0.4477,
      decaytime=7.24,
      decayhighratio=0.33,
      earlygain=0.2612,
      earlydelay=0.02,
      lategain=1.0186,
      latedelay=0.03,
   }
end
function efx_preset.hangar ()
   return {
      type="reverb",
      highgain=0.3162,
      decaytime=10.05,
      decayhighratio=0.23,
      earlygain=0.5,
      earlydelay=0.02,
      lategain=1.256,
      latedelay=0.03,
   }
end
function efx_preset.carpetedhallway ()
   return {
      type="reverb",
      highgain=0.01,
      density=0.4287,
      decaytime=0.3,
      decayhighratio=0.1,
      earlygain=0.1215,
      earlydelay=0.002,
      lategain=0.1531,
      latedelay=0.03,
   }
end
function efx_preset.hallway ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.3645,
      decayhighratio=0.59,
      earlygain=0.2458,
      lategain=1.6615,
   }
end
function efx_preset.stonecorridor ()
   return {
      type="reverb",
      highgain=0.7612,
      decaytime=2.7,
      decayhighratio=0.79,
      earlygain=0.2472,
      earlydelay=0.013,
      lategain=1.5758,
      latedelay=0.02,
   }
end
function efx_preset.alley ()
   return {
      type="reverb",
      highgain=0.7328,
      diffusion=0.3,
      decayhighratio=0.86,
      earlygain=0.25,
      lategain=0.9954,
   }
end
function efx_preset.forest ()
   return {
      type="reverb",
      highgain=0.0224,
      diffusion=0.3,
      decayhighratio=0.54,
      earlygain=0.0525,
      earlydelay=0.162,
      lategain=0.7682,
      latedelay=0.088,
   }
end
function efx_preset.city ()
   return {
      type="reverb",
      highgain=0.3981,
      diffusion=0.5,
      decayhighratio=0.67,
      earlygain=0.073,
      lategain=0.1427,
   }
end
function efx_preset.mountains ()
   return {
      type="reverb",
      highgain=0.0562,
      diffusion=0.27,
      decayhighratio=0.21,
      earlygain=0.0407,
      earlydelay=0.3,
      lategain=0.1919,
      latedelay=0.1,
      highlimit=0,
   }
end
function efx_preset.quarry ()
   return {
      type="reverb",
      highgain=0.3162,
      earlygain=0,
      earlydelay=0.061,
      lategain=1.7783,
      latedelay=0.025,
   }
end
function efx_preset.plain ()
   return {
      type="reverb",
      highgain=0.1,
      diffusion=0.21,
      decayhighratio=0.5,
      earlygain=0.0585,
      earlydelay=0.179,
      lategain=0.1089,
      latedelay=0.1,
   }
end
function efx_preset.parkinglot ()
   return {
      type="reverb",
      highgain=1,
      decaytime=1.65,
      decayhighratio=1.5,
      earlygain=0.2082,
      earlydelay=0.008,
      lategain=0.2652,
      latedelay=0.012,
      highlimit=0,
   }
end
function efx_preset.sewerpipe ()
   return {
      type="reverb",
      highgain=0.3162,
      density=0.3071,
      diffusion=0.8,
      decaytime=2.81,
      decayhighratio=0.14,
      earlygain=1.6387,
      earlydelay=0.014,
      lategain=3.2471,
      latedelay=0.021,
   }
end
function efx_preset.underwater ()
   return {
      type="reverb",
      highgain=0.01,
      density=0.3645,
      decayhighratio=0.1,
      earlygain=0.5963,
      lategain=7.0795,
   }
end
function efx_preset.drugged ()
   return {
      type="reverb",
      highgain=1,
      density=0.4287,
      diffusion=0.5,
      decaytime=8.39,
      decayhighratio=1.39,
      earlygain=0.876,
      earlydelay=0.002,
      lategain=3.1081,
      latedelay=0.03,
      highlimit=0,
   }
end
function efx_preset.dizzy ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.3645,
      diffusion=0.6,
      decaytime=17.23,
      decayhighratio=0.56,
      earlygain=0.1392,
      earlydelay=0.02,
      lategain=0.4937,
      latedelay=0.03,
      highlimit=0,
   }
end
function efx_preset.psychotic ()
   return {
      type="reverb",
      highgain=0.8404,
      density=0.0625,
      diffusion=0.5,
      decaytime=7.56,
      decayhighratio=0.91,
      earlygain=0.4864,
      earlydelay=0.02,
      lategain=2.4378,
      latedelay=0.03,
      highlimit=0,
   }
end
function efx_preset.castle_smallroom ()
   return {
      type="reverb",
      highgain=0.3981,
      diffusion=0.89,
      decaytime=1.22,
      earlygain=0.8913,
      earlydelay=0.022,
      lategain=1.9953,
   }
end
function efx_preset.castle_shortpassage ()
   return {
      type="reverb",
      highgain=0.3162,
      diffusion=0.89,
      decaytime=2.32,
      earlygain=0.8913,
      latedelay=0.023,
   }
end
function efx_preset.castle_mediumroom ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.93,
      decaytime=2.04,
      earlygain=0.631,
      earlydelay=0.022,
      lategain=1.5849,
   }
end
function efx_preset.castle_largeroom ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.82,
      decaytime=2.53,
      earlygain=0.4467,
      earlydelay=0.034,
      latedelay=0.016,
   }
end
function efx_preset.castle_longpassage ()
   return {
      type="reverb",
      highgain=0.3981,
      diffusion=0.89,
      decaytime=3.42,
      earlygain=0.8913,
      lategain=1.4125,
      latedelay=0.023,
   }
end
function efx_preset.castle_hall ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.81,
      decaytime=3.14,
      decayhighratio=0.79,
      earlygain=0.1778,
      earlydelay=0.056,
      lategain=1.122,
      latedelay=0.024,
   }
end
function efx_preset.castle_cupboard ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.89,
      decaytime=0.67,
      decayhighratio=0.87,
      earlygain=1.4125,
      earlydelay=0.01,
      lategain=3.5481,
      latedelay=0.007,
   }
end
function efx_preset.castle_courtyard ()
   return {
      type="reverb",
      highgain=0.4467,
      diffusion=0.42,
      decaytime=2.13,
      decayhighratio=0.61,
      earlygain=0.2239,
      earlydelay=0.16,
      lategain=0.7079,
      latedelay=0.036,
      highlimit=0,
   }
end
function efx_preset.castle_alcove ()
   return {
      type="reverb",
      highgain=0.5012,
      diffusion=0.89,
      decaytime=1.64,
      decayhighratio=0.87,
      earlygain=1,
      lategain=1.4125,
      latedelay=0.034,
   }
end
function efx_preset.factory_smallroom ()
   return {
      type="reverb",
      highgain=0.7943,
      density=0.3645,
      diffusion=0.82,
      decaytime=1.72,
      decayhighratio=0.65,
      earlygain=0.7079,
      earlydelay=0.01,
      lategain=1.7783,
      latedelay=0.024,
   }
end
function efx_preset.factory_shortpassage ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7943,
      density=0.3645,
      diffusion=0.64,
      decaytime=2.53,
      decayhighratio=0.65,
      earlygain=1,
      earlydelay=0.01,
      latedelay=0.038,
   }
end
function efx_preset.factory_mediumroom ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7943,
      density=0.4287,
      diffusion=0.82,
      decaytime=2.76,
      decayhighratio=0.65,
      earlygain=0.2818,
      earlydelay=0.022,
      lategain=1.4125,
      latedelay=0.023,
   }
end
function efx_preset.factory_largeroom ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7079,
      density=0.4287,
      diffusion=0.75,
      decaytime=4.24,
      decayhighratio=0.51,
      earlygain=0.1778,
      earlydelay=0.039,
      lategain=1.122,
      latedelay=0.023,
   }
end
function efx_preset.factory_longpassage ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7943,
      density=0.3645,
      diffusion=0.64,
      decaytime=4.06,
      decayhighratio=0.65,
      earlygain=1,
      earlydelay=0.02,
      latedelay=0.037,
   }
end
function efx_preset.factory_hall ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.4287,
      diffusion=0.75,
      decaytime=7.43,
      decayhighratio=0.51,
      earlygain=0.0631,
      earlydelay=0.073,
      lategain=0.8913,
      latedelay=0.027,
   }
end
function efx_preset.factory_cupboard ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7943,
      density=0.3071,
      diffusion=0.63,
      decaytime=0.49,
      decayhighratio=0.65,
      earlygain=1.2589,
      earlydelay=0.01,
      lategain=1.9953,
      latedelay=0.032,
   }
end
function efx_preset.factory_courtyard ()
   return {
      type="reverb",
      highgain=0.3162,
      density=0.3071,
      diffusion=0.57,
      decaytime=2.32,
      decayhighratio=0.29,
      earlygain=0.2239,
      earlydelay=0.14,
      lategain=0.3981,
      latedelay=0.039,
   }
end
function efx_preset.factory_alcove ()
   return {
      type="reverb",
      gain=0.2512,
      highgain=0.7943,
      density=0.3645,
      diffusion=0.59,
      decaytime=3.14,
      decayhighratio=0.65,
      earlygain=1.4125,
      earlydelay=0.01,
      lategain=1,
      latedelay=0.038,
   }
end
function efx_preset.icepalace_smallroom ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.84,
      decaytime=1.51,
      decayhighratio=1.53,
      earlygain=0.8913,
      earlydelay=0.01,
      lategain=1.4125,
   }
end
function efx_preset.icepalace_shortpassage ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.75,
      decaytime=1.79,
      decayhighratio=1.46,
      earlygain=0.5012,
      earlydelay=0.01,
      lategain=1.122,
      latedelay=0.019,
   }
end
function efx_preset.icepalace_mediumroom ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.87,
      decaytime=2.22,
      decayhighratio=1.53,
      earlygain=0.3981,
      earlydelay=0.039,
      lategain=1.122,
      latedelay=0.027,
   }
end
function efx_preset.icepalace_largeroom ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.81,
      decaytime=3.14,
      decayhighratio=1.53,
      earlygain=0.2512,
      earlydelay=0.039,
      lategain=1,
      latedelay=0.027,
   }
end
function efx_preset.icepalace_longpassage ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.77,
      decaytime=3.01,
      decayhighratio=1.46,
      earlygain=0.7943,
      earlydelay=0.012,
      latedelay=0.025,
   }
end
function efx_preset.icepalace_hall ()
   return {
      type="reverb",
      highgain=0.4467,
      diffusion=0.76,
      decaytime=5.49,
      decayhighratio=1.53,
      earlygain=0.1122,
      earlydelay=0.054,
      lategain=0.631,
      latedelay=0.052,
   }
end
function efx_preset.icepalace_cupboard ()
   return {
      type="reverb",
      highgain=0.5012,
      diffusion=0.83,
      decaytime=0.76,
      decayhighratio=1.53,
      earlygain=1.122,
      earlydelay=0.012,
      lategain=1.9953,
      latedelay=0.016,
   }
end
function efx_preset.icepalace_courtyard ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.59,
      decaytime=2.04,
      decayhighratio=1.2,
      earlygain=0.3162,
      earlydelay=0.173,
      lategain=0.3162,
      latedelay=0.043,
   }
end
function efx_preset.icepalace_alcove ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.84,
      decaytime=2.76,
      decayhighratio=1.46,
      earlygain=1.122,
      earlydelay=0.01,
      lategain=0.8913,
      latedelay=0.03,
   }
end
function efx_preset.spacestation_smallroom ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.2109,
      diffusion=0.7,
      decaytime=1.72,
      decayhighratio=0.82,
      earlygain=0.7943,
      lategain=1.4125,
      latedelay=0.013,
   }
end
function efx_preset.spacestation_shortpassage ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.2109,
      diffusion=0.87,
      decaytime=3.57,
      decayhighratio=0.5,
      earlygain=1,
      earlydelay=0.012,
      lategain=1.122,
      latedelay=0.016,
   }
end
function efx_preset.spacestation_mediumroom ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.2109,
      diffusion=0.75,
      decaytime=3.01,
      decayhighratio=0.5,
      earlygain=0.3981,
      earlydelay=0.034,
      lategain=1.122,
      latedelay=0.035,
   }
end
function efx_preset.spacestation_largeroom ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.3645,
      diffusion=0.81,
      decaytime=3.89,
      decayhighratio=0.38,
      earlygain=0.3162,
      earlydelay=0.056,
      lategain=0.8913,
      latedelay=0.035,
   }
end
function efx_preset.spacestation_longpassage ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.4287,
      diffusion=0.82,
      decaytime=4.62,
      decayhighratio=0.62,
      earlygain=1,
      earlydelay=0.012,
      latedelay=0.031,
   }
end
function efx_preset.spacestation_hall ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.4287,
      diffusion=0.87,
      decaytime=7.11,
      decayhighratio=0.38,
      earlygain=0.1778,
      earlydelay=0.1,
      lategain=0.631,
      latedelay=0.047,
   }
end
function efx_preset.spacestation_cupboard ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.1715,
      diffusion=0.56,
      decaytime=0.79,
      decayhighratio=0.81,
      earlygain=1.4125,
      lategain=1.7783,
      latedelay=0.018,
   }
end
function efx_preset.spacestation_alcove ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.2109,
      diffusion=0.78,
      decaytime=1.16,
      decayhighratio=0.81,
      earlygain=1.4125,
      lategain=1,
      latedelay=0.018,
   }
end
function efx_preset.wooden_smallroom ()
   return {
      type="reverb",
      highgain=0.1122,
      decaytime=0.79,
      decayhighratio=0.32,
      earlygain=1,
      earlydelay=0.032,
      lategain=0.8913,
      latedelay=0.029,
   }
end
function efx_preset.wooden_shortpassage ()
   return {
      type="reverb",
      highgain=0.1259,
      decaytime=1.75,
      decayhighratio=0.5,
      earlygain=0.8913,
      earlydelay=0.012,
      lategain=0.631,
      latedelay=0.024,
   }
end
function efx_preset.wooden_mediumroom ()
   return {
      type="reverb",
      highgain=0.1,
      decaytime=1.47,
      decayhighratio=0.42,
      earlygain=0.8913,
      earlydelay=0.049,
      lategain=0.8913,
      latedelay=0.029,
   }
end
function efx_preset.wooden_largeroom ()
   return {
      type="reverb",
      highgain=0.0891,
      decaytime=2.65,
      decayhighratio=0.33,
      earlygain=0.8913,
      earlydelay=0.066,
      lategain=0.7943,
      latedelay=0.049,
   }
end
function efx_preset.wooden_longpassage ()
   return {
      type="reverb",
      highgain=0.1,
      decaytime=1.99,
      decayhighratio=0.4,
      earlygain=1,
      earlydelay=0.02,
      lategain=0.4467,
      latedelay=0.036,
   }
end
function efx_preset.wooden_hall ()
   return {
      type="reverb",
      highgain=0.0794,
      decaytime=3.45,
      decayhighratio=0.3,
      earlygain=0.8913,
      earlydelay=0.088,
      lategain=0.7943,
      latedelay=0.063,
   }
end
function efx_preset.wooden_cupboard ()
   return {
      type="reverb",
      highgain=0.1413,
      decaytime=0.56,
      decayhighratio=0.46,
      earlygain=1.122,
      earlydelay=0.012,
      lategain=1.122,
      latedelay=0.028,
   }
end
function efx_preset.wooden_courtyard ()
   return {
      type="reverb",
      highgain=0.0794,
      diffusion=0.65,
      decaytime=1.79,
      decayhighratio=0.35,
      earlygain=0.5623,
      earlydelay=0.123,
      lategain=0.1,
      latedelay=0.032,
   }
end
function efx_preset.wooden_alcove ()
   return {
      type="reverb",
      highgain=0.1259,
      decaytime=1.22,
      decayhighratio=0.62,
      earlygain=1.122,
      earlydelay=0.012,
      lategain=0.7079,
      latedelay=0.024,
   }
end
function efx_preset.sport_emptystadium ()
   return {
      type="reverb",
      highgain=0.4467,
      decaytime=6.26,
      decayhighratio=0.51,
      earlygain=0.0631,
      earlydelay=0.183,
      lategain=0.3981,
      latedelay=0.038,
   }
end
function efx_preset.sport_squashcourt ()
   return {
      type="reverb",
      highgain=0.3162,
      diffusion=0.75,
      decaytime=2.22,
      decayhighratio=0.91,
      earlygain=0.4467,
      lategain=0.7943,
   }
end
function efx_preset.sport_smallswimmingpool ()
   return {
      type="reverb",
      highgain=0.7943,
      diffusion=0.7,
      decaytime=2.76,
      decayhighratio=1.25,
      earlygain=0.631,
      earlydelay=0.02,
      lategain=0.7943,
      latedelay=0.03,
      highlimit=0,
   }
end
function efx_preset.sport_largeswimmingpool ()
   return {
      type="reverb",
      highgain=0.7943,
      diffusion=0.82,
      decaytime=5.49,
      decayhighratio=1.31,
      earlygain=0.4467,
      earlydelay=0.039,
      lategain=0.5012,
      latedelay=0.049,
      highlimit=0,
   }
end
function efx_preset.sport_gymnasium ()
   return {
      type="reverb",
      highgain=0.4467,
      diffusion=0.81,
      decaytime=3.14,
      decayhighratio=1.06,
      earlygain=0.3981,
      earlydelay=0.029,
      lategain=0.5623,
      latedelay=0.045,
   }
end
function efx_preset.sport_fullstadium ()
   return {
      type="reverb",
      highgain=0.0708,
      decaytime=5.25,
      decayhighratio=0.17,
      earlygain=0.1,
      earlydelay=0.188,
      lategain=0.2818,
      latedelay=0.038,
   }
end
function efx_preset.sport_stadiumtannoy ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.78,
      decaytime=2.53,
      decayhighratio=0.88,
      earlygain=0.2818,
      earlydelay=0.23,
      lategain=0.5012,
      latedelay=0.063,
   }
end
function efx_preset.prefab_workshop ()
   return {
      type="reverb",
      highgain=0.1413,
      density=0.4287,
      decaytime=0.76,
      decayhighratio=1,
      earlygain=1,
      earlydelay=0.012,
      lategain=1.122,
      latedelay=0.012,
      highlimit=0,
   }
end
function efx_preset.prefab_schoolroom ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.4022,
      diffusion=0.69,
      decaytime=0.98,
      decayhighratio=0.45,
      earlygain=1.4125,
      earlydelay=0.017,
      lategain=1.4125,
      latedelay=0.015,
   }
end
function efx_preset.prefab_practiseroom ()
   return {
      type="reverb",
      highgain=0.3981,
      density=0.4022,
      diffusion=0.87,
      decaytime=1.12,
      decayhighratio=0.56,
      earlygain=1.2589,
      earlydelay=0.01,
      lategain=1.4125,
   }
end
function efx_preset.prefab_outhouse ()
   return {
      type="reverb",
      highgain=0.1122,
      diffusion=0.82,
      decaytime=1.38,
      decayhighratio=0.38,
      earlygain=0.8913,
      earlydelay=0.024,
      lategain=0.631,
      latedelay=0.044,
      highlimit=0,
   }
end
function efx_preset.prefab_caravan ()
   return {
      type="reverb",
      highgain=0.0891,
      decaytime=0.43,
      decayhighratio=1.5,
      earlygain=1,
      earlydelay=0.012,
      lategain=1.9953,
      latedelay=0.012,
      highlimit=0,
   }
end
function efx_preset.dome_tomb ()
   return {
      type="reverb",
      highgain=0.3548,
      diffusion=0.79,
      decaytime=4.18,
      decayhighratio=0.21,
      earlygain=0.3868,
      earlydelay=0.03,
      lategain=1.6788,
      latedelay=0.022,
      highlimit=0,
   }
end
function efx_preset.pipe_small ()
   return {
      type="reverb",
      highgain=0.3548,
      decaytime=5.04,
      decayhighratio=0.1,
      earlygain=0.5012,
      earlydelay=0.032,
      lategain=2.5119,
      latedelay=0.015,
   }
end
function efx_preset.dome_saintpauls ()
   return {
      type="reverb",
      highgain=0.3548,
      diffusion=0.87,
      decaytime=10.48,
      decayhighratio=0.19,
      earlygain=0.1778,
      earlydelay=0.09,
      latedelay=0.042,
   }
end
function efx_preset.pipe_longthin ()
   return {
      type="reverb",
      highgain=0.4467,
      density=0.256,
      diffusion=0.91,
      decaytime=9.21,
      decayhighratio=0.18,
      earlygain=0.7079,
      earlydelay=0.01,
      lategain=0.7079,
      latedelay=0.022,
      highlimit=0,
   }
end
function efx_preset.pipe_large ()
   return {
      type="reverb",
      highgain=0.3548,
      decaytime=8.45,
      decayhighratio=0.1,
      earlygain=0.3981,
      earlydelay=0.046,
      lategain=1.5849,
      latedelay=0.032,
   }
end
function efx_preset.pipe_resonant ()
   return {
      type="reverb",
      highgain=0.4467,
      density=0.1373,
      diffusion=0.91,
      decaytime=6.81,
      decayhighratio=0.18,
      earlygain=0.7079,
      earlydelay=0.01,
      lategain=1,
      latedelay=0.022,
      highlimit=0,
   }
end
function efx_preset.outdoors_backyard ()
   return {
      type="reverb",
      highgain=0.2512,
      diffusion=0.45,
      decaytime=1.12,
      decayhighratio=0.34,
      earlygain=0.4467,
      earlydelay=0.069,
      lategain=0.7079,
      latedelay=0.023,
      highlimit=0,
   }
end
function efx_preset.outdoors_rollingplains ()
   return {
      type="reverb",
      highgain=0.0112,
      diffusion=0,
      decaytime=2.13,
      decayhighratio=0.21,
      earlygain=0.1778,
      earlydelay=0.3,
      lategain=0.4467,
      latedelay=0.019,
      highlimit=0,
   }
end
function efx_preset.outdoors_deepcanyon ()
   return {
      type="reverb",
      highgain=0.1778,
      diffusion=0.74,
      decaytime=3.89,
      decayhighratio=0.21,
      earlygain=0.3162,
      earlydelay=0.223,
      lategain=0.3548,
      latedelay=0.019,
      highlimit=0,
   }
end
function efx_preset.outdoors_creek ()
   return {
      type="reverb",
      highgain=0.1778,
      diffusion=0.35,
      decaytime=2.13,
      decayhighratio=0.21,
      earlygain=0.3981,
      earlydelay=0.115,
      lategain=0.1995,
      latedelay=0.031,
      highlimit=0,
   }
end
function efx_preset.outdoors_valley ()
   return {
      type="reverb",
      highgain=0.0282,
      diffusion=0.28,
      decaytime=2.88,
      decayhighratio=0.26,
      earlygain=0.1413,
      earlydelay=0.263,
      lategain=0.3981,
      latedelay=0.1,
      highlimit=0,
   }
end
function efx_preset.mood_heaven ()
   return {
      type="reverb",
      highgain=0.7943,
      diffusion=0.94,
      decaytime=5.04,
      decayhighratio=1.12,
      earlygain=0.2427,
      earlydelay=0.02,
      latedelay=0.029,
      airabsorption=0.9977,
   }
end
function efx_preset.mood_hell ()
   return {
      type="reverb",
      highgain=0.3548,
      diffusion=0.57,
      decaytime=3.57,
      decayhighratio=0.49,
      earlygain=0,
      earlydelay=0.02,
      lategain=1.4125,
      latedelay=0.03,
      highlimit=0,
   }
end
function efx_preset.mood_memory ()
   return {
      type="reverb",
      highgain=0.631,
      diffusion=0.85,
      decaytime=4.06,
      decayhighratio=0.82,
      earlygain=0.0398,
      earlydelay=0,
      lategain=1.122,
      latedelay=0,
      airabsorption=0.9886,
      highlimit=0,
   }
end
function efx_preset.driving_commentator ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0,
      decaytime=2.42,
      decayhighratio=0.88,
      earlygain=0.1995,
      earlydelay=0.093,
      lategain=0.2512,
      latedelay=0.017,
      airabsorption=0.9886,
   }
end
function efx_preset.driving_pitgarage ()
   return {
      type="reverb",
      highgain=0.7079,
      density=0.4287,
      diffusion=0.59,
      decaytime=1.72,
      decayhighratio=0.93,
      earlygain=0.5623,
      earlydelay=0,
      latedelay=0.016,
      highlimit=0,
   }
end
function efx_preset.driving_incar_racer ()
   return {
      type="reverb",
      highgain=1,
      density=0.0832,
      diffusion=0.8,
      decaytime=0.17,
      decayhighratio=2,
      earlygain=1.7783,
      lategain=0.7079,
      latedelay=0.015,
   }
end
function efx_preset.driving_incar_sports ()
   return {
      type="reverb",
      highgain=0.631,
      density=0.0832,
      diffusion=0.8,
      decaytime=0.17,
      decayhighratio=0.75,
      earlygain=1,
      earlydelay=0.01,
      lategain=0.5623,
      latedelay=0,
   }
end
function efx_preset.driving_incar_luxury ()
   return {
      type="reverb",
      highgain=0.1,
      density=0.256,
      decaytime=0.13,
      decayhighratio=0.41,
      earlygain=0.7943,
      earlydelay=0.01,
      lategain=1.5849,
      latedelay=0.01,
   }
end
function efx_preset.driving_fullgrandstand ()
   return {
      type="reverb",
      highgain=0.2818,
      decaytime=3.01,
      decayhighratio=1.37,
      earlygain=0.3548,
      earlydelay=0.09,
      lategain=0.1778,
      latedelay=0.049,
      highlimit=0,
   }
end
function efx_preset.driving_emptygrandstand ()
   return {
      type="reverb",
      highgain=1,
      decaytime=4.62,
      decayhighratio=1.75,
      earlygain=0.2082,
      earlydelay=0.09,
      lategain=0.2512,
      latedelay=0.049,
      highlimit=0,
   }
end
function efx_preset.driving_tunnel ()
   return {
      type="reverb",
      highgain=0.3981,
      diffusion=0.81,
      decaytime=3.42,
      decayhighratio=0.94,
      earlygain=0.7079,
      earlydelay=0.051,
      lategain=0.7079,
      latedelay=0.047,
   }
end
function efx_preset.city_streets ()
   return {
      type="reverb",
      highgain=0.7079,
      diffusion=0.78,
      decaytime=1.79,
      decayhighratio=1.12,
      earlygain=0.2818,
      earlydelay=0.046,
      lategain=0.1995,
      latedelay=0.028,
   }
end
function efx_preset.city_subway ()
   return {
      type="reverb",
      highgain=0.7079,
      diffusion=0.74,
      decaytime=3.01,
      decayhighratio=1.23,
      earlygain=0.7079,
      earlydelay=0.046,
      latedelay=0.028,
   }
end
function efx_preset.city_museum ()
   return {
      type="reverb",
      highgain=0.1778,
      diffusion=0.82,
      decaytime=3.28,
      decayhighratio=1.4,
      earlygain=0.2512,
      earlydelay=0.039,
      lategain=0.8913,
      latedelay=0.034,
      highlimit=0,
   }
end
function efx_preset.city_library ()
   return {
      type="reverb",
      highgain=0.2818,
      diffusion=0.82,
      decaytime=2.76,
      decayhighratio=0.89,
      earlygain=0.3548,
      earlydelay=0.029,
      lategain=0.8913,
      latedelay=0.02,
      highlimit=0,
   }
end
function efx_preset.city_underpass ()
   return {
      type="reverb",
      highgain=0.4467,
      diffusion=0.82,
      decaytime=3.57,
      decayhighratio=1.12,
      earlygain=0.3981,
      earlydelay=0.059,
      lategain=0.8913,
      latedelay=0.037,
      airabsorption=0.992,
   }
end
function efx_preset.city_abandoned ()
   return {
      type="reverb",
      highgain=0.7943,
      diffusion=0.69,
      decaytime=3.28,
      decayhighratio=1.17,
      earlygain=0.4467,
      earlydelay=0.044,
      lategain=0.2818,
      latedelay=0.024,
      airabsorption=0.9966,
   }
end
function efx_preset.dustyroom ()
   return {
      type="reverb",
      highgain=0.7943,
      density=0.3645,
      diffusion=0.56,
      decaytime=1.79,
      decayhighratio=0.38,
      earlygain=0.5012,
      earlydelay=0.002,
      latedelay=0.006,
      airabsorption=0.9886,
   }
end
function efx_preset.chapel ()
   return {
      type="reverb",
      highgain=0.5623,
      diffusion=0.84,
      decaytime=4.62,
      decayhighratio=0.64,
      earlygain=0.4467,
      earlydelay=0.032,
      lategain=0.7943,
      latedelay=0.049,
   }
end
function efx_preset.smallwaterroom ()
   return {
      type="reverb",
      highgain=0.4477,
      diffusion=0.7,
      decaytime=1.51,
      decayhighratio=1.25,
      earlygain=0.8913,
      earlydelay=0.02,
      lategain=1.4125,
      latedelay=0.03,
      airabsorption=0.992,
      highlimit=0,
   }
end
return efx_preset
