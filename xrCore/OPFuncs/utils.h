#ifndef xrsUtilsH
#define xrsUtilsH


namespace OPFuncs {
	enum STR2INT_ERROR { SUCCESS, OVERRANGE, UNDERRANGE, INCONVERTIBLE };
	XRCORE_API std::string tolower(const char * str);
	XRCORE_API std::string toupper(const char * str);

	XRCORE_API inline void ltrim(std::string &);
	XRCORE_API inline void rtrim(std::string &);
	XRCORE_API inline void trim(std::string &);

	XRCORE_API inline void ltrimq(std::string &);
	XRCORE_API inline void rtrimq(std::string &);
	XRCORE_API inline void trimq(std::string &);

	template<class T> 
	XRCORE_API STR2INT_ERROR str2int(T &, char const *);

	template XRCORE_API STR2INT_ERROR str2int<int>(int &, char const*);
	template XRCORE_API STR2INT_ERROR str2int<u16>(u16 &, char const*);
	template XRCORE_API STR2INT_ERROR str2int<u32>(u32 &, char const*);

	template <typename T>
	std::string NumberToString ( T Number )
	{
		std::ostringstream ss;
		ss << Number;
		return ss.str();
	}

}
#endif