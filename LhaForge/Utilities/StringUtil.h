/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once
//---string operations

//trim trailing symbols
std::wstring UtilTrimString(const std::wstring &target, const std::wstring &trimTargets);

//builds filter string for CFileDialog from MFC style string, i.e., "*.txt|*.doc||"
std::wstring UtilMakeFilterString(const std::wstring& filterIn);

enum class UTIL_CODEPAGE {
	CP932 = 932,
	UTF8 = CP_UTF8,
	UTF16 = 1200,
	//UTF16BE = 1201,
};

std::wstring UtilToUNICODE(const char* lpSrc, size_t length, UTIL_CODEPAGE uSrcCodePage);
inline std::wstring UtilUTF8toUNICODE(const char* utf8, size_t length) { return UtilToUNICODE(utf8, length, UTIL_CODEPAGE::UTF8); }
inline std::wstring UtilCP932toUNICODE(const char* cp932, size_t length) { return UtilToUNICODE(cp932, length, UTIL_CODEPAGE::CP932); }
inline std::wstring UtilUTF16toUNICODE(const char* utf16, size_t length) { return UtilToUNICODE(utf16, length, UTIL_CODEPAGE::UTF16); }

UTIL_CODEPAGE UtilGuessCodepage(const char* lpSrc, size_t length);
//checks if the code page is correct for the given string
bool UtilVerityGuessedCodepage(const char* lpSrc, size_t length, UTIL_CODEPAGE uSrcCodePage);


//expand variables placed in braces, such as "{foo}"
std::wstring UtilExpandTemplateString(const std::wstring& format, const std::map<std::wstring, std::wstring> &envVars);

std::vector<std::wstring> UtilSplitString(const std::wstring& target, const std::wstring& separator);
//split string into number array
std::vector<int> UtilStringToIntArray(const std::wstring&);

// (size in bytes) to (size in suitable unit)
std::wstring UtilFormatSize(UINT64 size);

std::wstring UtilFormatTime(__time64_t timer);

template <typename ...Args>
std::wstring Format(const std::wstring& fmt, Args && ...args)
{
	//snprintf_s will not return the required buffer size
	std::wstring work;
#pragma warning(push)
#pragma warning(disable:4996)
	auto size = _snwprintf(nullptr, 0, fmt.c_str(), std::forward<Args>(args)...);
	work.resize(size + 1);
	_snwprintf(&work[0], work.size(), fmt.c_str(), std::forward<Args>(args)...);
#pragma warning(pop)
	return work.c_str();
}

//https://marycore.jp/prog/cpp/std-string-replace-first-all/
template<class T, class U>
std::wstring replace(const std::wstring &_s, const T& target, const U& replacement, bool replace_first = 0, bool replace_empty = 0)
{
	auto s = _s;
	using S = std::wstring;
	using C = S::value_type;
	using N = S::size_type;
	struct {
		N len(const S& s) { return s.size(); }
		N len(const C* p) { return std::char_traits<C>::length(p); }
		N len(const C  c) { return 1; }
		void sub(S* s, const S& t, N pos, N len) { s->replace(pos, len, t); }
		void sub(S* s, const C* t, N pos, N len) { s->replace(pos, len, t); }
		void sub(S* s, const C  t, N pos, N len) { s->replace(pos, len, 1, t); }
		void ins(S* s, const S& t, N pos) { s->insert(pos, t); }
		void ins(S* s, const C* t, N pos) { s->insert(pos, t); }
		void ins(S* s, const C  t, N pos) { s->insert(pos, 1, t); }
	} util;

	N target_length = util.len(target);
	N replacement_length = util.len(replacement);
	if (target_length == 0) {
		if (!replace_empty || replacement_length == 0) return s;
		N n = s.size() + replacement_length * (1 + s.size());
		s.reserve(!replace_first ? n : s.size() + replacement_length);
		for (N i = 0; i < n; i += 1 + replacement_length) {
			util.ins(&s, replacement, i);
			if (replace_first) break;
		}
		return s;
	}

	N pos = 0;
	while ((pos = s.find(target, pos)) != std::string::npos) {
		util.sub(&s, replacement, pos, target_length);
		if (replace_first) return s;
		pos += replacement_length;
	}
	return s;
}

inline std::wstring toLower(const std::wstring& input) {
	std::wstring output;
	std::transform(input.begin(), input.end(), std::back_inserter(output),
		[](wchar_t c) {
		return towlower(c);
	});
	return output;
}

inline std::wstring toUpper(const std::wstring& input) {
	std::wstring output;
	std::transform(input.begin(), input.end(), std::back_inserter(output),
		[](wchar_t c) {
		return towupper(c);
	});
	return output;
}
