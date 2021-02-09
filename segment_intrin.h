#pragma once
#include "hv_types.hpp"

extern "C" u16 readfs(void);
extern "C" u16 readgs(void);
extern "C" u16 reades(void);
extern "C" u16 readds(void);
extern "C" u16 readss(void);
extern "C" u16 readcs(void);
extern "C" u16 readtr(void);
extern "C" u16 readldt(void);