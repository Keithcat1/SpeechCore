#pragma once
#include <Windows.h>
#include "SCDriver.h"
#include "../ThirdParty/nvdaController.h"
#ifdef _WIN64
#define NVDA_MODULE L"nvdaControllerClient64.dll"
#else
#define NVDA_MODULE L"nvdaControllerClient32.dll"
#endif
#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
typedef error_status_t(__stdcall* NvdaController_testIfRunning_t)(void);
typedef error_status_t(__stdcall* NvdaController_speakText_t)(const wchar_t* text);
typedef error_status_t(__stdcall* NvdaController_cancelSpeech_t)(void);
#endif

class ScreenReaderNVDA : public ScreenReader {
private:
	bool loaded;
	bool Is_Active;
#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
	HMODULE module;
	NvdaController_testIfRunning_t nvdaController_testIfRunning_fn;
	NvdaController_speakText_t nvdaController_speakText_fn;
	NvdaController_cancelSpeech_t nvdaController_cancelSpeech_fn;
	#endif
	// if we're statically linking, these directly call NVDA controller. If we're not, this tries to call the loaded DLL and returns false if not loaded
	bool Controller_testIfRunning();
	bool Controller_speakText(const wchar_t* text);
	bool Controller_cancelSpeech();

public:
	ScreenReaderNVDA();
	~ScreenReaderNVDA();
	void init() override;
	void release() override;
	bool is_speaking() { return false; }
	bool is_running() override;
	bool speak_text(const wchar_t* text, bool interrupt = false) override;
	bool stop_speech() override;
};