#pragma once
#include <xaudio2.h>

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif


/*
XAudio2 Features:

	DSP Effects and Per Voice Filtering:
		Digital Signal Processing (DSP) effects are the pixel shaders of audio. They handle everything from transforming a sound—turning a pig squeal
		into a low, scary monster sound—to placing sounds in the game environment using reverb and occlusion or obstruction filtering.
		XAudio2 provides a flexible and powerful DSP framework. It also provides a built-in filter on every voice, for efficient low/high/band-pass
		filtering effects.
		See https://docs.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-audio-effects

	Submixing:
		Submixing combines several sounds into a single audio stream—for example, an engine sound made up of composite parts,
		all of which are playing simultaneously. Also, you can use submixing to process and combine similar parts of a game.
		For example, you could combine all game sound effects to allow a user volume setting to be applied while a separate setting controls
		music volume. Combined with DSP, submixing provides the type of data routing and processing necessary for today's games.
		XAudio2 allows for arbitrary levels of submixing, enabling the creation of complex sounds and game mixes.
		See https://docs.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-audio-graph

	Compressed Audio Support:
		One of the major feature requests for DirectSound has been for compressed audio support. XAudio2 supports compressed
		formats—ADPCM—natively with run-time decompression.

	Enhanced Multichannel and Surround Sound Support:
		Multichannel, 3D, and surround sound support is expanded. 3D and surround sound are now much more flexible and transparent.
		XAudio2 removes the 6-channel limit on multichannel sounds, and supports multichannel audio on any multichannel-capable audio card.
		The card does not need to be hardware-accelerat

	Multirate Processing:
		To help minimize CPU usage, XAudio2 provides the technology to create multiple, low-rate audio processing graphs.
		This can significantly reduce CPU usage by allowing a game to process audio at the rate of the source material if
		the rate is less than 48 kHz.

	Nonblocking API Model:
		With few exceptions, an XAudio2 method call will not block the audio processing engine. This means that a client can safely
		make a set of method calls at any time without blocking on long-running calls causing delays.
		The exceptions are the IXAudio2Voice::DestroyVoice method (which may block the engine until the voice being destroyed is
		finished processing) and the methods that terminate the audio thread: IXAudio2::StopEngine and IXAudio2::Release.
		Note that while XAudio2 method calls will not block the audio processing engine, the XAudio2 methods contain critical
		sections and may themselves become blocked in some circumstances.
*/

//namespace Assets
//{
//	const TCHAR* left = L"assets/music/Left.wav";
//	const TCHAR* right = L"assets/music/Right.wav";
//	const TCHAR* stereo = L"assets/music/Stero.wav";
//};
//namespace Assets
//{
//	const wchar_t* left = L"assets/music/Left.wav";
//	const wchar_t* right = L"assets/music/Right.wav";
//	const wchar_t* stereo = L"assets/music/Stero.wav";
//};



//const TCHAR* fileNames[] = {
//	Assets::left,
//	Assets::right,
//	Assets::stereo
//};

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;
	VoiceCallback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
	~VoiceCallback()
	{
		CloseHandle(hBufferEndEvent);
	}

	//Called when the voice has just finished playing a contiguous audio stream.
	void OnStreamEnd()
	{
		SetEvent(hBufferEndEvent);
	}

	//Unused methods are stubs
	void OnVoiceProcessingPassEnd() { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {    }
	void OnBufferEnd(void* pBufferContext) { }
	void OnBufferStart(void* pBufferContext) {    }
	void OnLoopEnd(void* pBufferContext) {    }
	void OnVoiceError(void* pBufferContext, HRESULT Error) { }
};


TCHAR* waveFileNames[3] = {
	L"assets/music/Left.wav",
	L"assets/music/Right.wav",
	L"assets/music/Stereo.wav"
};
/*const wchar_t* left = L"assets/music/Left.wav";
const wchar_t* right = L"assets/music/Right.wav";
const wchar_t* stereo = L"assets/music/Stereo.wav";*/

int soundIndex = 0;

VoiceCallback* voiceCallbackPtr;
VoiceCallback voiceCallback;


class SoundManager
{
private:
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	static const int voiceBufferSize = 2;
	IXAudio2SourceVoice* voices[voiceBufferSize] = {};

	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;

		}

		return S_OK;

	}

	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

