#!
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/large/milspec_orion_8601_core_system.xml dat/outfits/core_system/large/milspec_orion_9901_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/large/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/large/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/large/milspec_thalos_8502_core_system.xml dat/outfits/core_system/large/milspec_thalos_9802_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/large/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/large/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/large/unicorp_pt1750_core_system.xml dat/outfits/core_system/large/unicorp_pt440_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/large/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/large/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/medium/milspec_orion_4801_core_system.xml dat/outfits/core_system/medium/milspec_orion_5501_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/medium/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/medium/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/medium/milspec_thalos_4702_core_system.xml dat/outfits/core_system/medium/milspec_thalos_5402_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/medium/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/medium/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/medium/unicorp_pt200_core_system.xml dat/outfits/core_system/medium/unicorp_pt310_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/medium/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/medium/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/small/milspec_orion_2301_core_system.xml dat/outfits/core_system/small/milspec_orion_3701_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/small/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/small/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/small/milspec_thalos_2202_core_system.xml dat/outfits/core_system/small/milspec_thalos_3602_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/small/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/small/$NAM.xml
NAM=`./utils/outfits/multicore.py dat/outfits/core_system/small/unicorp_pt16_core_system.xml dat/outfits/core_system/small/unicorp_pt68_core_system.xml | ./utils/outfits/multicore2lua.py "dat/outfits/multicores/core_system/small/"`
./utils/outfits/deprecate_outfit.py dat/outfits/core_system/small/$NAM.xml
