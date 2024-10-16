#include "Hooks.h"
#include "Language.h"

namespace Hooks
{
	static void do_replacement(std::string& text, const srell::regex& pattern, RE::TESBoundObject* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
	{
		srell::smatch match;

		std::istringstream iss(text);
		std::ostringstream oss;
		std::string        line;
		while (std::getline(iss, line)) {
			if (srell::regex_search(line, match, pattern)) {
				oss << Language::GetOutput(a_activator, a_this, match);
			} else {
				oss << line;
			}
			oss << '\n';
		}

		a_dst = string::trim_copy(oss.str());
	}

	namespace TESObjectCONT
	{
		struct GetActivateText
		{
			static bool thunk(RE::TESObjectCONT* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
			{
				auto result = func(a_this, a_activator, a_dst);

				if (Language::gameLanguageHash == "ENGLISH"_h) {
					return result;
				}

				if (std::string text = a_dst.c_str(); text.contains("'s ")) {				
					static srell::regex pattern(R"(([\S\s]+?)'s\s([\S\s]+))");
					do_replacement(text, pattern, a_this, a_activator, a_dst);
				}

				return result;
			}
			static inline REL::Relocation<decltype(thunk)> func;

			static inline constexpr std::size_t size = 0x4C;
		};
	}

	namespace TESNPC
	{
		struct GetActivateText
		{
			static bool thunk(RE::TESNPC* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
			{
				auto result = func(a_this, a_activator, a_dst);

				if (Language::gameLanguageHash == "ENGLISH"_h) {
					return result;
				}

				if (std::string text = a_dst.c_str(); text.contains("'s ") || text.contains(" - ")) {				
					srell::regex pattern(
						text.contains("'s ") ?
							R"(([\S\s]+?)'s\s([\S\s]+))" :
							R"(([\S\s]+?)\s-\s([\S\s]+))");
					do_replacement(text, pattern, a_this, a_activator, a_dst);
				}

				return result;
			}
			static inline REL::Relocation<decltype(thunk)> func;

			static inline constexpr std::size_t size = 0x4C;
		};
	}

	void Install()
	{
		logger::info("{:*^30}", "HOOKS");

		stl::write_vfunc<RE::TESObjectCONT, TESObjectCONT::GetActivateText>();
		logger::info("Installed container name hook");
		
		if (Language::doNPCReplacement) {
			stl::write_vfunc<RE::TESNPC, TESNPC::GetActivateText>();
			logger::info("Installed NPC name hook");
		}
	}
}
