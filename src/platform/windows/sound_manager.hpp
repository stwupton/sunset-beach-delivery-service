#pragma once

#include <xaudio2.h>
#include <strsafe.h>
#include "platform/windows/utils.hpp"
#include "common/game_state.hpp"

#ifdef _XBOX // Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX // Little-Endian
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


void findWaveDataChunk(HANDLE fileHandle, DWORD fourcc, DWORD *chunkSize, DWORD *chunkDataPosition) {
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(fileHandle, 0, NULL, FILE_BEGIN)) {
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD readOffset = 0;

	while (hr == S_OK) {
		DWORD bytesToRead;
		if (!ReadFile(fileHandle, &chunkType, sizeof(DWORD), &bytesToRead, NULL)) {
			hr = (HRESULT_FROM_WIN32(GetLastError()));
		}
		ASSERT_HRESULT(hr)

		if (!ReadFile(fileHandle, &chunkDataSize, sizeof(DWORD), &bytesToRead, NULL)) {
			hr = (HRESULT_FROM_WIN32(GetLastError()));
		}
		ASSERT_HRESULT(hr)

		switch (chunkType) {
			case fourccRIFF:
				riffDataSize = chunkDataSize;
				chunkDataSize = 4;
				if (!ReadFile(fileHandle, &fileType, sizeof(DWORD), &bytesToRead, NULL)) {
					hr = (HRESULT_FROM_WIN32(GetLastError()));
				}
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(fileHandle, chunkDataSize, NULL, FILE_CURRENT)) {
					hr = (HRESULT_FROM_WIN32(GetLastError()));
				}
		}
		ASSERT_HRESULT(hr)

		readOffset += sizeof(DWORD) * 2;

		if (chunkType == fourcc) {
			*chunkSize = chunkDataSize;
			*chunkDataPosition = readOffset;
			return;
		}

		readOffset += chunkDataSize;

		if (bytesRead >= riffDataSize) {
			ASSERT_HRESULT(S_FALSE)
		}
	}
}

void readChunkData(HANDLE fileHandle, void *buffer, DWORD bufferSize, DWORD bufferOffset) {
	if (INVALID_SET_FILE_POINTER == SetFilePointer(fileHandle, bufferOffset, NULL, FILE_BEGIN)) {
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()));
	}

	DWORD bytesToRead;
	if (!ReadFile(fileHandle, buffer, bufferSize, &bytesToRead, NULL)) {
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()));
	}
}

// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--stream-a-sound-from-disk
// 1.Create an array of read buffers.
#define STREAMBUFFERSIZE 65536 // The size of each buffer
#define BUFFERNUM 5 // Number of buffers
BYTE streamBuffers[BUFFERNUM][STREAMBUFFERSIZE]; // Buffer array

IXAudio2 *musicXAudio2 = NULL;
IXAudio2MasteringVoice *musicMasterVoice = NULL;
int currentBufferIndex = 0;

class StreamingVoiceContext : public IXAudio2VoiceCallback {
public:
	HANDLE bufferEndEvent;
	StreamingVoiceContext() : bufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
	~StreamingVoiceContext() { CloseHandle(bufferEndEvent); }

	// Called when the voice has just finished playing a contiguous audio stream.
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnStreamEnd() override {
		SetEvent(bufferEndEvent);
	}

	// Inherited via IXAudio2VoiceCallback
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferStart(void *bufferContext) override {}
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnLoopEnd(void *bufferContext) override {}
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceError(void *bufferContext, HRESULT Error) override {}

	// Inherited via IXAudio2VoiceCallback
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassEnd(void) override {}
	virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferEnd(void *bufferContext) override {
		SetEvent(bufferEndEvent);
	}
};

