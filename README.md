# Wick
Wick contractor

# Examples
## 2 pions I=1 and a local vector operator
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

## Two pions in I=1
```
# ./wick test/2piI1_P000p100p900.op test/2piI1_P000p100p900.op --replace_right t t0 --replace_right local  --replace_left local  --avoid_source t --replace_left mu 5 --replace_right mu 5
#
# Parsed test/2piI1_P000p100p900.op: 20 commands
# Parsed test/2piI1_P000p100p900.op: 20 commands
#
# 16 term(s) before simplification
# 6 term(s) after simplification
#

FACTOR -1 0
BEGINTRACE
GAMMA 5
MOM p_100_type_p0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_n0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_p0 t0
LIGHT t0 t0
GAMMA 5
MOM p_100_type_n0 t0
LIGHTBAR t0 t
ENDTRACE

FACTOR 1 0
BEGINTRACE
GAMMA 5
MOM p_100_type_p0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_n0 t0
LIGHTBAR t0 t
ENDTRACE
BEGINTRACE
GAMMA 5
MOM p_100_type_n0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_p0 t0
LIGHTBAR t0 t
ENDTRACE

FACTOR -1 0
BEGINTRACE
GAMMA 5
MOM p_100_type_p0 t0
LIGHTBAR t0 t
GAMMA 5
MOM p_100_type_n0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_p0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_n0 t0
LIGHT t0 t0
ENDTRACE

FACTOR 1 -0
BEGINTRACE
GAMMA 5
MOM p_100_type_n0 t0
LIGHT t0 t0
GAMMA 5
MOM p_100_type_p0 t0
LIGHTBAR t0 t
GAMMA 5
MOM p_100_type_p0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_n0 t
LIGHT t t0
ENDTRACE

FACTOR -1 0
BEGINTRACE
GAMMA 5
MOM p_100_type_p0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_p0 t0
LIGHTBAR t0 t
ENDTRACE
BEGINTRACE
GAMMA 5
MOM p_100_type_n0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_n0 t0
LIGHTBAR t0 t
ENDTRACE

FACTOR 1 -0
BEGINTRACE
GAMMA 5
MOM p_100_type_n0 t0
LIGHTBAR t0 t
GAMMA 5
MOM p_100_type_n0 t
LIGHTBAR t t
GAMMA 5
MOM p_100_type_p0 t
LIGHT t t0
GAMMA 5
MOM p_100_type_p0 t0
LIGHT t0 t0
ENDTRACE
```
