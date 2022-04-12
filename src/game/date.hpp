#pragma once

#include <cmath>
#include <cstdio>

#include "common/game_definitions.hpp"
#include "types/string.hpp"

namespace DateUtils {
	const u16 year = 3485;
	const DayValue daysInYear = 365;
	const DayValue daysInMonths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	const wchar_t monthNames[][4] = {
		L"JAN", L"FEB", L"MAR", 
		L"APR", L"MAY", L"JUN", 
		L"JUL", L"AUG", L"SEP", 
		L"OCT", L"NOV", L"DEC"
	};

	String16<32> getDate(DayValue daysPassed) {
		const DayValue dayInYear = daysPassed % daysInYear + 1;

		const size_t monthLength = sizeof(daysInMonths) / sizeof(*daysInMonths);
		size_t monthIndex = floor(dayInYear / daysInYear / monthLength);

		DayValue daysOffset = 0;
		for (size_t i = 0; i < monthIndex; i++) {
			daysOffset += daysInMonths[i];
		}

		DayValue dayInMonth = dayInYear - daysOffset;
		if (dayInMonth < 1) {
			monthIndex--;
			dayInMonth += daysInMonths[monthIndex];
		}

		if (dayInMonth > daysInMonths[monthIndex]) {
			dayInMonth -= daysInMonths[monthIndex];
			monthIndex++;
		}

		String16<3> daySuffix = L"th";
		if (dayInMonth < 4 || dayInMonth > 20) {
			const DayValue singleDigit = dayInMonth % 10;
			if (singleDigit == 1) daySuffix = L"st";
			else if (singleDigit == 2) daySuffix = L"nd";
			else if (singleDigit == 3) daySuffix = L"rd";
		}

		const wchar_t *month = monthNames[monthIndex];

		String16<32> result;
		swprintf_s(result.data, L"%u%s %s %u", dayInMonth, daySuffix, month, year);

		return result;
	}
};