void getWaveInfo(
	const TCHAR *fileName, 
	WAVEFORMATEXTENSIBLE *wfx, 
	DWORD *fileType, 
	DWORD *waveSize, 
	HANDLE *fileHandle, 
	DWORD *waveDataStartPosition
) {
	*fileHandle = CreateFile(
		fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (INVALID_HANDLE_VALUE == *fileHandle) {
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(*fileHandle, 0, NULL, FILE_BEGIN)) {
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
	}

	DWORD chunkSize;
	DWORD chunkPosition;
	// Check the file type, should be fourccWAVE or 'XWMA'
	findWaveDataChunk(*fileHandle, fourccRIFF, &chunkSize, &chunkPosition);
	readChunkData(*fileHandle, fileType, sizeof(DWORD), chunkPosition);
	if (*fileType != fourccWAVE) {
		ASSERT_HRESULT(S_FALSE)
	}

	*wfx = { 0 };

	// 4. Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	findWaveDataChunk(*fileHandle, fourccFMT, &chunkSize, &chunkPosition);
	readChunkData(*fileHandle, wfx, chunkSize, chunkPosition);

	// 5. Locate the 'data' chunk, and read its contents into a buffer.
	// Fill out the audio data buffer with the contents of the fourccDATA chunk.
	findWaveDataChunk(*fileHandle, fourccDATA, waveSize, waveDataStartPosition);
}

void getStreamingData(
	const TCHAR *fileName, 
	WAVEFORMATEXTENSIBLE *wfx, 
	DWORD *fileType, 
	DWORD *waveSize, 
	HANDLE *fileHandle, 
	DWORD *waveDataStartPosition
) {
	getWaveInfo(fileName, wfx, fileType, waveSize, fileHandle, waveDataStartPosition);
}

void getWaveData(
	const TCHAR *fileName, 
	WAVEFORMATEXTENSIBLE *wfx, 
	DWORD *fileType, 
	DWORD *waveSize, 
	BYTE **waveData
) {
	HANDLE fileHandle;
	DWORD waveDataStartPosition;

	getWaveInfo(fileName, wfx, fileType, waveSize, &fileHandle, &waveDataStartPosition);

	*waveData = new BYTE[*waveSize];
	readChunkData(fileHandle, *waveData, *waveSize, waveDataStartPosition);

	// Close fileHandle stream now that we've loaded all the data into memory
	CloseHandle(fileHandle);
}

struct StreamMusic {
	TCHAR *nextFileName;

	bool isChanging = false;
	bool isPlaying = false;
	bool isAlive = true;
	bool isLooping = false;

	void playNewFile(const TCHAR *newFileName) {
		isChanging = true;
		nextFileName = (TCHAR *)newFileName;
	}

	// Thread callback
	void streamAudioFile(LPWSTR fileName) {
		isPlaying = true;
		// If playing for first time, then clear down this flag
		isChanging = false;
		nextFileName = fileName;

		while (nextFileName != nullptr) {
			WAVEFORMATEXTENSIBLE wfx = { 0 };
			DWORD fileType, waveFileSize, waveDataStartPosition;
			fileType = waveFileSize = waveDataStartPosition = 0;
			HANDLE fileHandle = NULL;
			getStreamingData(
				nextFileName, 
				&wfx, 
				&fileType, 
				&waveFileSize, 
				&fileHandle, 
				&waveDataStartPosition
			);

			StreamingVoiceContext musicCallBack;
			IXAudio2SourceVoice *musicSourceVoice = NULL;

			// Create source sound to submit data
			HRESULT hr = musicXAudio2->CreateSourceVoice(
				&musicSourceVoice, 
				(WAVEFORMATEX*)&wfx, 
				0, 
				XAUDIO2_DEFAULT_FREQ_RATIO, 
				&musicCallBack
			); 
			ASSERT_HRESULT(hr)

			int fileSize = waveFileSize;
			int currentPos = waveDataStartPosition;

			// clear so doesn't play again
			nextFileName = nullptr;

			while (currentPos < fileSize) {
				if (isChanging || !isAlive) {
					break;
				}

				DWORD size = STREAMBUFFERSIZE;
				readChunkData(fileHandle, streamBuffers[currentBufferIndex], size, currentPos);

				// The size of the data that has been played
				currentPos += size;

				// The file data to be read will be assigned XAUDIO2_BUFFER
				XAUDIO2_BUFFER buffer = { 0 };
				buffer.AudioBytes = size;
				buffer.pAudioData = streamBuffers[currentBufferIndex];

				// Submit memory data
				hr = musicSourceVoice->SubmitSourceBuffer(&buffer);
				ASSERT_HRESULT(hr)

				//Start source sound
				hr = musicSourceVoice->Start(XAUDIO2_COMMIT_NOW);
				ASSERT_HRESULT(hr)

				XAUDIO2_VOICE_STATE state;
				musicSourceVoice->GetState(&state);
				// Do not let the audio data of the buffer be overwritten
				while (state.BuffersQueued > BUFFERNUM - 1) {
					WaitForSingleObject(musicCallBack.bufferEndEvent, INFINITE);
					musicSourceVoice->GetState(&state);
				}

				// Recycle buffer
				currentBufferIndex++;
				currentBufferIndex %= BUFFERNUM;

				// If we're looping and we've reached the end, reset and go again
				if (isLooping && currentPos >= fileSize) {
					currentPos = waveDataStartPosition;
				}
			}

			XAUDIO2_VOICE_STATE state;
			if (!isChanging) {
				// Wait for the completion of data playback in the queue, exit the thread				
				while (musicSourceVoice->GetState(&state), state.BuffersQueued > 0) {
					WaitForSingleObject(musicCallBack.bufferEndEvent, INFINITE);
				}
			}

			musicSourceVoice->DestroyVoice();
			isChanging = false;
		}

		isPlaying = false;
	}
};

StreamMusic *streamMusicData;
DWORD streamMusicThreadId;
HANDLE streamMusicThread;

DWORD WINAPI playMusicThread(LPVOID lpParam) {
	StreamMusic *streamMusic;
	streamMusic = (StreamMusic*)lpParam;
	while (streamMusic->isAlive) {
		if (streamMusic->nextFileName != nullptr) {
			streamMusic->streamAudioFile(streamMusic->nextFileName);
		}
		Sleep(100);
	}
	return 0;
}

class SoundManager {
	class VoiceCallback : public IXAudio2VoiceCallback {
	public:
		HANDLE bufferEndEvent;
		SoundManager *soundManager;
		VoiceCallback() : bufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
		~VoiceCallback() {
			CloseHandle(bufferEndEvent);
		}

		// Called when the voice has just finished playing a contiguous audio stream.
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnStreamEnd() override {
			SetEvent(bufferEndEvent);
			soundManager->clearCallback();
		}

		// Unused methods are stubs
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassEnd() override {}
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassStart(UINT32 samplesRequired) override {} 
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferEnd(void *bufferContext) override {}
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferStart(void *bufferContext) override {} 
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnLoopEnd(void *bufferContext) override {}

	private:
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceError(void *bufferContext, HRESULT Error) override {}
	};

private:
	IXAudio2 *xAudio2;
	IXAudio2MasteringVoice *masterVoice;

	VoiceCallback *voiceCallbackPtr;
	VoiceCallback voiceCallback;

	int soundIndex = 0;

	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2sourcevoice-setfrequencyratio
	// Frequency adjustment is expressed as source frequency / target frequency. 
	// Changing the frequency ratio changes the rate audio is played on the voice. 
	// A ratio greater than 1.0 will cause the audio to play faster and a ratio less 
	// than 1.0 will cause the audio to play slower. Additionally, the frequency 
	// ratio affects the pitch of audio on the voice. As an example, a value of 1.0 
	// has no effect on the audio, whereas a value of 2.0 raises pitch by one octave 
	// and 0.5 lowers it by one octave.
	float sourceRate = 1.0f;
	float targetRate = 1.0f;

	static const int voiceBufferSize = 2;
	IXAudio2SourceVoice *voices[voiceBufferSize] = {};

public:
	~SoundManager() {
		xAudio2->Release();

		DWORD threadID = 0;
		streamMusicData->isAlive = false;

		WaitForMultipleObjects(1, &streamMusicThread, TRUE, 5000);

		// Close all thread handles and free memory allocations.
		CloseHandle(streamMusicThread);
		if (streamMusicData != NULL) {
			HeapFree(GetProcessHeap(), 0, streamMusicData);

			// Ensure address is not reused.
			streamMusicData = NULL; 
		}

		// Release resources
		masterVoice->DestroyVoice();
		musicXAudio2->Release();
		CoUninitialize();
	}

	void clearCallback() {
		voiceCallbackPtr = nullptr;
	}

	void initialise() {
		xAudio2 = nullptr;
		voiceCallbackPtr = nullptr;
		HRESULT hr;

		hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		ASSERT_HRESULT(hr)

		masterVoice = nullptr;
		hr = xAudio2->CreateMasteringVoice(&masterVoice);
		ASSERT_HRESULT(hr)

#ifdef DEBUG
		XAUDIO2_DEBUG_CONFIGURATION flags;
		flags.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_DETAIL;
		flags.BreakMask = XAUDIO2_LOG_ERRORS;
		flags.LogFileline = true;
		flags.LogFunctionName = true;
		flags.LogThreadID = true;
		flags.LogTiming = true;

		xAudio2->SetDebugConfiguration(&flags);
#endif

		hr = XAudio2Create(&musicXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		ASSERT_HRESULT(hr)

		// Create a master sound, the default is to output the current speaker
		hr = musicXAudio2->CreateMasteringVoice(&masterVoice);
		ASSERT_HRESULT(hr)

		streamMusicData = (StreamMusic*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(StreamMusic));
		if (streamMusicData == NULL) {
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
		}
		streamMusicData->isAlive = true;
		streamMusicData->isLooping = true;

		// Create the thread to begin execution on its own.
		streamMusicThread = CreateThread(
			NULL,                // default security attributes
			0,                   // use default stack size  
			playMusicThread,     // thread function name
			streamMusicData,     // argument to thread function 
			0,                   // use default creation flags 
			&streamMusicThreadId // returns the thread identifier
		); 

		if (streamMusicThread == NULL) {
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
		}
	}

	void process(SoundLoadQueue *soundQueue, MusicAssetId *musicToPlay) {
		for (SoundAssetId assetId : *soundQueue) {
			LPCWSTR fileName = soundNames[(size_t)assetId];
			playGameSound(fileName);
		}

		if (*musicToPlay != MusicAssetId::none) {
			LPCWSTR fileName = musicNames[(size_t)*musicToPlay];
			streamMusicData->playNewFile(fileName);
		}

		soundQueue->clear();
		*musicToPlay = MusicAssetId::none;
	}

	void playGameSound(const TCHAR *strFileName) {
		const TCHAR *fileName;
		fileName = strFileName;

		HRESULT hr;
		WAVEFORMATEXTENSIBLE wfx;
		DWORD fileType, waveFileSize;
		BYTE *soundDataBuffer;
		HANDLE fileHandle;
		getWaveData(fileName, &wfx, &fileType, &waveFileSize, &soundDataBuffer);

		XAUDIO2_BUFFER buffer = { 0 };
		// 6. Populate an XAUDIO2_BUFFER structure.
		buffer.AudioBytes = waveFileSize; // size of the audio buffer in bytes
		buffer.pAudioData = soundDataBuffer; // buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		bool canPlaySound = false;
		int freeVoiceBufferIndex = 0;
		for (;freeVoiceBufferIndex < voiceBufferSize; freeVoiceBufferIndex++) {
			if (voices[freeVoiceBufferIndex] != nullptr) {
				XAUDIO2_VOICE_STATE state;
				voices[freeVoiceBufferIndex]->GetState(&state);
				if (state.BuffersQueued <= 0) {
					voices[freeVoiceBufferIndex]->DestroyVoice();
					voices[freeVoiceBufferIndex] = nullptr;
					canPlaySound = true;
					break;
				}
			} else {
				canPlaySound = true;
				break;
			}
		}

		if (canPlaySound) {
			// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
			// 3. Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. 
			// The format of the voice is specified by the values set in a WAVEFORMATEX structure.
			IXAudio2SourceVoice *soundSourceVoice;

			hr = xAudio2->CreateSourceVoice(
				&soundSourceVoice, 
				(WAVEFORMATEX *)&wfx,
				0, 
				XAUDIO2_DEFAULT_FREQ_RATIO, 
				&voiceCallback, 
				NULL, 
				NULL
			);
			ASSERT_HRESULT(hr)

			voiceCallbackPtr = &voiceCallback;
			voiceCallbackPtr->soundManager = this;

			// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
			hr = soundSourceVoice->SubmitSourceBuffer(&buffer);
			ASSERT_HRESULT(hr)

			// 5. Use the Start function to start the source voice. Since all XAudio2 
			// voices send their output to the mastering voice by default, audio from 
			// the source voice automatically makes its way to the audio device selected 
			// at initialization. In a more complicated audio graph, the source voice 
			// would have to specify the voice to which its output should be sent.
			hr = soundSourceVoice->Start(0);
			ASSERT_HRESULT(hr)

			voices[freeVoiceBufferIndex] = soundSourceVoice;
		}
	}

	void setPitch(float target) {
		// Frequency adjustment ratio.This value must be between XAUDIO2_MIN_FREQ_RATIO 
		// and the MaxFrequencyRatio parameter specified when the voice was created
		// (see IXAudio2::CreateSourceVoice). XAUDIO2_MIN_FREQ_RATIO currently is 
		// 0.0005, which allows pitch to be lowered by up to 11 octaves.

		// Input of 0 means 1.0f so always add this on
		target += 1.0f;

		for (int index = 0; index < voiceBufferSize; index++) {
			if (voices[index] != nullptr) {
				voices[index]->SetFrequencyRatio(target);
			}
		}
	}

	void produceSineWave() {
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
