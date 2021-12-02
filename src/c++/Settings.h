#pragma once

#include <Json2Settings.h>

class Settings
{
public:
	Settings() = delete;

	static bool LoadSettings(bool a_dumpParse = false)
	{
		auto [log, success] = Json2Settings::load_settings(FILE_NAME, a_dumpParse);
		if (!log.empty()) {
			logger::error("{:s}", log.c_str());
		}
		return success;
	}

	static inline Json2Settings::bSetting printQuestFormID{ "printQuestFormID", false };
	static inline Json2Settings::bSetting useRandomMessages{ "useRandomMessages", false };
	static inline Json2Settings::iSetting totalMessageCount{ "totalMessageCount", 5 };

private:
	static constexpr char FILE_NAME[] = "Data\\SKSE\\Plugins\\WhoseQuestIsItAnyway.json";
};
