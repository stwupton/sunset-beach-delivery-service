#pragma once
#include <xaudio2.h>
#include "platform/windows/utils.hpp"
#include "platform/windows/sound_resource.hpp"
#include "common/game_state.hpp"

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

#include <mutex>
// a global instance of std::mutex to protect global variable
std::mutex xAudioMutex;

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


void FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) {
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK) {
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		switch (dwChunkType) {
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc) {
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			//return S_OK;
			return;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize)
			ASSERT_HRESULT(S_FALSE)

	}

}

void ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset) {
	//HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

}

// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--stream-a-sound-from-disk
// 1.Create an array of read buffers.
#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3
BYTE buffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];

typedef struct StreamMusic
{
	IXAudio2* pXAudio2;
	TCHAR* strFileName;
} StreamMusic, * PStreamMusic;

void GetWaveInfo(const TCHAR* fileName, WAVEFORMATEXTENSIBLE wfx, DWORD filetype, DWORD waveSize, bool readWaveData, BYTE* waveData, 
										DWORD waveDataStartPosition, bool keepFileOpen, HANDLE audioFileHandle)
{
	HANDLE hFile = CreateFile(
		fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
		ASSERT_HRESULT(S_FALSE)

	wfx = { 0 };

	// 4. Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	// 5. Locate the 'data' chunk, and read its contents into a buffer.
	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, waveSize, waveDataStartPosition);

	if (readWaveData)
	{
		waveData = new BYTE[waveSize];
		ReadChunkData(hFile, waveData, waveSize, waveDataStartPosition);
	}

	if (keepFileOpen)
	{
		audioFileHandle = hFile;
	}
	else
	{
		// Close hFile stream now that we've loaded all the data into memory
		CloseHandle(hFile);
	}

}

DWORD WINAPI PlayMusicThread(LPVOID lpParam)
{
	// get info from param
	PStreamMusic streamMusic;
	streamMusic = (PStreamMusic)lpParam;

	HRESULT hr;
	WAVEFORMATEXTENSIBLE wfx;
	DWORD fileType, cbWaveSize, dataStartPos;
	HANDLE hFile;
	GetWaveInfo(streamMusic->strFileName, wfx, fileType, cbWaveSize, false, NULL, dataStartPos, true, hFile);

	IXAudio2SourceVoice* pSourceVoice;

	// the access to this function is mutually exclusive
	//std::lock_guard<std::mutex> guard(xAudioMutex);
	try
	{
		xAudioMutex.lock();
		if (FAILED(hr = streamMusic->pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx)))
			return hr;
		xAudioMutex.unlock();
	}
	catch (const std::exception&)
	{
		xAudioMutex.unlock();
	}

	// 2. Initialize an OVERLAPPED structure.
	// The structure is used to check when an asynchronous disk read has finished.
	OVERLAPPED Overlapped = { 0 };
	Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 3. Call the Start function on the source voice that will be playing the streaming audio.
	hr = pSourceVoice->Start(0, 0);

	// 4. Loop while the current read position is not passed the end of the audio file.
	int CurrentDiskReadBuffer = 0;
	int CurrentPosition = 0;
	while (CurrentPosition < cbWaveSize)
	{
		// 4a. Read a chunk of data from the disk into the current read buffer.
		DWORD dwRead;
		if (SUCCEEDED(hr) && 0 == ReadFile(hFile, buffers[CurrentDiskReadBuffer], STREAMING_BUFFER_SIZE, &dwRead, &Overlapped))
			hr = HRESULT_FROM_WIN32(GetLastError());
		DWORD cbValid = min(STREAMING_BUFFER_SIZE, cbWaveSize - CurrentPosition);
		//DWORD dwRead;
		if (0 == ReadFile(hFile, buffers[CurrentDiskReadBuffer], STREAMING_BUFFER_SIZE, &dwRead, &Overlapped))
			hr = HRESULT_FROM_WIN32(GetLastError());
		Overlapped.Offset += cbValid;

		//update the file position to where it will be once the read finishes
		CurrentPosition += cbValid;

		// 4b. Use the GetOverlappedResult function to wait for the event that signals the read has finished.
		DWORD NumberBytesTransferred;
		::GetOverlappedResult(hFile, &Overlapped, &NumberBytesTransferred, TRUE);

		// 4c. Wait for the number of buffers queued on the source voice to be less than the number of read buffers.
		// The state of the source voice is checked with the GetState function.
		XAUDIO2_VOICE_STATE state;
		while (pSourceVoice->GetState(&state), state.BuffersQueued >= MAX_BUFFER_COUNT - 1)
		{
			WaitForSingleObject(Context.hBufferEndEvent, INFINITE);
		}

		// 4d. Submit the current read buffer to the source voice using the SubmitSourceBuffer function.
		XAUDIO2_BUFFER buf = { 0 };
		buf.AudioBytes = cbValid;
		buf.pAudioData = buffers[CurrentDiskReadBuffer];
		if (CurrentPosition >= cbWaveSize)
		{
			buf.Flags = XAUDIO2_END_OF_STREAM;
		}
		pSourceVoice->SubmitSourceBuffer(&buf);

		// 4e. Set the current read buffer index to the next buffer.
		CurrentDiskReadBuffer++;
		CurrentDiskReadBuffer %= MAX_BUFFER_COUNT;
	}

	// 5. After the loop has finished, wait for the remaining queued buffers to finish playing.
	// When the remaining buffers have finished playing, the sound stops, and the thread can exit or be reused to stream another sound.
	XAUDIO2_VOICE_STATE state;
	while (pSourceVoice->GetState(&state), state.BuffersQueued > 0)
	{
		WaitForSingleObjectEx(Context.hBufferEndEvent, INFINITE, TRUE);
	}
}

