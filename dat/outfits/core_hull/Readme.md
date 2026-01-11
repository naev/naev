## Balancing Core Hulls

Rough rule for absorb and penetration:
 * 0% absorb for 2+ class above (e.g., medium attacking ultra-light)
 * 30% absorb for same class
 * 60% absorb for -2 class (e.g., ultra-light attacking medium)
 * 90% absorb for -4 class (e.g., ultra-light attacking heavy)

So for a ship to take 30% damage from current class, the rough rule is:
 * ultra-light: 30% absorb
 * light: 40% absorb
 * medium: 55% absorb
 * medium-heavy: 70% absorb
 * heavy: 85% absorb
 * ultra-heavy: 105% absorb

Weapons should fit this framework, so a weapon that targets ulta-light should have 0% penetration, while one that targets medium should have 30% penetration.

Other outfits should _not_ use `absorb`, and instead reduce `damage_taken` when applicable.

### Variants

* Shadow + RS cargo => baseline
* S&K => +4/2, +5/3, +6/4
* Unicorp => -2/1, -3/2, -4/3
* Cargo => -4/2, -5/3, -6/4
