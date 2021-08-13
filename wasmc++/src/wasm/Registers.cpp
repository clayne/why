#include "parser/StringSet.h"
#include "wasm/Registers.h"

namespace Wasmc {
	std::map<const std::string *, int> registerMap {
		{StringSet::intern("$0"),    0},
		{StringSet::intern("$g"),    1},
		{StringSet::intern("$sp"),   2},
		{StringSet::intern("$fp"),   3},
		{StringSet::intern("$rt"),   4},
		{StringSet::intern("$lo"),   5},
		{StringSet::intern("$hi"),   6},
		{StringSet::intern("$r0"),   7},
		{StringSet::intern("$r1"),   8},
		{StringSet::intern("$r2"),   9},
		{StringSet::intern("$r3"),  10},
		{StringSet::intern("$r4"),  11},
		{StringSet::intern("$r5"),  12},
		{StringSet::intern("$r6"),  13},
		{StringSet::intern("$r7"),  14},
		{StringSet::intern("$r8"),  15},
		{StringSet::intern("$r9"),  16},
		{StringSet::intern("$ra"),  17},
		{StringSet::intern("$rb"),  18},
		{StringSet::intern("$rc"),  19},
		{StringSet::intern("$rd"),  20},
		{StringSet::intern("$re"),  21},
		{StringSet::intern("$rf"),  22},
		{StringSet::intern("$a0"),  23},
		{StringSet::intern("$a1"),  24},
		{StringSet::intern("$a2"),  25},
		{StringSet::intern("$a3"),  26},
		{StringSet::intern("$a4"),  27},
		{StringSet::intern("$a5"),  28},
		{StringSet::intern("$a6"),  29},
		{StringSet::intern("$a7"),  30},
		{StringSet::intern("$a8"),  31},
		{StringSet::intern("$a9"),  32},
		{StringSet::intern("$aa"),  33},
		{StringSet::intern("$ab"),  34},
		{StringSet::intern("$ac"),  35},
		{StringSet::intern("$ad"),  36},
		{StringSet::intern("$ae"),  37},
		{StringSet::intern("$af"),  38},
		{StringSet::intern("$t0"),  39},
		{StringSet::intern("$t1"),  40},
		{StringSet::intern("$t2"),  41},
		{StringSet::intern("$t3"),  42},
		{StringSet::intern("$t4"),  43},
		{StringSet::intern("$t5"),  44},
		{StringSet::intern("$t6"),  45},
		{StringSet::intern("$t7"),  46},
		{StringSet::intern("$t8"),  47},
		{StringSet::intern("$t9"),  48},
		{StringSet::intern("$ta"),  49},
		{StringSet::intern("$tb"),  50},
		{StringSet::intern("$tc"),  51},
		{StringSet::intern("$td"),  52},
		{StringSet::intern("$te"),  53},
		{StringSet::intern("$tf"),  54},
		{StringSet::intern("$t10"), 55},
		{StringSet::intern("$t11"), 56},
		{StringSet::intern("$t12"), 57},
		{StringSet::intern("$t13"), 58},
		{StringSet::intern("$t14"), 59},
		{StringSet::intern("$t15"), 60},
		{StringSet::intern("$t16"), 61},
		{StringSet::intern("$s0"),  62},
		{StringSet::intern("$s1"),  63},
		{StringSet::intern("$s2"),  64},
		{StringSet::intern("$s3"),  65},
		{StringSet::intern("$s4"),  66},
		{StringSet::intern("$s5"),  67},
		{StringSet::intern("$s6"),  68},
		{StringSet::intern("$s7"),  69},
		{StringSet::intern("$s8"),  70},
		{StringSet::intern("$s9"),  71},
		{StringSet::intern("$sa"),  72},
		{StringSet::intern("$sb"),  73},
		{StringSet::intern("$sc"),  74},
		{StringSet::intern("$sd"),  75},
		{StringSet::intern("$se"),  76},
		{StringSet::intern("$sf"),  77},
		{StringSet::intern("$s10"), 78},
		{StringSet::intern("$s11"), 79},
		{StringSet::intern("$s12"), 80},
		{StringSet::intern("$s13"), 81},
		{StringSet::intern("$s14"), 82},
		{StringSet::intern("$s15"), 83},
		{StringSet::intern("$s16"), 84},
		{StringSet::intern("$k0"),  85},
		{StringSet::intern("$k1"),  86},
		{StringSet::intern("$k2"),  87},
		{StringSet::intern("$k3"),  88},
		{StringSet::intern("$k4"),  89},
		{StringSet::intern("$k5"),  90},
		{StringSet::intern("$k6"),  91},
		{StringSet::intern("$k7"),  92},
		{StringSet::intern("$k8"),  93},
		{StringSet::intern("$k9"),  94},
		{StringSet::intern("$ka"),  95},
		{StringSet::intern("$kb"),  96},
		{StringSet::intern("$kc"),  97},
		{StringSet::intern("$kd"),  98},
		{StringSet::intern("$ke"),  99},
		{StringSet::intern("$kf"), 100},
		{StringSet::intern("$st"), 101},
		{StringSet::intern("$m0"), 102},
		{StringSet::intern("$m1"), 103},
		{StringSet::intern("$m2"), 104},
		{StringSet::intern("$m3"), 105},
		{StringSet::intern("$m4"), 106},
		{StringSet::intern("$m5"), 107},
		{StringSet::intern("$m6"), 108},
		{StringSet::intern("$m7"), 109},
		{StringSet::intern("$m8"), 110},
		{StringSet::intern("$m9"), 111},
		{StringSet::intern("$ma"), 112},
		{StringSet::intern("$mb"), 113},
		{StringSet::intern("$mc"), 114},
		{StringSet::intern("$md"), 115},
		{StringSet::intern("$me"), 116},
		{StringSet::intern("$mf"), 117},
		{StringSet::intern("$f0"), 118},
		{StringSet::intern("$f1"), 119},
		{StringSet::intern("$f2"), 120},
		{StringSet::intern("$f3"), 121},
		{StringSet::intern("$e0"), 122},
		{StringSet::intern("$e1"), 123},
		{StringSet::intern("$e2"), 124},
		{StringSet::intern("$e3"), 125},
		{StringSet::intern("$e4"), 126},
		{StringSet::intern("$e5"), 127},
	};

