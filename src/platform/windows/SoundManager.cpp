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

class SoundManager
{
private:
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

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
	HRESULT Initialise()
	{
		pXAudio2 = nullptr;
		HRESULT hr;

		if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
			return hr;

		pMasterVoice = nullptr;
		if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
			return hr;

#ifdef DEBUG

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
		HRESULT hr = NULL;
		// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2
		// 3. Locate the 'RIFF' chunk in the audio file, and check the file type.
		// Open the file
		HANDLE hFile = CreateFile(
			strFileName,
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

		// 6. Populate an XAUDIO2_BUFFER structure.
		buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		// https://docs.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
		// 3. Create a source voice by calling the IXAudio2::CreateSourceVoice method on an instance of the XAudio2 engine. 
		// The format of the voice is specified by the values set in a WAVEFORMATEX structure.
		IXAudio2SourceVoice* pSourceVoice;
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx)))
			return hr;

		// 4. Submit an XAUDIO2_BUFFER to the source voice using the function SubmitSourceBuffer.
		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
			return hr;

		// 5. Use the Start function to start the source voice. Since all XAudio2 voices send their output to the mastering voice by default, 
		// audio from the source voice automatically makes its way to the audio device selected at initialization. 
		// In a more complicated audio graph, the source voice would have to specify the voice to which its output should be sent.
		if (FAILED(hr = pSourceVoice->Start(0)))
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
