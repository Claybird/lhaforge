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
//Original code from http://hp.vector.co.jp/authors/VA016117/

class CLFDnDEnumFORMATETC : public IEnumFORMATETC
{
private:
	LONG _RefCount;
protected:
	std::vector<FORMATETC> m_fmtArray;
	size_t _current;
public:
	CLFDnDEnumFORMATETC() : _RefCount(1), _current(0) {};
	~CLFDnDEnumFORMATETC(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)override {
		HRESULT hr;
		if (iid == IID_IEnumFORMATETC || iid == IID_IUnknown) {
			hr = S_OK;
			*ppv = (void*)this;
			AddRef();
		} else {
			hr = E_NOINTERFACE;
			*ppv = 0;
		}
		return hr;
	}
	virtual ULONG __stdcall AddRef(void)override {
		InterlockedIncrement(&_RefCount);
		return (ULONG)_RefCount;
	}
	virtual ULONG __stdcall Release(void)override {
		ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
		if (ret == 0) {
			delete this;
		}
		return (ULONG)_RefCount;
	}

	virtual HRESULT __stdcall Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched)override {
		ULONG n = celt;
		if (pceltFetched != nullptr) *pceltFetched = 0;
		if (celt <= 0 || rgelt == nullptr || _current >= m_fmtArray.size())	return S_FALSE;
		if (pceltFetched == nullptr && celt != 1)	return S_FALSE;

		while (_current < m_fmtArray.size() && n > 0) {
			*rgelt++ = m_fmtArray[_current];
			_current++;
			n--;
		}
		if (pceltFetched != nullptr) *pceltFetched = celt - n;

		return (n == 0) ? S_OK : S_FALSE;
	}
	virtual HRESULT __stdcall Skip(ULONG celt)override {
		while (_current < m_fmtArray.size() && celt > 0) {
			_current++;
			celt--;
		}
		return (celt == 0) ? S_OK : S_FALSE;
	}
	virtual HRESULT __stdcall Reset(void)override {
		_current = 0;
		return S_OK;
	}
	virtual HRESULT __stdcall Clone(IEnumFORMATETC ** ppenum)override {
		CLFDnDEnumFORMATETC	*pfmt;

		if (!ppenum)return E_POINTER;
		pfmt = new CLFDnDEnumFORMATETC;
		if (pfmt == NULL) return E_OUTOFMEMORY;

		pfmt->m_fmtArray = m_fmtArray;
		pfmt->_current = _current;
		*ppenum = pfmt;
		return S_OK;
	}

	BOOL SetFormat(const FORMATETC &fmt) {
		m_fmtArray.push_back(fmt);
		return	TRUE;
	}
};
