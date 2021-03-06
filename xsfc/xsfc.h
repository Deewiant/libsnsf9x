#if _MSC_VER >= 1200
# pragma once
#endif

#ifndef XSFC_H__
#define XSFC_H__

#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4290)
#endif

#ifdef __cplusplus

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if !defined(HAVE_SPRINTF_S) && !defined(HAVE_SNPRINTF_) && defined(_MSC_VER)
# if _MSC_VER >= 1400
#  define HAVE_SPRINTF_S
# elif _MSC_VER >= 1200
#  define HAVE_SNPRINTF_
# endif
#endif

namespace xsfc
{

using std::nothrow;

class EBaseException
{
protected:
	EBaseException(int c) throw()
		: Code(c)
	{
	}
public:
	int Code;
	EBaseException(const EBaseException &e) throw()
		: Code(e.Code)
	{
	}
	~EBaseException() throw() {} 
};

class EShortOfMemory : public EBaseException
{
public:
	EShortOfMemory() throw()
		: EBaseException(1)
	{
	}
	EShortOfMemory(const EShortOfMemory &e) throw()
		: EBaseException(e)
	{
	}
	~EShortOfMemory() throw() {}
};

template <class T> class TAutoPtr
{
protected:
	T *ptr;

public:
	void Release() throw()
	{
		if (ptr)
		{
			delete ptr;
			ptr = 0;
		}
	}
	T *Ptr() const throw()
	{
		return ptr;
	}
	T *Detach() throw()
	{
		T *ret = ptr;
		ptr = 0;
		return ret;
	}
	TAutoPtr<T>(T *p = 0) throw()
		: ptr(p)
	{
	}
	~TAutoPtr<T>() throw()
	{
		Release();
	}
	T * operator->() const throw()
	{
		return ptr;
	}
	operator T *() const throw()
	{
		return ptr;
	}
	TAutoPtr<T> &operator =(T *p) throw()
	{
		Release();
		ptr = p;
		return *this;
	}
};

template <class T> class TSimpleArray
{
protected:
	T *ptr;
	size_t length;

	void Free() throw()
	{
		if (ptr)
		{
			delete [] ptr;
			ptr = 0;
		}
		length = 0;
	}

public:

	~TSimpleArray<T>() throw()
	{
		Free();
	}

	TSimpleArray<T>() throw()
		: ptr(0), length(0)
	{
	}
	
	bool Resize(size_t l) throw()
	{
		Free();
		if (l > 0)
		{
			ptr = new(nothrow) T[l];
			if (ptr) length = l;
		}
		return ptr != 0 || l == 0;
	}

	bool ResizeE(size_t l)
	{
		Free();
		if (l > 0)
		{
			ptr = new(nothrow) T[l];
			if (ptr) length = l;
		}
		return ptr != 0 || l == 0;
	}

	inline T* Ptr() const throw()
	{
		return ptr;
	}

	inline size_t Len() const throw()
	{
		return length;
	}
};

template <class T> class TRefCnt
{
protected:
	T target;
	unsigned count;

	~TRefCnt<T>() throw()
	{
	}

	TRefCnt<T>() throw()
		: count(0)
	{
	}

public:

	void Release() throw()
	{
		if (--count == 0)
		{
			delete this;
		}
	}

	TRefCnt<T> *AddRef() throw()
	{
		++count;
		return this;
	}

	inline T &Target() throw() 
	{
		return target;
	}

	static TRefCnt<T> *Create() throw()
	{
		TRefCnt<T> *ret = new(nothrow) TRefCnt<T>();
		return ret ? ret->AddRef() : 0;
	}
};

template <class T> class TReference
{
protected:
	TRefCnt<T> *refbuf;
public:
	void Free() throw()
	{
		if (refbuf)
		{
			refbuf->Release();
			refbuf = 0;
		}
	}

	~TReference<T>() throw()
	{
		Free();
	}

	TReference<T>(const bool fInitialze = true) throw()
		: refbuf(fInitialze ? TRefCnt<T>::Create() : 0)
	{
	}

	TReference<T>(const TReference<T> &s) throw()
		: refbuf(s.refbuf ? s.refbuf->AddRef() : 0)
	{
	}

	TReference<T> &operator =(const TReference<T> &s) throw()
	{
		// AddRef must be called before than Free because refbuf may be equal to s.refbuf.
		TRefCnt<T> *newbuf = s.refbuf ? s.refbuf->AddRef() : 0;
		Free();
		refbuf = newbuf;
		return *this;
	}

	inline T *Ptr() const throw() 
	{
		return refbuf ? &refbuf->Target() : 0;
	}
};

template <typename T> class TAutoBuffer
{
protected:
	typedef TSimpleArray<T> array_t;
	TReference<array_t> ref;

public:
	inline void Free() throw()
	{
		ref.Free();
	}

	bool Resize(size_t l) throw()
	{
		ref = TReference<array_t>(true);
		if (ref.Ptr() && !ref.Ptr()->Resize(l))
		{
			ref.Free();
		}
		return ref.Ptr()->Ptr() != 0;
	}

	~TAutoBuffer<T>() throw()
	{
	}

	TAutoBuffer<T>() throw()
		: ref(false)
	{
	}

	TAutoBuffer<T>(const TAutoBuffer<T> &s) throw()
		: ref(s.ref)
	{
	}

	TAutoBuffer<T> &operator =(const TAutoBuffer<T> &s) throw()
	{
		ref = s.ref;
		return *this;
	}

	T * Ptr() const throw()
	{
		array_t *ptr = ref.Ptr();
		return ptr ? ptr->Ptr() : 0;
	}

	size_t Len() const throw()
	{
		array_t *ptr = ref.Ptr();
		return ptr ? ptr->Len() : 0;
	}
};

class TIConv
{
public:
	static size_t ToWide(bool isUTF8, const char *s, size_t sl, wchar_t *d, size_t dl) throw();
	static size_t ToMulti(bool isUTF8, const wchar_t *s, size_t sl, char *d, size_t dl) throw();
};

template <class T> size_t StrNLen(const T *s, size_t n = -1) throw()
{
	size_t l;
	for (l = 0; s && ((n == (size_t)-1) || (l < n)) && s[l]; l++)
		;
	return l;
}

class TStringM
{
protected:
	TAutoBuffer<char> buf;
	bool isutf8;

