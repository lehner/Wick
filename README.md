# Wick
Wick contractor

# Example run
```
./wick test/2piI1_P000p100p900.op test/2piI1_vec.op --replace_right t t0 --replace_right local "local" --replace_left local "" --avoid_source t --replace_left "mu" 5

# ./wick test/2piI1_P000p100p900.op test/2piI1_vec.op --replace_right t t0 --replace_right local local --replace_left local  --avoid_source t --replace_left mu 5
#
# Parsed test/2piI1_P000p100p900.op: 20 commands
# Parsed test/2piI1_vec.op: 10 commands
#
# 8 term(s) before simplification
# 2 term(s) after simplification
#

FACTOR -1 0
BEGINTRACE
GAMMA 5
MOM p_100_type_p0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_n0 t
LIGHT_LGAMMA_LIGHT t t0 mu t
ENDTRACE

FACTOR 1 -0
BEGINTRACE
GAMMA 5
MOM p_100_type_n0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_p0 t
LIGHT_LGAMMA_LIGHT t t0 mu t
ENDTRACE
```
