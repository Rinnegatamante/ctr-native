#include "231_002_RB_Player_ModifyWumpa.c"
#include "231_003_RB_MinePool_Init.c"
#include "231_004_RB_MinePool_Remove.c"
#include "231_005_RB_MinePool_Add.c"
#include "231_006_RB_Hazard_HurtDriver.c"
#include "231_007_RB_Hazard_CollideWithDrivers.c"
#include "231_008_RB_Hazard_CollideWithBucket.c"
#include "231_009_RB_Hazard_ThCollide_Generic_Alt.c"
#include "231_010_RB_Hazard_ThCollide_Missile.c"
#include "231_011_RB_Hazard_ThCollide_Generic.c"

// TODO(aalhendi): BSP search, reflective placement, rot force helpers
// TODO(aalhendi): 012-020 are the potion / mine / TNT cluster.
#include "231_021_RB_Explosion_ThTick.c"
#include "231_022_RB_Hazard_CollLevInst.c"
#include "231_023_RB_Hazard_InterpolateValue.c"
#include "231_024_RB_MovingExplosive_ThTick.c"
#include "231_025_RB_MovingExplosive_Explode.c"

// TODO(aalhendi): unported path-node, audio recycle, and matrix-parent behavior.
// TODO(aalhendi): 026-038 are the warpball / mask / shield cluster.
#include "231_039_RB_Player_ToggleInvisible.c"
#include "231_040_RB_Player_ToggleFlicker.c"
#include "231_041_RB_RainCloud_FadeAway.c"
#include "231_042_RB_RainCloud_ThTick.c"
#include "231_043_RB_RainCloud_Init.c"

// TODO(aalhendi) Particle_FuncPtr_PotionShatter.
// TODO(aalhendi): 044
#include "231_045_RB_Explosion_InitGeneric.c"
#include "231_047_RB_Blowup_ThTick.c"

// TODO(aalhendi): 048 and 053 still need RB_Burst_CollLevInst / Teeth dependency cleanup.
#include "231_050_RB_Burst_ThTick.c"
#include "231_051_RB_Burst_CollThBucket.c"
#include "231_055_RB_GetThread_ClosestTracker.c"
#include "231_056_RB_Baron_ThTick.c"
#include "231_057_RB_Baron_LInB.c"
#include "231_058_RB_Blade_ThTick.c"
#include "231_059_RB_Blade_LInB.c"
#include "231_060_RB_Bubbles_RoosTubes.c"

// TODO(aalhendi): sound/voice paths and RB_MakeInstanceReflective.
// TODO(aalhendi): 061-077 (crate, crystal, default LevInst, and CTR letters).

// TODO(aalhendi):  RB_MakeInstanceReflective. Keep the temporary zGlobal stub until ported
// TODO(aalhendi) 231_073_RB_Default_LInB

#include "231_082_085_RB_Armadillo.c"
#include "231_086_088_RB_Fireball.c"
// TODO(aalhendi): RngDeadCoed, Vector_SpecLightNoSpin3D, sound helpers.
// TODO(aalhendi): 089-091 FlameJet
#include "231_092_094_RB_Follower.c"
#include "231_095_098_RB_Fruit.c"
#include "231_099_100_RB_Minecart.c"
#include "231_104_109_RB_Plant.c"
#include "231_110_113_RB_Seal.c"
#include "231_114_115_RB_Snowball.c"
#include "231_116_119_RB_Spider.c"
// TODO(aalhendi): 122-126 Teeth still need sound/runtime dependency cleanup.
#include "231_127_129_RB_Turtle.c"
