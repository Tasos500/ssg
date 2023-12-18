/*
 *   Sound interface
 *
 */

#include "game/snd.h"
#include "game/enum_flags.h"
#include "platform/snd_backend.h"
#include <assert.h>

static enum class SND_SYS {
	_HAS_BITFLAG_OPERATORS,
	NOTHING = 0x0,
	SYSTEM = 0x1,
	SE = 0x2,
} Initialized;

bool Snd_SystemInit(void)
{
	if(Initialized & SND_SYS::SYSTEM) {
		return true;
	}
	assert(Initialized == SND_SYS::NOTHING);
	if(!SndBackend_Init()) {
		return false;
	}
	Initialized |= SND_SYS::SYSTEM;
	return true;
}

bool Snd_SubsystemInit(SND_SYS sys, bool (&SubsystemInit)(void))
{
	if(Initialized & sys) {
		return true;
	} else if(!Snd_SystemInit() || !SubsystemInit()) {
		return false;
	}
	Initialized |= sys;
	return true;
}

void Snd_Cleanup(SND_SYS sys)
{
	auto cleanup_sys = [](SND_SYS should, SND_SYS sys, void (&cleanup)(void)) {
		if((should & sys) && (Initialized & sys)) {
			cleanup();
			Initialized &= ~sys;
		}
	};

	cleanup_sys(sys, SND_SYS::SE, SndBackend_SECleanup);

	// Silly double negation to work around C26813...
	if(~Initialized == ~SND_SYS::SYSTEM) {
		SndBackend_Cleanup();
		Initialized = SND_SYS::NOTHING;
	}
}

void Snd_Cleanup(void)
{
	Snd_Cleanup(SND_SYS::SE);
}

bool Snd_SEInit(void)
{
	return Snd_SubsystemInit(SND_SYS::SE, SndBackend_SEInit);
}

void Snd_SECleanup(void)
{
	Snd_Cleanup(SND_SYS::SE);
}

bool Snd_SELoad(BYTE_BUFFER_OWNED buffer, uint8_t id, SND_INSTANCE_ID max)
{
	return SndBackend_SELoad(std::move(buffer), id, max);
}

void Snd_SEPlay(uint8_t id, int x, bool loop)
{
	return SndBackend_SEPlay(id, x, loop);
}

void Snd_SEStop(uint8_t id)
{
	return SndBackend_SEStop(id);
}

void Snd_SEStopAll(void)
{
	for(auto i = 0; i < SND_OBJ_MAX; i++) {
		SndBackend_SEStop(i);
	}
}