void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}


class SoundManager {

	class VoiceCallback : public IXAudio2VoiceCallback {
	public:
		HANDLE hBufferEndEvent;
		SoundManager* soundManager;
		VoiceCallback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
		~VoiceCallback() {
			CloseHandle(hBufferEndEvent);
		}

		//Called when the voice has just finished playing a contiguous audio stream.
		void OnStreamEnd() {
			SetEvent(hBufferEndEvent);
			soundManager->ClearCallback();
		}

		//Unused methods are stubs
		void OnVoiceProcessingPassEnd() { }
		void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {    }
		void OnBufferEnd(void* pBufferContext) { }
		void OnBufferStart(void* pBufferContext) {    }
		void OnLoopEnd(void* pBufferContext) {    }
	private:
		void OnVoiceError(void* pBufferContext, HRESULT Error) { }
	};

private:
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;

	VoiceCallback* voiceCallbackPtr;
	VoiceCallback voiceCallback;

	int soundIndex = 0;

	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2sourcevoice-setfrequencyratio
	// Frequency adjustment is expressed as source frequency / target frequency. Changing the frequency ratio changes the rate audio 
	// is played on the voice.A ratio greater than 1.0 will cause the audio to play faster and a ratio less than 1.0 will cause the 
	// audio to play slower. Additionally, the frequency ratio affects the pitch of audio on the voice. As an example, a value of 1.0 has 
	// no effect on the audio, whereas a value of 2.0 raises pitch by one octave and 0.5 lowers it by one octave
	float sourceRate = 1.0f;
	float targetRate = 1.0f;	
	//float frequencyRatio;

	/*const wchar_t* left = L"assets/music/Left.wav";
	const wchar_t* right = L"assets/music/Right.wav";
	const wchar_t* stereo = L"assets/music/Stereo.wav";*/
	//static const int waveFileCount = 3;
	//WCHAR* waveFileNames[waveFileCount] = {
	//	L"assets/music/Left.wav",
	//	L"assets/music/Right.wav",
	//	L"assets/music/Stereo.wav"
	//};
	SoundResources* resources;

	static const int voiceBufferSize = 2;
	IXAudio2SourceVoice* voices[voiceBufferSize] = {};
	IXAudio2SourceVoice* backGroundMusic;

public:
	~SoundManager()	{
		pXAudio2->Release();
	}

	void ClearCallback() {
		//delete voiceCallbackPtr;
		voiceCallbackPtr = nullptr;
	}

	void Initialise(SoundResources* resources) {
		this->resources = resources;
		pXAudio2 = nullptr;
		voiceCallbackPtr = nullptr;
		HRESULT hr;

		hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		ASSERT_HRESULT(hr)

		pMasterVoice = nullptr;
		hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
		ASSERT_HRESULT(hr)

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
	}

	void process(SoundLoadQueue* soundQueue, SoundAssetId musicToPlay) {

		for (SoundAssetId assetId : *soundQueue) {
			//LPCWSTR fileName = textureNames[(size_t)assetId];
			LPCWSTR fileName = soundNames[(size_t)assetId];
			PlaySound(fileName);
		}

		soundQueue->clear();
	}

