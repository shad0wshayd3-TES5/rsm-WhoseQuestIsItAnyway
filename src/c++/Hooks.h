#pragma once

#include "NotificationManager.h"

namespace Hooks
{
	namespace
	{
		void Hook_DebugNotification(RE::InventoryEntryData* a_entryData, const char* a_soundToPlay, bool a_cancelIfAlreadyQueued)
		{
			RE::TESQuest* quest{ nullptr };
			if (a_entryData && a_entryData->extraLists) {
				for (auto& xList : *a_entryData->extraLists) {
					auto xAliasInstArr = xList->GetByType<RE::ExtraAliasInstanceArray>();
					if (xAliasInstArr) {
						for (auto& instance : xAliasInstArr->aliases) {
							if (instance->quest && instance->alias && instance->alias->IsQuestObject()) {
								quest = instance->quest;
								break;
							}
						}
					}
					if (quest) {
						break;
					}
				}
			}

			if (quest) {
				auto mgr = NotificationManager::GetSingleton();
				RE::DebugNotification(mgr->BuildNotification(quest).c_str(), a_soundToPlay, a_cancelIfAlreadyQueued);
			} else {
				static auto dropQuestItemWarning = RE::GameSettingCollection::GetSingleton()->GetSetting("sDropQuestItemWarning");
				RE::DebugNotification(dropQuestItemWarning->GetString(), a_soundToPlay, a_cancelIfAlreadyQueued);
			}
		}

		class ContainerMenuEx :
			public RE::ContainerMenu
		{
		public:
			static void Hook_ItemDrop(const RE::FxDelegateArgs& a_params)
			{
				if (auto Handler = a_params.GetHandler(); Handler) {
					if (auto ContainerMenu = reinterpret_cast<RE::ContainerMenu*>(Handler); ContainerMenu && ContainerMenu->itemList) {
						auto entryData = ContainerMenu->itemList->GetSelectedEntry();
						if (entryData && entryData->IsQuestObject()) {
							Hook_DebugNotification(entryData, "", true);
							return;
						}
					}
				}

				_ItemDrop(a_params);
			}

			static inline void InstallHooks()
			{
				struct Patch : Xbyak::CodeGenerator
				{
					Patch(void* buf) :
						Xbyak::CodeGenerator(20, buf)
					{
						Xbyak::Label retnLabel;

						push(rbx);
						push(rbp);
						sub(rsp, 0x48);
						jmp(ptr[rip + retnLabel]);

						L(retnLabel);
						dq(target.address() + 7);
					}
				};

				auto& trampoline = SKSE::GetTrampoline();
				void* codeBuf = trampoline.allocate(20);
				Patch patch{ codeBuf };

				_ItemDrop = reinterpret_cast<decltype(_ItemDrop)>(codeBuf);
				trampoline.write_branch<6>(target.address(), Hook_ItemDrop);

				logger::info("Installed drop hook"sv);
			}

			static inline REL::Relocation<std::uintptr_t> target{ REL::ID(51857) };
			static inline decltype(&ContainerMenuEx::Hook_ItemDrop) _ItemDrop;
		};

		// This is the original InstallDropHook with some updated IDs.
		// But the asm changed in a way that I don't quite understand, so I've disabled it and replaced it with the ContainerMenuEx hook.
		/*
		void InstallDropHook()
		{
			constexpr std::size_t CAVE_SIZE = 7;
			constexpr std::size_t MOV_HOOK = 0x3D;
			constexpr std::size_t POP_START = 0x4E;
			constexpr std::size_t JMP_HOOK = 0x54;
			constexpr std::size_t CALL_BACK = 0x108;

			REL::Relocation<std::uintptr_t> funcBase{ REL::ID(51857) };

			// Move InventoryEntryData into rcx
			{
				struct Patch : Xbyak::CodeGenerator
				{
					Patch() :
						Xbyak::CodeGenerator(CAVE_SIZE)
					{
						mov(rcx, rax);	// rax = InventoryEntryData*
						nop(4);
					}
				};

				Patch patch;
				REL::safe_write(funcBase.address() + MOV_HOOK, patch.getCode(), patch.getSize());
			}

			// Prevent the function from popping off the stack
			REL::safe_fill(funcBase.address() + POP_START, REL::NOP, JMP_HOOK - POP_START);

			// Detour the jump
			{
				struct Patch : Xbyak::CodeGenerator
				{
					Patch(std::uintptr_t a_callAddr, std::uintptr_t a_retAddr) :
						Xbyak::CodeGenerator()
					{
						Xbyak::Label callLbl;
						Xbyak::Label retLbl;

						call(ptr[rip + callLbl]);
						jmp(ptr[rip + retLbl]);

						L(callLbl);
						dq(a_callAddr);

						L(retLbl);
						dq(a_retAddr);
					}
				};

				Patch patch(reinterpret_cast<std::uintptr_t>(&Hook_DebugNotification), funcBase.address() + CALL_BACK);

				auto& trampoline = SKSE::GetTrampoline();
				trampoline.write_branch<5>(funcBase.address() + JMP_HOOK, patch.getCode());
			}

			logger::info("Installed drop hook"sv);
		}
		*/

		void InstallStoreHook()
		{
			constexpr std::size_t CAVE_SIZE = 7;
			constexpr std::size_t MOV_HOOK = 0x3A4;
			constexpr std::size_t CALL_HOOK = 0x3AB;

			REL::Relocation<std::uintptr_t> funcBase{ REL::ID(51141) };

			struct Patch : Xbyak::CodeGenerator
			{
				Patch() :
					Xbyak::CodeGenerator(CAVE_SIZE)
				{
					mov(rcx, r12);	// r12 = InventoryEntryData*
					nop(4);
				}
			};

			Patch patch;
			REL::safe_write(funcBase.address() + MOV_HOOK, patch.getCode(), patch.getSize());

			auto& trampoline = SKSE::GetTrampoline();
			trampoline.write_call<5>(funcBase.address() + CALL_HOOK, Hook_DebugNotification);
			logger::info("Installed store hook"sv);
		}
	}

	void Install()
	{
		ContainerMenuEx::InstallHooks();
		InstallStoreHook();

		logger::info("Hooks installed"sv);
	}
}
