// PORTED BUT NOT INCLUDED (exist on disk, still using D233 incbin blob):
//   233_19_CS_LoadBossCallback.c     — CS_LoadBossCallback
//   233_20_CS_LoadBoss.c             — CS_LoadBoss
//   233_21_CS_Camera_ThTick_Boss.c   — CS_Camera_ThTick_Boss
//   233_22_CS_Camera_BoolGotoBoss.c  — CS_Camera_BoolGotoBoss
//   233_27_CS_Podium_Prize_ThTick2.c — CS_Podium_Prize_ThTick2
//   233_31_CS_Podium_Stand_ThTick.c  — CS_Podium_Stand_ThTick
//   233_32_CS_Podium_Stand_Init.c    — CS_Podium_Stand_Init
//   233_33_CS_Podium_FullScene_Init.c — CS_Podium_FullScene_Init (+ fwd decls for CS_Podium_Prize_Init, CS_Camera_ThTick_Podium)
//   233_36_CS_BoxScene_InstanceSplitLines.c — CS_BoxScene_InstanceSplitLines
//
// NOT PORTED (still in D233 incbin blob):
//   CS_Thread_Particles              (0x800abdd4) — particle effects in cutscenes
//   CS_Thread_MoveOnPath             (0x800ade8c) — thread path-following movement
//   CS_Thread_AnimateScale           (0x800ae2b8) — scale animation
//   CS_Thread_InterpolateFramesMS    (0x800ae318) — frame interpolation
//   CS_Camera_ThTick_Podium          (0x800aedf8) — camera for podium cutscenes
//   CS_Podium_Prize_Spin             (0x800af7c0) — prize spinning animation
//   CS_Podium_Prize_ThTick3          (0x800af994) — podium prize tick phase 3
//   CS_Podium_Prize_ThTick1          (0x800afcc4) — podium prize tick phase 1
//   CS_Podium_Prize_Init             (0x800afe90) — prize initialization
//   CS_Thread_LInB                   (0x800b06ac) — thread linear interpolation
//   CS_Garage_GetMenuPtr             (0x800b854c) — returns garage menu pointer
