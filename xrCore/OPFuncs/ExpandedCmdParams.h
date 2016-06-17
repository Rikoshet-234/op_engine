#ifndef ExpandedCmdParamsH
#define ExpandedCmdParamsH

namespace OPFuncs
{
	template <typename TKey,typename TValue>
	class createMap
	{
	private:
		std::map<TKey,TValue> _map;
	public:
		createMap(const TKey& key, const TValue& val)
		{
			_map[key] = val;
		}

		createMap<TKey, TValue>& operator()(const TKey& key, const TValue& val)
		{
			_map[key] = val;
			return *this;
		}

		operator std::map<TKey, TValue>()
		{
			return _map;
		}
	};

	typedef std::map<std::string, int> ParamsMap;

	class XRCORE_API ExpandedCmdParams
	{
		public:
			enum KnownParams
			{
				dTHMLoad				=1<<0	, //показывать имя thm файла при загрузке 
				dFileSystemVars			=1<<1	, //показывать вычитанные переменные FS $__$
				dClassScriptRegister	=1<<2	,//показывать детали о регистрации классов
				dMaterialsLoad			=1<<3	,//показыват детали о загрузке материалов
				dScriptsLoad			=1<<4	,//показываает процесс загрузки скриптов
			    dSpawnRegistryLoad		=1<<5	,//показывать загрузку spawn registry
				dAlifeObjectRegister	=1<<6	,  //показывать добавление объектов в alife story registry
				dTextureLoad			=1<<7	,   //показывать загрузку текстур
				dMapLoad				=1<<8	, //показывать загрузку карт
				dMapLoadErrors			=1<<9	,//показывать ошибки при загрузке карт
				dFileSystem				=1<<10	,//показывать загрузку игровых архивов
				dSoundPrefixNotPresent  =1<<11	, //показывать отсутсвующие звуки для звуковых префиксов 
				pShowLogTime			=1<<15	//показывать время в логе
				//-dumpall добавляет все параметры
			};	
			
			ExpandedCmdParams();
			void ParseCmdParams(std::string commandLine);
			bool isParamSet(int param) const;
			bool isAnyParamsSet(int paramsMask) const;
		private:
			Flags32	paramFlags;
			ParamsMap knownCmdParams;
	};

	extern XRCORE_API ExpandedCmdParams* Dumper;
}

#endif