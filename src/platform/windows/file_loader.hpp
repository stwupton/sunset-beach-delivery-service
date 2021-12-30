#pragma once

#include <Windows.h>

#include "platform/windows/utils.hpp"

void load(const wchar_t *filePath, void *destination, size_t size) {
	HANDLE handle = CreateFile(
		filePath, 
		GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
	);

	const DWORD error = GetLastError();
	bool succeeded = error == ERROR_SUCCESS;
	if (!succeeded) {
		void *buffer;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&buffer,
			0,
			NULL
		);

		LOG((LPWSTR)buffer)
		LocalFree(buffer);

		assert(false);
	}

	succeeded = ReadFile(handle, destination, size, nullptr, nullptr);
	assert(succeeded);

	CloseHandle(handle);
}