public:
	~SoundManager()
	{
		pXAudio2->Release();
	}

	HRESULT Initialise()
	{
		pXAudio2 = nullptr;
		voiceCallbackPtr = nullptr;
		HRESULT hr;

		// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2

		// This always fails???
		//// 1. Make sure you have initialized COM. For a Windows Store app, 
		//// this is done as part of initializing the Windows Runtime. Otherwise, use CoInitializeEx.
		//hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		//if (FAILED(hr))
		//	return hr;

		// 2. Use the XAudio2Create function to create an instance of the XAudio2 engine
		if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
			return hr;

		// 3. Use the CreateMasteringVoice method to create a mastering voice.
		// The mastering voices encapsulates an audio device. It is the ultimate destination for 
		// all audio that passes through an audio graph.
		pMasterVoice = nullptr;
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
			return hr;

#ifdef _DEBUG

		XAUDIO2_DEBUG_CONFIGURATION flags;
		flags.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_DETAIL;
		flags.BreakMask = XAUDIO2_LOG_ERRORS;
		flags.LogFileline = true;
		flags.LogFunctionName = true;
		flags.LogThreadID = true;
		flags.LogTiming = true;

		pXAudio2->SetDebugConfiguration(&flags);

#endif // DEBUG

		return 0;
	}

	HRESULT PlaySound(const TCHAR* strFileName)
	{

		const TCHAR* fileName;

		// Check if we have an available slot
		if (voiceCallbackPtr == nullptr)
		{
			int length = 3;
			//strcpy(fileName, fileNames[soundIndex++]);
			/*wcscpy(fileName, fileNames[soundIndex++]);
			TCHAR dest[20];
			_tcscpy_s(dest, _countof(dest), _T("Hello"));
			int length = (sizeof(fileNames) / sizeof(*fileNames));*/
			fileName = waveFileNames[soundIndex++];

			/*switch (soundIndex)
			{
			case 0:
				fileName = left;
				break;
			case 1:
				fileName = right;
				break;
			case 2:
				fileName = stereo;
				break;
			default:
				break;
			}*/

			//soundIndex++;

			if (soundIndex >= length)
			{
				soundIndex = 0;
			}
		}
		else
		{
			fileName = strFileName;
		}

		HRESULT hr = NULL;
		// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
		// 3. Locate the 'RIFF' chunk in the audio file, and check the file type.
		// Open the file
		HANDLE hFile = CreateFile(
			fileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
			return HRESULT_FROM_WIN32(GetLastError());

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
			return S_FALSE;

		// 4. Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

		// 5. Locate the 'data' chunk, and read its contents into a buffer.
		//fill out the audio data buffer with the contents of the fourccDATA chunk
		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

		// Close hFile stream now that we've loaded all the data into memory
		CloseHandle(hFile);

		// 6. Populate an XAUDIO2_BUFFER structure.
		buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		bool canPlaySound = false;

		int freeVoiceBufferIndex = 0;

		for (;freeVoiceBufferIndex < voiceBufferSize; freeVoiceBufferIndex++)
		{
			if (voices[freeVoiceBufferIndex] != nullptr)
			{
				//XAUDIO2_VOICE_STATE state;
				//SDL_assert(!_this->enabled);  /* flag that stops playing. */
				//source->Discontinuity();
				//source->GetState(&state);
				//while (state.BuffersQueued > 0) {
				//	WaitForSingleObjectEx(_this->hidden->semaphore, INFINITE, 0);
				//	source->GetState(&state);
				//}
				XAUDIO2_VOICE_STATE state;
				voices[freeVoiceBufferIndex]->GetState(&state);
				if (state.BuffersQueued <= 0)
				{
					voices[freeVoiceBufferIndex]->DestroyVoice();
					voices[freeVoiceBufferIndex] = nullptr;
					canPlaySound = true;
					break;
				}
			}
			else
			{
				canPlaySound = true;
				break;
			}
		}

		

		if (canPlaySound)
		{
			// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
			// 3. Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. 
			// The format of the voice is specified by the values set in a WAVEFORMATEX structure.
			IXAudio2SourceVoice* pSourceVoice;
			//if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx)))
			//	return hr;
			if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx,
				0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL)))
			{
				return hr;
			}
			voiceCallbackPtr = &voiceCallback;

			// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
			if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
				return hr;

			// 5. Use the Start function to start the source voice. Since all XAudio2 voices send their output to the mastering voice by default, 
			// audio from the source voice automatically makes its way to the audio device selected at initialization. 
			// In a more complicated audio graph, the source voice would have to specify the voice to which its output should be sent.
			if (FAILED(hr = pSourceVoice->Start(0)))
				return hr;

			voices[freeVoiceBufferIndex] = pSourceVoice;


			//WaitForSingleObjectEx(voiceCallback.hBufferEndEvent, INFINITE, TRUE);
		}

		/*else if (!voiceCallbackPtr->hBufferEndEvent)
		{
			voiceCallbackPtr = nullptr;
		}*/

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