	void SetWU(bool isUTF8, const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		isutf8 = isUTF8;
		size_t sl = StrNLen<wchar_t>(p, n);
		size_t l = TIConv::ToMulti(isutf8, p, sl, 0, 0);
		if (!buf.Resize(l + 1))
			throw EShortOfMemory();
		else
		{
			char *ptr = buf.Ptr();
			ptr[l] = '\0';
			if (l > 0)
				TIConv::ToMulti(isutf8, p, sl, ptr, l);
		}
	}


	void SetMU(bool isUTF8, const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		isutf8 = isUTF8;
		size_t l = StrNLen<char>(p, n);
		if (!buf.Resize(l + 1))
			throw EShortOfMemory();
		else
		{
			char *ptr = buf.Ptr();
			ptr[l] = L'\0';
			memcpy(ptr, p, l * sizeof(char));
		}
	}

public:
	TStringM(bool utf8, const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetMU(utf8, p, n);
	}

	TStringM(const TStringM &p) throw()
		: buf(p.buf), isutf8(p.isutf8)
	{
	}

	TStringM &operator =(const TStringM &s) throw()
	{
		buf = s.buf;
		isutf8 = s.isutf8;
		return *this;
	}

	void SetW(const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetWU(false, p, n);
	}

	void SetW(bool utf8, const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetWU(utf8, p, n);
	}

	const char *GetM(void) const throw()
	{
		return buf.Len() ? buf.Ptr() : "";
	}
	
	operator const char *(void) const throw()
	{
		return GetM();
	}

	TStringM() throw()
		: isutf8(false)
	{
	}

	TStringM(const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetWU(false, p, n);
	}

	TStringM(bool utf8, const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetWU(utf8, p, n);
	}

	~TStringM() throw()
	{
	}

	inline bool IsUTF8(void) const throw()
	{
		return isutf8;
	}
};

class TString
{
protected:
	TAutoBuffer<wchar_t> buf;

	void SetMU(bool isUTF8, const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		size_t sl = StrNLen<char>(p, n);
		size_t l = TIConv::ToWide(isUTF8, p, sl, 0, 0);
		if (!buf.Resize(l + 1))
			throw EShortOfMemory();
		else
		{
			wchar_t *ptr = buf.Ptr();
			if (ptr) ptr[l] = L'\0';
			if (l > 0)
				TIConv::ToWide(isUTF8, p, sl, ptr, l);
		}
	}

