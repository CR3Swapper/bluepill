#pragma once
#include "hv_types.hpp"

extern "C" u64 readfs(void);
extern "C" u64 readgs(void);
extern "C" u64 reades(void);
extern "C" u64 readds(void);
extern "C" u64 readss(void);
extern "C" u64 readcs(void);