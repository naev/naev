Damage types follow roughly an Lp with p=1.5 norm. Thus a damage type can give something like below:
* shield dmg / armour dmg
* 100% / 100%
* 159% / 0%
* 150% / 30%
* 125% / 75%
* 110% / 90%

To compute you can use the following python code:

```python
import math
other = lambda x,p: math.pow( (1**p+1**p) - x**p, 1/p )
print( other( 1, 1.5 ) ) # gives 1
print( other( 0, 1.5 ) ) # gives 1.5874010519681994
print( other( 1.5, 1.5 ) ) # gives 0.29825195142778593
```
