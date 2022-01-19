#pragma once

#include <xaudio2.h>
#include <strsafe.h>
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
		Digital Signal Processing (DSP) effects are the pixel shaders of audio. They handle everything from transforming a sound�turning a pig squeal
		into a low, scary monster sound�to placing sounds in the game environment using reverb and occlusion or obstruction filtering.
		XAudio2 provides a flexible and powerful DSP framework. It also provides a built-in filter on every voice, for efficient low/high/band-pass
		filtering effects.
		See https://docs.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-audio-effects

	Submixing:
		Submixing combines several sounds into a single audio stream�for example, an engine sound made up of composite parts,
		all of which are playing simultaneously. Also, you can use submixing to process and combine similar parts of a game.
		For example, you could combine all game sound effects to allow a user volume setting to be applied while a separate setting controls
		music volume. Combined with DSP, submixing provides the type of data routing and processing necessary for today's games.
		XAudio2 allows for arbitrary levels of submixing, enabling the creation of complex sounds and game mixes.
		See https://docs.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-audio-graph

	Compressed Audio Support:
		One of the major feature requests for DirectSound has been for compressed audio support. XAudio2 supports compressed
		formats�ADPCM�natively with run-time decompression.

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
			hr = (HRESULT_FROM_WIN32(GetLastError()));

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = (HRESULT_FROM_WIN32(GetLastError()));

		switch (dwChunkType) {
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = (HRESULT_FROM_WIN32(GetLastError()));
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				hr = (HRESULT_FROM_WIN32(GetLastError()));
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
		FAILED(HRESULT_FROM_WIN32(GetLastError()));
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		FAILED(HRESULT_FROM_WIN32(GetLastError()));
}

// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--stream-a-sound-from-disk
// 1.Create an array of read buffers.
#define STREAMBUFFERSIZE 65536//The size of each buffer
#define BUFFERNUM 5//Number of buffers
BYTE streamBuffers[BUFFERNUM][STREAMBUFFERSIZE];//Buffer array
#define MAX_THREADS 1

IXAudio2* pMusicEngine = NULL;
int currentBufferIndex = 0;

class StreamingVoiceContext : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;
	StreamingVoiceContext() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
	~StreamingVoiceContext() { CloseHandle(hBufferEndEvent); }
	//void OnStreamEnd() {/* SetEvent (hBufferEndEvent); */ }
	//Called when the voice has just finished playing a contiguous audio stream.
	void STDMETHODCALLTYPE  OnStreamEnd() {
		SetEvent(hBufferEndEvent);
	}
	//void STDMETHODCALLTYPE  OnVoiceProcessingPassEnd() {}
	//void STDMETHODCALLTYPE  OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
	////Called when the voice has just finished playing an audio buffer.
	//void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) { SetEvent(hBufferEndEvent); }

	// Inherited via IXAudio2VoiceCallback
	virtual void __stdcall OnBufferStart(void* pBufferContext) override
	{
	}
	virtual void __stdcall OnLoopEnd(void* pBufferContext) override
	{
	}
	virtual void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override
	{
	}

	// Inherited via IXAudio2VoiceCallback
	virtual void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override
	{
	}
	virtual void __stdcall OnVoiceProcessingPassEnd(void) override
	{
	}
	virtual void __stdcall OnBufferEnd(void* pBufferContext) override
	{
		SetEvent(hBufferEndEvent);
	}
	//void OnBufferStart(void* pBufferContext) {}
	//void OnLoopEnd(void* pBufferContext) {/* SetEvent (hBufferEndEvent); */ }
	//void OnVoiceError(void* pBufferContext, HRESULT Error) {}
};

void GetWaveInfo(const TCHAR* fileName, WAVEFORMATEXTENSIBLE& wfx, DWORD& filetype, DWORD& waveSize, bool readWaveData,
	BYTE** waveData, bool storeHandle, HANDLE& hFile, DWORD& waveDataStartPosition)
{
	hFile = CreateFile(
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
		*waveData = new BYTE[waveSize];
		ReadChunkData(hFile, *waveData, waveSize, waveDataStartPosition);
	}

	if (!storeHandle)
	{
		// Close hFile stream now that we've loaded all the data into memory
		CloseHandle(hFile);
	}
}

