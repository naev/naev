# Rarity

Rarity begins at 0 for normal items. It is modified based on the following rules:

1. Items that are acquired conditionally (such as reputation limits) get +1 rarity.
1. Items that are sold in very specific spobs or not available more generally get +1 rarity.
1. Easily unlocked or hard to get models get +2 rarity (e.g., Thurion Ships).
1. Hard to unlock or very hard to get models get +3 rarity.
1. Extremely hard to acquire models get +4 rarity.
1. Ship Variants get +2 rarity, unless unlocked or hard to get, then it is +1.

## Examples

* `Za'lek Sting` gets a rarity of 1 as it is conditional.
* `Koala Armoured` gets a rarity of 2 as it is a normal variant.
* `Za'lek Sting Type II` gets a rarity of 3 as it is conditional (+1) and a variant (+2).
* `Thurion Perspicacity` gets a rarity of 3 as it is unlocked (+2) and conditional (+1).
* `Thurion Perspicacity Beta` gets a rarity of 4 as it is unlocked (+2), conditional (+1), and a variant (+1 as it is already unlocked).
