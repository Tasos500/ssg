/*
 *   Format-independent background music interface
 *
 */

#include "game/bgm.h"
#include "game/bgm_track.h"
#include "game/midi.h"
#include "game/snd.h"
#include "game/string_format.h"
#include "game/volume.h"
#include "platform/file.h"
#include "platform/midi_backend.h"
#include "platform/path.h"
#include "platform/snd_backend.h"
#include <algorithm>

using namespace std::chrono_literals;

static constexpr std::u8string_view BGM_ROOT = u8"bgm/";
static constexpr std::u8string_view EXT_MID = u8".mid";

// State
// -----

uint8_t BGM_Tempo_Num = BGM_TEMPO_DENOM;

static bool Enabled = false;
static bool Playing = false;
static unsigned int LoadedNum = 0; // 0 = nothing

static std::u8string PackPath;
static std::shared_ptr<BGM::TRACK> Waveform; // nullptr = playing MIDI
// -----

// External dependencies
// ---------------------

const uint8_t& Mid_TempoNum = BGM_Tempo_Num;
const uint8_t& Mid_TempoDenom = BGM_TEMPO_DENOM;
// ---------------------

bool BGM_Init(void)
{
	BGM_SetTempo(0);
	Enabled = (MidBackend_Init() | Snd_BGMInit());
	return Enabled;
}

void BGM_Cleanup(void)
{
	BGM_Stop();
	MidBackend_Cleanup();
	Snd_BGMCleanup();
	Enabled = false;
}

bool BGM_Enabled(void)
{
	return Enabled;
}

std::chrono::duration<int32_t, std::milli> BGM_PlayTime(void)
{
	if(Waveform) {
		return SndBackend_BGMPlayTime();
	}
	return Mid_PlayTime.realtime;
}

Narrow::string_view BGM_Title(void)
{
	auto ret = Mid_GetTitle();

	// pbg bug: Four of the original track titles start with leading fullwidth
	// spaces:
	//
	// 	#04: "　　　幻想帝都"
	// 	#07: "　　天空アーミー"
	// 	#11: "　魔法少女十字軍"
	// 	#16: "　　シルクロードアリス"
	//
	// This looks like it was done on purpose to center the titles within the
	// 216 maximum pixels that the original code designated for the in-game
	// animation. However:
	//
	// • None of those actually has the correct amount of spaces that would
	//   have been required for exact centering.
	// • If pbg intended to center all the tracks, there should have been
	//   leading whitespace in 14 of the track titles, and not just in 4.
	//
	// Since the in-game animation code does clearly intend these titles to be
	// right-aligned, it makes more sense to just remove all leading
	// whitespace. Doing this here will also benefit the Music Room.
	const auto trim_leading = [](auto& str, Narrow::string_view prefix) {
		const auto ret = str.starts_with(prefix);
		if(ret) {
			str.remove_prefix(prefix.size());
		}
		return ret;
	};
	while(trim_leading(ret, " ") || trim_leading(ret, "\x81\x40")) {
	};

	// The original MIDI sequence titles also come with lots of *trailing*
	// whitespace. Adding 1 also turns `npos` to 0.
	ret = ret.substr(0, (ret.find_last_not_of(" ") + 1));

	return ret;
}

bool BGM_ChangeMIDIDevice(int8_t direction)
{
	// 各関数に合わせて停止処理を行う //
	Mid_Stop();

	const auto ret = MidBackend_DeviceChange(direction);
	if(ret && Playing && !Waveform) {
		Mid_Play();
	}
	return ret;
}

static bool BGM_Load(unsigned int id)
{
	if(!PackPath.empty()) {
		const auto prefix_len = PackPath.size();
		StringCatNum<2>((id + 1), PackPath);

		// Try loading a waveform track
		bool waveform_new = false;
		if(Waveform = BGM::TrackOpen(PackPath)) {
			if(SndBackend_BGMLoad(Waveform)) {
				waveform_new = true;
			}
		}

		// Try loading a MIDI
		bool mid_new = false;
		if(!mid_new) {
			PackPath += EXT_MID;
			mid_new = BGM_MidLoadBuffer(FileLoad(PackPath.c_str()));
		}

		PackPath.resize(prefix_len);
		if(waveform_new || mid_new) {
			return true;
		}
	}
	return BGM_MidLoadOriginal(id);
}

bool BGM_Switch(unsigned int id)
{
	if(!Enabled) {
		return false;
	}
	BGM_Stop();
	Waveform = nullptr;
	const auto ret = BGM_Load(id);
	if(ret) {
		LoadedNum = (id + 1);
		BGM_Play();
	}
	return ret;
}

void BGM_Play(void)
{
	if(Waveform) {
		SndBackend_BGMPlay();
	} else {
		Mid_Play();
	}
	Playing = true;
}

void BGM_Stop(void)
{
	if(Waveform) {
		SndBackend_BGMStop();
	} else {
		Mid_Stop();
	}
	Playing = false;
}

void BGM_Pause(void)
{
	// Waveform tracks are automatically paused as part of the Snd subsystem
	// once the game window loses focus. We might need independent pausing in
	// the future?
	if(!Waveform) {
		Mid_Pause();
	}
}

void BGM_Resume(void)
{
	// Same as for pausing; /s/paus/resum/g, /s/loses/regains/
	if(!Waveform) {
		Mid_Resume();
	}
}

void BGM_FadeOut(uint8_t speed)
{
	// pbg quirk: The original game always reduced the volume by 1 on the first
	// call to the MIDI FadeIO() method after the start of the fade. This
	// allowed you to hold the fade button in the Music Room for a faster
	// fade-out.
	const auto volume_start = (Mid_GetFadeVolume() - 1);

	const auto duration = (
		10ms * VOLUME_MAX * ((((256 - speed) * 4) / (VOLUME_MAX + 1)) + 1)
	);
	Mid_FadeOut(volume_start, duration);
}

int8_t BGM_GetTempo(void)
{
	return (BGM_Tempo_Num - BGM_TEMPO_DENOM);
}

void BGM_SetTempo(int8_t tempo)
{
	tempo = std::clamp(tempo, BGM_TEMPO_MIN, BGM_TEMPO_MAX);
	BGM_Tempo_Num = (BGM_TEMPO_DENOM + tempo);
}

void BGM_PackSet(const std::u8string_view pack)
{
	const std::u8string_view cur = PackPath;
	if(!pack.empty()) {
		const auto path_data = PathForData();
		const auto root_len = (path_data.size() + BGM_ROOT.size());
		if(
			(cur.size() > root_len) &&
			(cur.substr(root_len, pack.size()) == pack) &&
			(cur[root_len + pack.size()] == '/') // !!!
		) {
			return;
		}
		const auto pack_len = (pack.size() + 1);
		const auto file_len = (STRING_NUM_CAP<unsigned int> + EXT_MID.size());
		const auto len = (root_len + pack_len + file_len + 1);
		PackPath.resize_and_overwrite(len, [&](char8_t* buf, size_t len) {
			std::ranges::in_out_result p = { path_data.begin(), buf };
			p = std::ranges::copy(path_data, p.out);
			p = std::ranges::copy(BGM_ROOT, p.out);
			p = std::ranges::copy(pack, p.out);
			*(p.out++) = '/';
			return (p.out - buf);
		});
	} else {
		if(cur.empty()) {
			return;
		}
		PackPath.clear();
	}

	if((LoadedNum != 0) && Playing) {
		BGM_Switch(LoadedNum - 1);
	}
}