typedef struct StreamMusic
{
	//IXAudio2* pXAudio2;
	TCHAR* strFileName;
	TCHAR* nextFileName;

	bool isChanging = false;
	bool isPlaying = false;
	bool isAlive = true;

	void PlayNewFile(LPWSTR newFileName)
	{
		isChanging = true;

		/*strFileName = newFileName;

		while (isPlaying)
		{
			Sleep(100);
		}

		isChanging = false;

		StreamAudioFile(strFileName);*/

		nextFileName = newFileName;
	}

	void StreamAudioFile(LPWSTR fileName)//Thread callback
	{
		isPlaying = true;
		//CWaveFile waveFile;
		//HRESULT hr = waveFile.Open(fileName, NULL, WAVEFILE_READ);//Load the file
		//if (FAILED(hr))
		//	return;

		nextFileName = fileName;

		while (nextFileName != nullptr)
		{
			WAVEFORMATEXTENSIBLE wfx = { 0 };
			DWORD fileType, cbWaveSize, waveDataStartPosition;
			fileType = cbWaveSize = waveDataStartPosition = 0;
			HANDLE hFile = NULL;
			GetWaveInfo(nextFileName, wfx, fileType, cbWaveSize, false, NULL, true, hFile, waveDataStartPosition);
			//WAVEFORMATEX* waveFormat = waveFile.GetFormat();//Get file format

			StreamingVoiceContext pCallBack;
			IXAudio2SourceVoice* pSourceVoice = NULL;

			HRESULT hr = pMusicEngine->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx, 0, 1.0f, &pCallBack);//Create source sound to submit data
			if (FAILED(hr))
				return;

			OVERLAPPED Overlapped = { 0 };

			//int fileSize = waveFile.GetSize();//Get the file size
			int fileSize = cbWaveSize;
			int currentPos = waveDataStartPosition;

			// clear so doesn't play again
			nextFileName = nullptr;

			while (currentPos < fileSize)
			{
				if (isChanging)
				{
					break;
				}

				DWORD size = STREAMBUFFERSIZE;


				//if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, currentPos, NULL, FILE_BEGIN))
				//	FAILED(HRESULT_FROM_WIN32(GetLastError()));

				//if (0 == ReadFile(hFile, streamBuffers[currentBufferIndex], size, &size, &Overlapped))
				//	hr = HRESULT_FROM_WIN32(GetLastError());

				ReadChunkData(hFile, streamBuffers[currentBufferIndex], size, currentPos);

				//hr = waveFile.Read(streamBuffers[currentBufferIndex], size, &size);//Read the file content
				//if (FAILED(hr))
				//	break;

				currentPos += size;//The size of the data that has been played

				XAUDIO2_BUFFER buffer = { 0 };//The file data to be read will be assigned XAUDIO2_BUFFER
				buffer.AudioBytes = size;
				buffer.pAudioData = streamBuffers[currentBufferIndex];

				hr = pSourceVoice->SubmitSourceBuffer(&buffer);//Submit memory data
				if (FAILED(hr))
					break;

				hr = pSourceVoice->Start(XAUDIO2_COMMIT_NOW);//Start source sound
				if (FAILED(hr))
					break;

				XAUDIO2_VOICE_STATE state;
				pSourceVoice->GetState(&state);//Get state
				while (state.BuffersQueued > BUFFERNUM - 1)//Do not let the audio data of the buffer be overwritten
				{
					WaitForSingleObject(pCallBack.hBufferEndEvent, INFINITE);
					pSourceVoice->GetState(&state);
				}
				currentBufferIndex++;//Recycle buffer
				currentBufferIndex %= BUFFERNUM;
			}

			XAUDIO2_VOICE_STATE state;
			if (!isChanging)
			{
				while (pSourceVoice->GetState(&state), state.BuffersQueued > 0)//Wait for the completion of data playback in the queue, exit the thread
				{
					WaitForSingleObject(pCallBack.hBufferEndEvent, INFINITE);
				}
			}
			pSourceVoice->DestroyVoice();//Release resources

			isChanging = false;
		}

		isPlaying = false;
	}
} StreamMusic, * PStreamMusic;

PStreamMusic pDataArray[MAX_THREADS];
DWORD   dwThreadIdArray[MAX_THREADS];
HANDLE  hThreadArray[MAX_THREADS];

DWORD WINAPI PlayMusicThread(LPVOID lpParam)
{
	// get info from param
	PStreamMusic streamMusic;
	streamMusic = (PStreamMusic)lpParam;
	while (streamMusic->isAlive)
	{
		streamMusic->StreamAudioFile(streamMusic->strFileName);
	}
	return 0;
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
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnStreamEnd() override {
			SetEvent(hBufferEndEvent);
			soundManager->ClearCallback();
		}

		//Unused methods are stubs
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassEnd() override { }
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) override {    }
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferEnd(void* pBufferContext) override { }
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnBufferStart(void* pBufferContext) override {    }
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnLoopEnd(void* pBufferContext) override {    }
	private:
		virtual COM_DECLSPEC_NOTHROW void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override { }
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

		/*for (int i = 0; i < MAX_THREADS; i++)
	{*/
		int i = 0;
		// Allocate memory for thread data.
	//pDataArray[i] = (PStreamMusic)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		pDataArray[0] = (PStreamMusic)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(StreamMusic));

		if (pDataArray[i] == NULL) {
			// If the array allocation fails, the system is out of memory
			// so there is no point in trying to print an error message.
			// Just terminate execution.
			ExitProcess(2);
	  } //

		// Generate unique data for each thread to work with.
		//pDataArray[i]->strFileName = fileName;

		// Create the thread to begin execution on its own.

		hThreadArray[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			PlayMusicThread,       // thread function name
			pDataArray[i],          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]);   // returns the thread identifier 


	// Check the return value for success.
	// If CreateThread fails, terminate execution. 
	// This will automatically clean up threads and memory. 

		if (hThreadArray[i] == NULL)
		{
			//ErrorHandler(TEXT("CreateThread"));
			/*LPTSTR errorText = "CreateThread";
			ErrorHandler(errorText);*/
			ExitProcess(3);
		}
		//} // End of main thread creation loop.

	}

	void process(SoundLoadQueue* soundQueue, MusicAssetId* musicToPlay) {

		for (SoundAssetId assetId : *soundQueue) {
			//LPCWSTR fileName = textureNames[(size_t)assetId];
			LPCWSTR fileName = soundNames[(size_t)assetId];
			PlaySound(fileName);
		}

		if (musicToPlay != NULL)
		{
			// Potential to have multiple music threads, but for now hardcode to first thread
			DWORD threadID = 0;
			/*LPWSTR fileName = musicNames[(size_t)musicToPlay];
			pDataArray[threadID]->PlayNewFile(fileName);*/
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
		DWORD fileType, cbWaveSize, waveStartPos;
		BYTE* pDataBuffer;
		HANDLE fileHandle;
		GetWaveInfo(fileName, wfx, fileType, cbWaveSize, true, &pDataBuffer, false, fileHandle, waveStartPos);

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
