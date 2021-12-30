#pragma once

#include <cassert>

#include <Windows.h>

#include "platform/windows/utils.hpp"

void save(const wchar_t *filePath, void *source, size_t size) {
	HANDLE handle = CreateFile(
		filePath, 
		GENERIC_WRITE, 
		0, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
	);

	const DWORD error = GetLastError();
	bool succeeded = error & (ERROR_SUCCESS | ERROR_ALREADY_EXISTS);
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

	succeeded = WriteFile(handle, source, size, nullptr, nullptr);
	assert(succeeded);

	CloseHandle(handle);
}