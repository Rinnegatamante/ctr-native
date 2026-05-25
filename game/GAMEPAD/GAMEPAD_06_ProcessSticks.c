#include <common.h>

void GAMEPAD_ProcessSticks(struct GamepadSystem *gGamepads)
{
	// this is missing a bunch of stuff vs ghidra
	u8 bVar1;
	u8 bVar2;
	s16 uVar3;
	int iVar4;
	int iVar7;
	s16 sVar8;

	struct GamepadBuffer *pad;
	struct ControllerPacket *packet;
	struct RacingWheelData *rwd;


	// for RWD
	int i;

	for (pad = &gGamepads->gamepad[0], rwd = &data.rwd[0], i = 0; pad < &gGamepads->gamepad[8]; pad++, rwd++, i++)
	{
		packet = pad->ptrControllerPacket;

		// wipe to default
		pad->rwd = NULL;
		pad->stickLX = 0x80;
		pad->stickLY = 0x80;
		pad->stickLX_dontUse1 = 0x80;
		pad->stickLY_dontUse1 = 0x80;
		pad->stickRX = 0x80;
		pad->stickRY = 0x80;

		// if pointer is valid
		if (packet == NULL)
			continue;

		if (packet->plugged == PLUGGED)
		{
			bVar1 = packet->controllerData;

			if ((bVar1 == ((PAD_ID_ANALOG_STICK << 4) | 3)) || (bVar1 == ((PAD_ID_ANALOG << 4) | 3)))
			{
				pad->stickLX_dontUse1 = packet->analog.leftX;
				pad->stickLY_dontUse1 = packet->analog.leftY;
				pad->stickRX = packet->analog.rightX;
				pad->stickRY = packet->analog.rightY;
			}

			else if (bVar1 == ((PAD_ID_NEGCON << 4) | 3))
			{
				if (i < 4)
				{
					// racingwheel data
					pad->rwd = rwd;
				}
				pad->stickLX_dontUse1 = packet->neGcon.twist;
			}

			else if (bVar1 == ((PAD_ID_JOGCON << 4) | 3))
			{
				if (i < 4)
				{
					// racingwheel data
					pad->rwd = rwd;
				}

				sVar8 = packet->jogcon.jog_rot;
				iVar4 = (int)sVar8;

				if (iVar4 < 0)
				{
					iVar7 = ((-10 - iVar4) - rwd->deadZone) * 8;
					if (iVar7 < 0)
						iVar7 = 0;
					if (iVar7 > 0xff)
						iVar7 = 0xff;

					sVar8 += 0x80;
					if (iVar4 < -0x80)
					{
						sVar8 = -0x80;
						sVar8 += 0x80;
					}
				}
				else
				{
					iVar7 = ((iVar4 - 10) - rwd->deadZone) * 8;
					if (iVar7 < 0)
						iVar7 = 0;
					if (iVar7 > 0xff)
						iVar7 = 0xff;

					sVar8 += 0x80;
					if (0x7f < iVar4)
					{
						sVar8 = 0x7f;
						sVar8 += 0x80;
					}
				}
				pad->unk43 = (char)iVar7;
				pad->stickLX_dontUse1 = sVar8;
			}
		}

		if ((u32)(pad->stickLX_dontUse1 - 0x80) > 0x30)
			pad->framesSinceLastInput = 0;

		if ((u32)(pad->stickLY_dontUse1 - 0x80) > 0x30)
			pad->framesSinceLastInput = 0;

		if ((u32)(pad->stickRX - 0x80) > 0x30)
			pad->framesSinceLastInput = 0;

		if ((u32)(pad->stickRY - 0x80) > 0x30)
			pad->framesSinceLastInput = 0;

		pad->stickLX = pad->stickLX_dontUse1;
		pad->stickLY = pad->stickLY_dontUse1;

		int held = pad->buttonsHeldCurrFrame;

		// In this order: Up, Down, Left, Right
		if ((held & 1) != 0)
			pad->stickLY = 0;
		if ((held & 2) != 0)
			pad->stickLY = 0xFF;
		if ((held & 4) != 0)
			pad->stickLX = 0;
		if ((held & 8) != 0)
			pad->stickLX = 0xFF;
	}
}