	std::array<const std::string *, 128> registerArray {
		StringSet::intern("$0"),   StringSet::intern("$g"),   StringSet::intern("$sp"),  StringSet::intern("$fp"),
		StringSet::intern("$rt"),  StringSet::intern("$lo"),  StringSet::intern("$hi"),  StringSet::intern("$r0"),
		StringSet::intern("$r1"),  StringSet::intern("$r2"),  StringSet::intern("$r3"),  StringSet::intern("$r4"),
		StringSet::intern("$r5"),  StringSet::intern("$r6"),  StringSet::intern("$r7"),  StringSet::intern("$r8"),
		StringSet::intern("$r9"),  StringSet::intern("$ra"),  StringSet::intern("$rb"),  StringSet::intern("$rc"),
		StringSet::intern("$rd"),  StringSet::intern("$re"),  StringSet::intern("$rf"),  StringSet::intern("$a0"),
		StringSet::intern("$a1"),  StringSet::intern("$a2"),  StringSet::intern("$a3"),  StringSet::intern("$a4"),
		StringSet::intern("$a5"),  StringSet::intern("$a6"),  StringSet::intern("$a7"),  StringSet::intern("$a8"),
		StringSet::intern("$a9"),  StringSet::intern("$aa"),  StringSet::intern("$ab"),  StringSet::intern("$ac"),
		StringSet::intern("$ad"),  StringSet::intern("$ae"),  StringSet::intern("$af"),  StringSet::intern("$t0"),
		StringSet::intern("$t1"),  StringSet::intern("$t2"),  StringSet::intern("$t3"),  StringSet::intern("$t4"),
		StringSet::intern("$t5"),  StringSet::intern("$t6"),  StringSet::intern("$t7"),  StringSet::intern("$t8"),
		StringSet::intern("$t9"),  StringSet::intern("$ta"),  StringSet::intern("$tb"),  StringSet::intern("$tc"),
		StringSet::intern("$td"),  StringSet::intern("$te"),  StringSet::intern("$tf"),  StringSet::intern("$t10"),
		StringSet::intern("$t11"), StringSet::intern("$t12"), StringSet::intern("$t13"), StringSet::intern("$t14"),
		StringSet::intern("$t15"), StringSet::intern("$t16"), StringSet::intern("$s0"),  StringSet::intern("$s1"),
		StringSet::intern("$s2"),  StringSet::intern("$s3"),  StringSet::intern("$s4"),  StringSet::intern("$s5"),
		StringSet::intern("$s6"),  StringSet::intern("$s7"),  StringSet::intern("$s8"),  StringSet::intern("$s9"),
		StringSet::intern("$sa"),  StringSet::intern("$sb"),  StringSet::intern("$sc"),  StringSet::intern("$sd"),
		StringSet::intern("$se"),  StringSet::intern("$sf"),  StringSet::intern("$s10"), StringSet::intern("$s11"),
		StringSet::intern("$s12"), StringSet::intern("$s13"), StringSet::intern("$s14"), StringSet::intern("$s15"),
		StringSet::intern("$s16"), StringSet::intern("$k0"),  StringSet::intern("$k1"),  StringSet::intern("$k2"),
		StringSet::intern("$k3"),  StringSet::intern("$k4"),  StringSet::intern("$k5"),  StringSet::intern("$k6"),
		StringSet::intern("$k7"),  StringSet::intern("$k8"),  StringSet::intern("$k9"),  StringSet::intern("$ka"),
		StringSet::intern("$kb"),  StringSet::intern("$kc"),  StringSet::intern("$kd"),  StringSet::intern("$ke"),
		StringSet::intern("$kf"),  StringSet::intern("$st"),  StringSet::intern("$m0"),  StringSet::intern("$m1"),
		StringSet::intern("$m2"),  StringSet::intern("$m3"),  StringSet::intern("$m4"),  StringSet::intern("$m5"),
		StringSet::intern("$m6"),  StringSet::intern("$m7"),  StringSet::intern("$m8"),  StringSet::intern("$m9"),
		StringSet::intern("$ma"),  StringSet::intern("$mb"),  StringSet::intern("$mc"),  StringSet::intern("$md"),
		StringSet::intern("$me"),  StringSet::intern("$mf"),  StringSet::intern("$f0"),  StringSet::intern("$f1"),
		StringSet::intern("$f2"),  StringSet::intern("$f3"),  StringSet::intern("$e0"),  StringSet::intern("$e1"),
		StringSet::intern("$e2"),  StringSet::intern("$e3"),  StringSet::intern("$e4"),  StringSet::intern("$e5"),
	};
}
