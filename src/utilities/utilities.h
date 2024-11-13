#pragma once

namespace Utilities
{
	namespace EDID
	{
		using _GetFormEditorID = const char* (*)(std::uint32_t);

		inline std::string GetEditorID(const RE::TESForm* a_form)
		{
			switch (a_form->GetFormType()) {
			case RE::FormType::Keyword:
			case RE::FormType::LocationRefType:
			case RE::FormType::Action:
			case RE::FormType::MenuIcon:
			case RE::FormType::Global:
			case RE::FormType::HeadPart:
			case RE::FormType::Race:
			case RE::FormType::Sound:
			case RE::FormType::Script:
			case RE::FormType::Navigation:
			case RE::FormType::Cell:
			case RE::FormType::WorldSpace:
			case RE::FormType::Land:
			case RE::FormType::NavMesh:
			case RE::FormType::Dialogue:
			case RE::FormType::Quest:
			case RE::FormType::Idle:
			case RE::FormType::AnimatedObject:
			case RE::FormType::ImageAdapter:
			case RE::FormType::VoiceType:
			case RE::FormType::Ragdoll:
			case RE::FormType::DefaultObject:
			case RE::FormType::MusicType:
			case RE::FormType::StoryManagerBranchNode:
			case RE::FormType::StoryManagerQuestNode:
			case RE::FormType::StoryManagerEventNode:
			case RE::FormType::SoundRecord:
				return a_form->GetFormEditorID();
			default:
			{
				static auto tweaks = SKSE::WinAPI::GetModuleHandle(L"po3_Tweaks");
				static auto func = reinterpret_cast<_GetFormEditorID>(SKSE::WinAPI::GetProcAddress(tweaks, "GetFormEditorID"));
				if (func) {
					return func(a_form->formID);
				}
				return {};
			}
			}
		}
	}

	namespace Singleton
	{
		template <class T>
		class ISingleton
		{
		public:
			static T* GetSingleton()
			{
				static T singleton;
				return std::addressof(singleton);
			}

			ISingleton(const ISingleton&) = delete;
			ISingleton(ISingleton&&) = delete;
			ISingleton& operator=(const ISingleton&) = delete;
			ISingleton& operator=(ISingleton&&) = delete;

		protected:
			ISingleton() = default;
			~ISingleton() = default;
		};
	}

	namespace String
	{

		// Credit: https://github.com/powerof3/CLibUtil
		inline std::vector<std::string> split(const std::string& a_str, std::string_view a_delimiter)
		{
			auto range = a_str | std::ranges::views::split(a_delimiter) | std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
			return { range.begin(), range.end() };
		}

		inline bool is_only_hex(std::string_view a_str, bool a_requirePrefix = true)
		{
			if (!a_requirePrefix) {
				return std::ranges::all_of(a_str, [](unsigned char ch) {
					return std::isxdigit(ch);
					});
			}
			else if (a_str.compare(0, 2, "0x") == 0 || a_str.compare(0, 2, "0X") == 0) {
				return a_str.size() > 2 && std::all_of(a_str.begin() + 2, a_str.end(), [](unsigned char ch) {
					return std::isxdigit(ch);
					});
			}
			return false;
		}

		template <class T>
		T to_num(const std::string& a_str, bool a_hex = false)
		{
			const int base = a_hex ? 16 : 10;

			if constexpr (std::is_same_v<T, double>) {
				return static_cast<T>(std::stod(a_str, nullptr));
			}
			else if constexpr (std::is_same_v<T, float>) {
				return static_cast<T>(std::stof(a_str, nullptr));
			}
			else if constexpr (std::is_same_v<T, std::int64_t>) {
				return static_cast<T>(std::stol(a_str, nullptr, base));
			}
			else if constexpr (std::is_same_v<T, std::uint64_t>) {
				return static_cast<T>(std::stoull(a_str, nullptr, base));
			}
			else if constexpr (std::is_signed_v<T>) {
				return static_cast<T>(std::stoi(a_str, nullptr, base));
			}
			else {
				return static_cast<T>(std::stoul(a_str, nullptr, base));
			}
		}

		inline std::string tolower(std::string_view a_str)
		{
			std::string result(a_str);
			std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
			return result;
		}

		inline bool replace_all(std::string& a_str, std::string_view a_search, std::string_view a_replace)
		{
			if (a_search.empty()) {
				return false;
			}

			std::size_t pos = 0;
			bool wasReplaced = false;
			while ((pos = a_str.find(a_search, pos)) != std::string::npos) {
				a_str.replace(pos, a_search.length(), a_replace);
				pos += a_replace.length();
				wasReplaced = true;
			}

			return wasReplaced;
		}
	}

	namespace Forms
	{
		template <typename T>
		T* GetFormFromString(const std::string& a_str)
		{
			if (const auto splitID = String::split(a_str, "|"); splitID.size() == 2) {
				if (!String::is_only_hex(splitID[1])) return nullptr;
				const auto  formID = String::to_num<RE::FormID>(splitID[1], true);

				const auto& modName = splitID[0];
				if (!RE::TESDataHandler::GetSingleton()->LookupModByName(modName)) return nullptr;

				return RE::TESDataHandler::GetSingleton()->LookupForm<T>(formID, modName);
			}
			return RE::TESForm::LookupByEditorID<T>(a_str);
		}
	}
}