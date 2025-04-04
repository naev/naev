
## Summary (User-visible changes)

 - S&K Small/Medium/Large Cargo Hulls keeps same price & cargo inertia & mod but becomes stackable. However, mass and cargo are split (1/2(1/2) and 2/3(1/3) resp.)
 - Red Star Small/Medium/Large Cargo Hulls work the same as their S&K counterparts.
 - S&K Superheavy/Medium-Heavy/Light combat plating become resp. 2x Heavy/Medium/Ultralight combat plating.
 - Unicorp D-72/38/9 plating become resp. 2x D-58/23/2 plating.
 - Nexus Light Stealth Plating becomes 2x Nexus Ultralight Stealth Plating.
 - Nexus Medium Stealth Plating stays as is, and becomes stackable.
 - Nexus Heavy Stealth Plating is new.
 - Patchwork Light/Medium/Heavy Plating stay as is.

## Explanations about that

There were the difficult choice to decide whether current hulls are of size 2k-1 or 2k.

Concerning Stealth Plating, Medium Stealth was found of size 2k-1 while Light Stealth Plating was found of size 2k.

Concerning the cargo hulls, that was not obvious, as some parameters suggested the first one while other the second one. So I kept parameters suggesting 2k-1 as is and extrapolated their secondary effect, while the parameters suggesting 2k were split into primary and secondary.

In particular, as I considered (which one could disagree with) the price suggesting size 2k-1, the price stayed the same and therefore the outfit translate into its single version (therefore no save\_update to do)

## Summary (Other changes)

I will detail anything on request.

For now, the list of commits gives an idea.

## TODO

 - [ ] Finish equipopt updates
 - [ ] Remove this file PR.md
 - [ ] Remove the placeholder deprecated outfits:
```
medium/sk_mediumheavy_combat_plating.xml:<outfit name="S&amp;K Medium-Heavy Combat Plating
deprecated)">
medium/unicorp_d38_medium_plating.xml:<outfit name="Unicorp D-38 Medium Plating (deprecated)">
large/sk_superheavy_combat_plating.xml:<outfit name="S&amp;K Superheavy Combat Plating (dep
ecated)">
large/unicorp_d72_heavy_plating.xml:<outfit name="Unicorp D-72 Heavy Plating (deprecated)">
small/sk_light_combat_plating.xml:<outfit name="S&amp;K Light Combat Plating (deprecated)">
small/unicorp_d9_light_plating.xml:<outfit name="Unicorp D-9 Light Plating (deprecated)">
small/nexus_light_stealth_plating.xml:<outfit name="Nexus Light Stealth Plating (deprecated)">
```


