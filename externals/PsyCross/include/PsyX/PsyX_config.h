#ifndef PSYX_CONFIG_H
#define PSYX_CONFIG_H

// PGXP-Z: Precise GTE transform pipeline with Z-depth and widescreen fixes.
// CTR-native keeps retail-shaped GPU primitive tags, so PGXP is disabled.
#ifndef USE_PGXP
#define USE_PGXP 0
#endif

#endif // PSYX_CONFIG_H
