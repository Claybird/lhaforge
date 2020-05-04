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

//inter-process lock by semaphore
class CSemaphoreLocker{
protected:
	HANDLE hSemaphore;
public:
	CSemaphoreLocker():hSemaphore(NULL){}
	virtual ~CSemaphoreLocker(){
		Release();
	}
	virtual bool Lock(const wchar_t* lpName, LONG MaxCount){
		Release();
		hSemaphore=CreateSemaphore(NULL,MaxCount,MaxCount,lpName);
		if(ERROR_INVALID_HANDLE==GetLastError()){	//failed to get
			hSemaphore=NULL;
			return false;
		}
		return WAIT_OBJECT_0==WaitForSingleObject(hSemaphore,INFINITE);
	}
	virtual void Release(){
		if(hSemaphore){
			ReleaseSemaphore(hSemaphore,1,NULL);
			CloseHandle(hSemaphore);
			hSemaphore=NULL;
		}
	}
};