	void PlaySound(const TCHAR* strFileName) {

		const TCHAR* fileName;

		//// Check if we have an available slot
		//if (voiceCallbackPtr == nullptr) {
		//	fileName = waveFileNames[soundIndex++];

		//	if (soundIndex >= waveFileCount) {
		//		soundIndex = 0;
		//	}
		//}
		//else {
			fileName = strFileName;
		//}


		HRESULT hr;
		WAVEFORMATEXTENSIBLE wfx;
		DWORD fileType, cbWaveSize;
		BYTE* pDataBuffer;
		GetWaveInfo(fileName, wfx, fileType, cbWaveSize, true, pDataBuffer, NULL, false, NULL);

		//HRESULT hr = NULL;
		//// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
		//// 3. Locate the 'RIFF' chunk in the audio file, and check the file type.
		//// Open the file
		//HANDLE hFile = CreateFile(
		//	fileName,
		//	GENERIC_READ,
		//	FILE_SHARE_READ,
		//	NULL,
		//	OPEN_EXISTING,
		//	0,
		//	NULL);

		//if (INVALID_HANDLE_VALUE == hFile)
		//	ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		//if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		//	ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		//DWORD dwChunkSize;
		//DWORD dwChunkPosition;
		////check the file type, should be fourccWAVE or 'XWMA'
		//FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		//DWORD filetype;
		//ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		//if (filetype != fourccWAVE)
		//	ASSERT_HRESULT(S_FALSE)

		//// Declare local variables
		//WAVEFORMATEXTENSIBLE wfx = { 0 };

		//// 4. Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
		//FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		//ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

		//// 5. Locate the 'data' chunk, and read its contents into a buffer.
		////fill out the audio data buffer with the contents of the fourccDATA chunk
		//FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		//BYTE* pDataBuffer = new BYTE[dwChunkSize];
		//ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

		//// Close hFile stream now that we've loaded all the data into memory
		//CloseHandle(hFile);

		XAUDIO2_BUFFER buffer = { 0 };
		// 6. Populate an XAUDIO2_BUFFER structure.
		buffer.AudioBytes = cbWaveSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		bool canPlaySound = false;

		int freeVoiceBufferIndex = 0;

		for (;freeVoiceBufferIndex < voiceBufferSize; freeVoiceBufferIndex++) {
			if (voices[freeVoiceBufferIndex] != nullptr) {
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
				if (state.BuffersQueued <= 0) {
					voices[freeVoiceBufferIndex]->DestroyVoice();
					voices[freeVoiceBufferIndex] = nullptr;
					canPlaySound = true;
					break;
				}
			}
			else {
				canPlaySound = true;
				break;
			}
		}

		if (canPlaySound) {
			// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
			// 3. Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. 
			// The format of the voice is specified by the values set in a WAVEFORMATEX structure.
			IXAudio2SourceVoice* pSourceVoice;
			//if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx)))
			//	return hr;


			// the access to this function is mutually exclusive
			//std::lock_guard<std::mutex> guard(xAudioMutex);
			try
			{
				xAudioMutex.lock();
				hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx,
					0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL);
				ASSERT_HRESULT(hr)
				xAudioMutex.unlock();
			}
			catch (const std::exception&)
			{
				xAudioMutex.unlock();
			}

			voiceCallbackPtr = &voiceCallback;
			voiceCallbackPtr->soundManager = this;

			// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
			hr = pSourceVoice->SubmitSourceBuffer(&buffer);
			ASSERT_HRESULT(hr)

			// 5. Use the Start function to start the source voice. Since all XAudio2 voices send their output to the mastering voice by default, 
			// audio from the source voice automatically makes its way to the audio device selected at initialization. 
			// In a more complicated audio graph, the source voice would have to specify the voice to which its output should be sent.
			hr = pSourceVoice->Start(0);
			ASSERT_HRESULT(hr)

			voices[freeVoiceBufferIndex] = pSourceVoice;


			//WaitForSingleObjectEx(voiceCallback.hBufferEndEvent, INFINITE, TRUE);
		}

		/*else if (!voiceCallbackPtr->hBufferEndEvent)
		{
			voiceCallbackPtr = nullptr;
		}*/
	}

	void PlayMusic(const TCHAR* strFileName) {

		HRESULT hr = NULL;

		HANDLE hFile = CreateFile(
			strFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (INVALID_HANDLE_VALUE == hFile)
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()))

		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
			ASSERT_HRESULT(S_FALSE)


		// Declare local variables
		WAVEFORMATEXTENSIBLE wfx = { 0 };

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

		XAUDIO2_BUFFER buffer = { 0 };

		// 6. Populate an XAUDIO2_BUFFER structure.
		buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		bool canPlaySound = false;


		if (backGroundMusic != nullptr) {
			XAUDIO2_VOICE_STATE state;
			backGroundMusic->GetState(&state);
			if (state.BuffersQueued <= 0) {
				backGroundMusic->DestroyVoice();
				backGroundMusic = nullptr;
				canPlaySound = true;
			}
		}
		else {
			canPlaySound = true;
		}

		if (canPlaySound) {
			// the access to this function is mutually exclusive
			//std::lock_guard<std::mutex> guard(xAudioMutex);
			try
			{
				xAudioMutex.lock();
				hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx,
					0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL);
				ASSERT_HRESULT(hr)
				xAudioMutex.unlock();
			}
			catch (const std::exception&)
			{
				xAudioMutex.unlock();
			}
			IXAudio2SourceVoice* pSourceVoice;
			ASSERT_HRESULT(hr)
			voiceCallbackPtr = &voiceCallback;
			voiceCallbackPtr->soundManager = this;

			hr = pSourceVoice->SubmitSourceBuffer(&buffer);
			ASSERT_HRESULT(hr)

			hr = pSourceVoice->Start(0);
			ASSERT_HRESULT(hr)

			backGroundMusic = pSourceVoice;
		}

	}

	void SetPitch(float target)
	{
		// Frequency adjustment ratio.This value must be between XAUDIO2_MIN_FREQ_RATIOand the MaxFrequencyRatio parameter specified when the 
		// voice was created(see IXAudio2::CreateSourceVoice). XAUDIO2_MIN_FREQ_RATIO currently is 0.0005, which allows pitch to be lowered 
		// by up to 11 octaves.


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
