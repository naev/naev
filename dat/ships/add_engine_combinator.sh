#!/usr/bin/bash

for i in `grep -rl \"engines_secondary\" | grep .xml`; do
   sed -i '/engines_combinator/d' $i
done

for i in `grep -rl \"engines_secondary\" | grep .xml`; do
   sed -i '/engines_secondary.*small/i\  <structure prop="engines_combinator" name="engines_combinator" size="small">Engines Combinator</structure>' $i
   sed -i '/engines_secondary.*medium/i\  <structure prop="engines_combinator" name="engines_combinator" size="medium">Engines Combinator</structure>' $i
   sed -i '/engines_secondary.*large/i\  <structure prop="engines_combinator" name="engines_combinator" size="large">Engines Combinator</structure>' $i
   sed -i '/combinator/!{x;1!p;$!d;g;q}' $i
done

