#pragma once

#include "Settings.h"

class NotificationManager
{
public:
	static inline NotificationManager* GetSingleton()
	{
		static NotificationManager singleton;
		return &singleton;
	}

	inline std::string BuildNotification(RE::TESQuest* a_quest)
	{
		std::string msg = "$WHQA_Msg";
		if (*Settings::useRandomMessages) {
			std::snprintf(_buf, sizeof(_buf), "%02lli", (_rng() % *Settings::totalMessageCount));
			msg += _buf;
		} else {
			msg += "00";
		}
		msg += "{";

		auto foundQuestName = false;
		if (!a_quest->fullName.empty()) {
			msg += a_quest->fullName.c_str();
			foundQuestName = true;
		} else if (!a_quest->formEditorID.empty()) {
			msg += a_quest->formEditorID.c_str();
			foundQuestName = true;
		}

		if (*Settings::printQuestFormID) {
			if (foundQuestName) {
				msg += " ";
			}
			std::snprintf(_buf, sizeof(_buf), "[0x%08X]", a_quest->GetFormID());
			msg += _buf;
		}

		msg += "}";
		return msg;
	}

protected:
	NotificationManager() :
		_rng(std::random_device()()),
		_buf{}
	{}

	NotificationManager(const NotificationManager&) = delete;
	NotificationManager(NotificationManager&&) = delete;
	~NotificationManager() = default;

	NotificationManager& operator=(const NotificationManager&) = delete;
	NotificationManager& operator=(NotificationManager&&) = delete;

	std::mt19937 _rng;
	char _buf[1000];
};