	void SetFloat(const double &s) throw(EShortOfMemory)
	{
		char buf[32];
#if defined(HAVE_SPRINTF_S)
		sprintf_s(buf, sizeof(buf), "%g", s);
#elif defined(HAVE_SNPRINTF)
		snprintf(buf, sizeof(buf), "%g", s);
#elif defined(HAVE_SNPRINTF_)
		_snprintf(buf, sizeof(buf), "%g", s);
#else
		sprintf(buf, "%g", s);
#endif
		SetMU(false, buf, sizeof(buf));
	}

	void SetULong(const unsigned long &s) throw(EShortOfMemory)
	{
		char buf[32];
#if defined(HAVE_SPRINTF_S)
		sprintf_s(buf, sizeof(buf), "%lu", s);
#elif defined(HAVE_SNPRINTF)
		snprintf(buf, sizeof(buf), "%lu", s);
#elif defined(HAVE_SNPRINTF_)
		_snprintf(buf, sizeof(buf), "%lu", s);
#else
		sprintf(buf, "%lu", s);
#endif
		SetMU(false, buf, sizeof(buf));
	}

	void SetLong(const long &s) throw(EShortOfMemory)
	{
		char buf[32];
#if defined(HAVE_SPRINTF_S)
		sprintf_s(buf, sizeof(buf), "%ld", s);
#elif defined(HAVE_SNPRINTF)
		snprintf(buf, sizeof(buf), "%ld", s);
#elif defined(HAVE_SNPRINTF_)
		_snprintf(buf, sizeof(buf), "%ld", s);
#else
		sprintf(buf, "%ld", s);
#endif
		SetMU(false, buf, sizeof(buf));
	}

public:

	void SetW(const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		size_t l = StrNLen<wchar_t>(p, n);
		if (!buf.Resize(l + 1))
			throw EShortOfMemory();
		else
		{
			wchar_t *ptr = buf.Ptr();
			ptr[l] = L'\0';
			memcpy(ptr, p, l * sizeof(wchar_t));
		}
	}

	void SetM(const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetMU(false, p, n);
	}

	void SetUTF8(const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetMU(true, p, n);
	}

	const wchar_t *GetW(void) const throw()
	{
		return buf.Len() ? buf.Ptr() : L"";
	}

	operator const wchar_t *(void) const throw()
	{
		return GetW();
	}

	TString() throw()
	{
	}

	TString(const TString &p) throw()
		: buf(p.buf)
	{
	}

	TString(const TStringM & p) throw(EShortOfMemory)
	{
		SetMU(p.IsUTF8(), p);
	}

	TString(const wchar_t *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetW(p, n);
	}

	TString(const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetM(p, n);
	}

	TString(bool isUTF8, const char *p, size_t n = -1) throw(EShortOfMemory)
	{
		SetMU(isUTF8, p, n);
	}

	TString(unsigned long s) throw(EShortOfMemory)
	{
		SetULong(s);
	}

	TString(long s) throw(EShortOfMemory)
	{
		SetLong(s);
	}

	TString(double s) throw(EShortOfMemory)
	{
		SetFloat(s);
	}

	~TString() throw()
	{
	}

	TString &operator =(const TString &s) throw()
	{
		buf = s.buf;
		return *this;
	}

	TString &operator =(const TStringM &s) throw()
	{
		SetMU(s.IsUTF8(), s);
		return *this;
	}

	friend TString operator+(const wchar_t *p, const TString &s) throw(EShortOfMemory)
	{
		TString r;
		const wchar_t *p2 = s.GetW();
		size_t l1 = StrNLen<wchar_t>(p);
		size_t l2 = StrNLen<wchar_t>(p2);
		if (!r.buf.Resize(l1 + l2 + 1))
			throw EShortOfMemory();
		else
		{
			wchar_t *des = r.buf.Ptr();
			des[l1 + l2] = L'\0';
			if (l1)
				memcpy(des, p, l1 * sizeof(wchar_t));
			if (l2)
				memcpy(des + l1, p2, l2 * sizeof(wchar_t));
		}
		return r;
	}

	double GetFloat() const throw(EShortOfMemory)
	{
		return atof(TStringM(GetW()));
	}
	unsigned long GetULong() const throw(EShortOfMemory)
	{
		return (unsigned long)(atol(TStringM(GetW())));
	}
	long GetLong() const throw(EShortOfMemory)
	{
		return long(atol(TStringM(GetW())));
	}
};

}

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#endif
