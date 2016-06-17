#include <Windows.h>

// Assigns the uID string resource to wsDest, returns length (0 if no resource)
inline int LoadString(std::wstring& wsDest, UINT uID, HINSTANCE hInstance = ::GetModuleHandle(nullptr))
{
    PWCHAR wsBuf; // no need to initialize
    wsDest.clear();
    if (size_t len = ::LoadStringW(hInstance, uID, reinterpret_cast<PWCHAR>(&wsBuf), 0))
        wsDest.assign(wsBuf, len);
    return wsDest.length();
}
 
// Assigns the uID string resource to sDest, returns length (0 if no resource)
inline int LoadString(std::string& sDest, UINT uID, HINSTANCE hInstance = ::GetModuleHandle(nullptr))
{
    PWCHAR wsBuf; // no need to initialize
    sDest.clear();
    if (size_t len = ::LoadStringW(hInstance, uID, reinterpret_cast<PWCHAR>(&wsBuf), 0) * sizeof WCHAR)
    {
        sDest.resize(++len); // make room for trailing '\0' in worst case
        sDest.resize(::LoadStringA(hInstance, uID, &*sDest.begin(), len));
    }
    return sDest.length();
}
 
// Returns a StringType with uID string resource content (empty if no resource)
template <class StringType>
inline StringType LoadString_(UINT uID, HINSTANCE hInstance)
{
    StringType sDest;
    return LoadString(sDest, uID, hInstance) ? sDest : StringType();
}
 
// Returns a std::string with uID string resource content (empty if no resource)
inline std::string LoadString_S(UINT uID, HINSTANCE hInstance = ::GetModuleHandle(nullptr))
{
    return LoadString_<std::string>(uID, hInstance);
}
 
// Returns a std::wstring with uID string resource content (empty if no resource)
inline std::wstring LoadString_W(UINT uID, HINSTANCE hInstance = ::GetModuleHandle(nullptr))
{
    return LoadString_<std::wstring>(uID, hInstance);
}
 
#ifdef UNICODE
    typedef std::wstring t_string;
#else
    typedef std::string t_string;
#endif

// Returns a UNICODE depending std::wstring or std::string, with uID string resource content (empty if no resource)
inline t_string LoadString_T(UINT uID, HINSTANCE hInstance = ::GetModuleHandle(nullptr))
{
    return LoadString_<t_string>(uID, hInstance);
}