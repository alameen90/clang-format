#include "AmnCoreTypes.h"
#include <math.h>
#include <sstream>
#include <iomanip>

static CString GetStringSecLegacy( LPCTSTR s, int n, BOOL Warn, TCHAR ch )
{
	TString a;
	LPCTSTR p = s;
	for( int i = 0; i<n; ++i )
	{
		p = pxstrchr( p, ch );
		if( p == nullptr )
		{
			if( Warn )
				RAISE_AMN_EXCEPTION( TString( _T( "String section not found: [" ) ) + iToStr( n ) + _T( "]: " ) + s );
			return a;
		}
		p++;
		if( *p == 0 )
		{
			if( Warn )
				RAISE_AMN_EXCEPTION( TString( _T( "String section not found: [" ) ) + iToStr( n ) + _T( "]: " ) + s );
			return a;
		}
	}
	a = p;
	const TCHAR *p1 = pxstrchr( a, ch );
	if( p1 != nullptr )
		return a.Left( p1 - LPCTSTR( a ) );
	return a;
}

static CString GetStringSecNew( LPCTSTR s, int n1, BOOL Warn, TCHAR ch )
{
	static LPCTSTR pSavedS = nullptr;
	static LPCTSTR pStr = nullptr;
	static int oldN = -1;

	LPCTSTR p = s;
	int n = n1;
	if( s == pSavedS )
	{
		if( n >= oldN )
		{
			p = pStr;
			if( oldN >= 0 )
				n -= oldN;
		}
	}
	else
	{
		pSavedS = s;
		oldN = n1;
	}
	for (int i = 0; i<n; ++i)
	{
		if( p != nullptr )
			p = pxstrchr( p, ch );
		if (p == nullptr)
		{
			if (Warn)
				RAISE_AMN_EXCEPTION( TString(_T("String section not found: [")) + iToStr(n) + _T("]: ") + (s != nullptr ? s : _T("nullptr")));
			pStr = p;
			oldN = n1;
			return "";
		}
		p++;
		if (*p == 0)
		{
			if (Warn)
				RAISE_AMN_EXCEPTION( TString( _T("String section not found: [" )) + iToStr( n ) + _T("]: ") + ( s == nullptr ? _T("nullptr") : s) );
			pStr = p;
			oldN = n1;
			return "";
		}
	}

	pStr = p;
	oldN = n1;

	const TCHAR *p1 = pxstrchr( p, ch );
	if( p1 == nullptr )
		return p;
	int len = p1 - p;
	return CString( p, len);
}

CString AMNCORE_API Amn::Sys::Text::GetStringSec( LPCTSTR s, int n1, BOOL Warn, TCHAR ch )
{
	return GetStringSecNew( s, n1, Warn, ch );
}

CString AMNCORE_API Amn::Sys::Text::GetStringSec2( LPCTSTR s, int i )
{
	return GetStringSec( s, i, FALSE, '\t' );
}

ldouble Amn::Sys::Text::Conversion::_Fix( ldouble f, unsigned precesion )
{
	ldouble base = 10;
	int p = precesion;
	ldouble exp = pow( base, p );
	f *= exp;
	if (f >= 0)
		f += 0.5;
	else
		f -= 0.5;
	auto rem = modfl( f, &f );
	UNUSED_ALWAYS( rem );

	f /= exp;
	return f;
}

ldouble AMNCORE_API Amn::Sys::Text::Conversion::pFix( ldouble f, unsigned precesion )
{
	if (int( precesion ) == -1)
		precesion = SysFormat.iPricePrec;// GetFileOptions().iPricePrec;
	return _Fix( f, precesion );
}

ldouble AMNCORE_API Amn::Sys::Text::Conversion::qFix( ldouble f, unsigned precesion )
{
	if (int( precesion ) == -1)
		precesion = SysFormat.iQtyPrec;// GetFileOptions().iQtyPrec;
	return _Fix( f, precesion );
}


CString AMNCORE_API Amn::Sys::Text::Conversion::ToString( const Date& d )
{
	wostringstream os;
	LPCTSTR dateSep = L"-";
	switch (SysFormat.eDateFormat)
	{
	case Format::DF_LEADINGZERO:
		os << setfill(L'0') << setw(2) << d.Day() << dateSep
			<< setfill(L'0') << setw(2) << d.Month() << dateSep
			<< setfill(L'0') << setw(4) << d.Year();
		break;
	case Format::DF_NOLEADINGZERO:
		os << d.Day() << dateSep << d.Month() << dateSep << d.Year();
		break;
	case Format::DF_LEADINGZEROYY:
		os << setfill(L'0') << setw(4) << d.Year() << dateSep
			<< setfill(L'0') << setw(2) << d.Month() << dateSep
			<< setfill(L'0') << setw(2) << d.Day();
		break;
	case Format::DF_NOLEADINGZEROYY:
	//default:
		os << d.Year() << dateSep << d.Month() << dateSep << d.Day();
		break;
	}
	return os.str().c_str();

}

