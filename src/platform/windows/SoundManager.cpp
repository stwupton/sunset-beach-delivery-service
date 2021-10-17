#pragma once
#include <xaudio2.h>

class SoundManager
{
private:
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
public:
	HRESULT Initialise()
	{
		pXAudio2 = nullptr;
		HRESULT hr;

		if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
			return hr;

		pMasterVoice = nullptr;
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
			return hr;

		return 0;
	}

	void produceSineWave()
	{
		// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1386r0.pdf

		//using namespace std::experimental::audio;
		//auto d = get_default_output_device();
		//const double frequency_hz = 440.0;
		//const double delta = 2.0 * M_PI * frequency_hz / d.get_sample_rate();
		//double phase = 0;
		//d.connect([=](device&, buffer_list& bl) mutable {
		//	for (auto& buffer : bl.output_buffers()) {
		//		for (auto& frame : buffer.samples()) {
		//			auto next_sample =
		//				static_cast<device::sample_type>(std::sin(phase));
		//			phase += delta;
		//			for (auto& sample : frame)
		//				sample = next_sample;
		//		}
		//	}
		//	});
		//device.start();
		//while (true); // Spin forever
	}
};
