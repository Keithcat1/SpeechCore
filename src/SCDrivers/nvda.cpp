#include "nvda.h"

ScreenReaderNVDA::ScreenReaderNVDA() :
	#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
	ScreenReader(L"NVDA", SC_HAS_SPEECH),module(NULL),
nvdaController_testIfRunning_fn(NULL),
nvdaController_speakText_fn(NULL),
nvdaController_cancelSpeech_fn(NULL),
loaded(false), Is_Active(false) {
}
	#else
	ScreenReader(L"NVDA", SC_HAS_SPEECH),
loaded(false), Is_Active(false) {
}
	#endif
ScreenReaderNVDA::~ScreenReaderNVDA() {
	this->release();
	}
	void ScreenReaderNVDA::init() {
		#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
		if (!this->loaded) {
			this->module = LoadLibrary(NVDA_MODULE);
			if (this->module) {
				nvdaController_testIfRunning_fn = (NvdaController_testIfRunning_t)GetProcAddress(this->module, "nvdaController_testIfRunning");
				nvdaController_speakText_fn = (NvdaController_speakText_t)GetProcAddress(this->module, "nvdaController_speakText");
				nvdaController_cancelSpeech_fn = (NvdaController_cancelSpeech_t)GetProcAddress(this ->module, "nvdaController_cancelSpeech");
				this->loaded = true;
}
			if (this->loaded && !this->Is_Active) {
				this->is_running();
			}
		}
		#else
		this->loaded = true;
		if(!this->Is_Active) {
				this->is_running();
		}
		#endif
	}
	void ScreenReaderNVDA::release() {
		#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
		if (this->module) {
			this->loaded = false;
			FreeLibrary(this->module);
			this->module = NULL;
		}
		#endif
		this->Is_Active = false;
	}
	bool ScreenReaderNVDA::is_running() {
	return this->Controller_testIfRunning();
	}
	bool ScreenReaderNVDA::speak_text(const wchar_t* text,bool interrupt) {
	if(interrupt && !this->Controller_cancelSpeech()) {
		return false;
	}
	auto state = this->Controller_speakText(text);
	return state;
	}
	bool ScreenReaderNVDA::stop_speech() {
	return this->Controller_cancelSpeech();
}
#ifndef SPEECH_CORE_STATIC_SCREEN_READER_APIS
bool ScreenReaderNVDA::Controller_testIfRunning() {
	if (this->module) {
		return (nvdaController_testIfRunning_fn() == 0) ? true : false;
	}
	return false;
}

	bool ScreenReaderNVDA::Controller_speakText(const wchar_t* text) {
if (this->module) {
	auto state = nvdaController_speakText_fn(text);
	return (state == 0) ? true : false;
		}
		return false;
	}


bool ScreenReaderNVDA::Controller_cancelSpeech() {
	if (this->module) {
		return (nvdaController_cancelSpeech_fn() == 0) ? true : false;
	}
	return false;
}


#else
bool ScreenReaderNVDA::Controller_testIfRunning() {
	return (nvdaController_testIfRunning() == 0) ? true : false;
}

	bool ScreenReaderNVDA::Controller_speakText(const wchar_t* text) {
	auto state = nvdaController_speakText(text);
	return (state == 0) ? true : false;
	}

bool ScreenReaderNVDA::Controller_cancelSpeech() {
	return (nvdaController_cancelSpeech() == 0) ? true : false;
}

#endif