CString AMNCORE_API Amn::Sys::Text::Conversion::ToString( const Time& t )
{
	wostringstream os;
	TString Result;
	Date d = t;
	os << (LPCTSTR)ToString( d ) << L" - ";
	LPCTSTR timeSep = L":";
	switch (SysFormat.eDateFormat)
	{
	case Format::DF_LEADINGZEROYY:
	case Format::DF_LEADINGZERO:
		os << setfill(L'0') << setw(2) << t.Hour() << timeSep
			<< setfill(L'0') << setw(2) << t.Minute() << timeSep
			<< setfill(L'0') << setw(2) << t.Second();
		break;
	case Format::DF_NOLEADINGZERO:
	case Format::DF_NOLEADINGZEROYY:
		os << setfill(L'0') << setw(2) << t.Hour() << timeSep
			<< setfill(L'0') << setw(2) << t.Minute() << timeSep
			<< setfill(L'0') << setw(2) << t.Second();
		break;
	}
	return os.str().c_str();
}

template <class charT>
struct amn_punc : std::numpunct<charT> {
protected:
	charT do_decimal_point() const override	{ return _T('.'); }
	charT do_thousands_sep()  const override { return *SysFormat.szThSep; }
	string do_grouping() const override
	{ 
		if( SysFormat.bUseThSep )
			return "\3";
		else
			return "\000";
}
};

CString AMNCORE_API Amn::Sys::Text::Conversion::_ToStr( ldouble f, unsigned precesion )
{
	if( ( f == 0.0L ) && ( SysFormat.eUseZero == Format::ZEROTEXT ) )
		return SysFormat.szZeroText;
	ldouble v = _Fix( f, precesion );
	if( v == 0.0L )
		f = 0;
	std::wostringstream os;
	os.precision( precesion );
	if( f < 0 )
		os << L'-';
	os.imbue( locale( std::locale( "" ),  // use default locale
		// create no_separator facet based on german locale
		new amn_punc<TCHAR>() ) );
	os << std::fixed << std::fabs( f );
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::pToStr( ldouble f, unsigned precesion )
{
	if (int( precesion ) == -1)
		precesion = SysFormat.iPricePrec;
	return _ToStr( f, precesion );
}

CString AMNCORE_API Amn::Sys::Text::Conversion::qToStr( ldouble f, unsigned precesion )
{
	if (int( precesion ) == -1)
		precesion = SysFormat.iQtyPrec;
	return _ToStr( f, precesion );
}

CString AMNCORE_API Amn::Sys::Text::Conversion::_qToStr( ldouble v )
{
	if (qFix(v) != 0.0L)
		return qToStr( v );
	else
		return _T("");
}

CString AMNCORE_API Amn::Sys::Text::Conversion::_pToStr( ldouble v )
{
	if (pFix(v) != 0.0L)
		return pToStr( v );
	else
		return _T("");
}


int AMNCORE_API Amn::Sys::Text::Conversion::HexToInt( LPCTSTR str )
{
	int v = 0;
	_stscanf_s( str, _T("%x"), &v );
	return v;
}

WORD AMNCORE_API Amn::Sys::Text::Conversion::HexToWord( LPCTSTR str )
{
	WORD v = 0;
	_stscanf_s( str, _T("%hx"), &v );
	return v;
}

CString AMNCORE_API Amn::Sys::Text::Conversion::tmToStr( const Time& d )
{
	return iToStr( d.Hour() ) + _T(":") + iToStr( d.Minute() ) + _T(":") + iToStr( d.Second() );
}

CString AMNCORE_API Amn::Sys::Text::Conversion::iToHex( unsigned long v )
{
	wostringstream os;
	os << setfill( L'0' ) << setw( 8 ) << hex << v;
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::iToStr64( __int64 d )
{
	wostringstream os;
	os << dec << d;
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::uToStr64( unsigned __int64 d )
{
	wostringstream os;
	os << dec << d;
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::iToStr( INT32 d )
{
	wostringstream os;
	os << dec << d;
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::uToStr( DWORD d )
{
	wostringstream os;
	os << dec << d;
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::SQLdToStr( const Date& dt )
{
	LPCTSTR dtSep = L"-";
	wostringstream os;
	os << "'"
		<< setfill( L'0' ) << setw( 4 ) << dt.Year() << dtSep
		<< setfill( L'0' ) << setw( 2 ) << dt.Month() << dtSep
		<< setfill( L'0' ) << setw( 2 ) << dt.Day()
		<< "'";
	return os.str().c_str();
}

CString AMNCORE_API Amn::Sys::Text::Conversion::SQLdToStr2( const Date& dt )
{
	LPCTSTR dtSep = L"-";
	wostringstream os;
	os 
		<< setfill( L'0' ) << setw( 4 ) << dt.Year() << dtSep
		<< setfill( L'0' ) << setw( 2 ) << dt.Month() << dtSep
		<< setfill( L'0' ) << setw( 2 ) << dt.Day();
	return os.str().c_str();
}
