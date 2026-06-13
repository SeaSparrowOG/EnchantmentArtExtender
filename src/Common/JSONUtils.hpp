#pragma once

#include <ClibUtil/string.hpp>
#include <ClibUtil/editorID.hpp>

namespace JSONUtils
{
	inline std::string GetObjectType(const Json::Value& a_val) {
		switch (a_val.type()) {
		case Json::ValueType::arrayValue: return "Array";
		case Json::ValueType::booleanValue: return "Bool";
		case Json::ValueType::intValue: return "Int";
		case Json::ValueType::nullValue: return "Null";
		case Json::ValueType::objectValue: return "Object";
		case Json::ValueType::realValue: return "Float/Double";
		case Json::ValueType::stringValue: return "String";
		case Json::ValueType::uintValue: return "UInt";
		default: return "Invalid/Unknown";
		}
	}

    // Helper function for extracting forms from a string
    template <typename T>
    struct form_type
    {
        static constexpr RE::FormType value = RE::FormType::None;
    };

    template <>
    struct form_type<RE::BGSKeyword> { static constexpr RE::FormType value = RE::FormType::Keyword; };
    template <>
    struct form_type<RE::BGSLocationRefType> { static constexpr RE::FormType value = RE::FormType::LocationRefType; };
    template<>
    struct form_type<RE::TESGlobal> { static constexpr RE::FormType value = RE::FormType::Global; };
    template<>
    struct form_type<RE::TESRace> { static constexpr RE::FormType value = RE::FormType::Race; };
    template<>
    struct form_type<RE::TESSound> { static constexpr RE::FormType value = RE::FormType::Sound; };
    template<>
    struct form_type<RE::TESObjectCELL> { static constexpr RE::FormType value = RE::FormType::Cell; };
    template<>
    struct form_type<RE::TESWorldSpace> { static constexpr RE::FormType value = RE::FormType::WorldSpace; };
    template<>
    struct form_type<RE::TESQuest> { static constexpr RE::FormType value = RE::FormType::Quest; };
    template<>
    struct form_type<RE::TESIdleForm> { static constexpr RE::FormType value = RE::FormType::Idle; };
    template<>
    struct form_type<RE::TESObjectANIO> { static constexpr RE::FormType value = RE::FormType::AnimatedObject; };
    template<>
    struct form_type<RE::TESImageSpaceModifier> { static constexpr RE::FormType value = RE::FormType::ImageAdapter; };
    template<>
    struct form_type<RE::BGSVoiceType> { static constexpr RE::FormType value = RE::FormType::VoiceType; };
    template<>
    struct form_type<RE::BGSMusicType> { static constexpr RE::FormType value = RE::FormType::MusicType; };
    template<>
    struct form_type<RE::BGSSoundDescriptorForm> { static constexpr RE::FormType value = RE::FormType::SoundRecord; };

    constexpr bool SupportsEDIDWithoutTweaks(RE::FormType type)
    {
        switch (type) {
        case RE::FormType::Keyword:
        case RE::FormType::LocationRefType:
        case RE::FormType::Global:
        case RE::FormType::Race:
        case RE::FormType::Sound:
        case RE::FormType::Cell:
        case RE::FormType::WorldSpace:
        case RE::FormType::Quest:
        case RE::FormType::Idle:
        case RE::FormType::AnimatedObject:
        case RE::FormType::ImageAdapter:
        case RE::FormType::VoiceType:
        case RE::FormType::MusicType:
        case RE::FormType::SoundRecord:
            return true;
        default:
            return false;
        }
    }

    enum class QueryResult
    {
        Success,          // Success
        FormatError,      // String is in an invalid format (EditorID while PO3's Tweaks is not present, FormID not hex, etc)
        FileNotFound,     // ESP/ESM/ESL missing
        FormNotInFile,    // Master exists, but form is not present
        WrongFormtype,    // Form exists in given file, but type is wrong.
        NoForm,           // Form simply not found

        MissingPo3Tweaks, // EditorID query that requires PO3's tweaks but PO3's tweaks is not present.
        GenericFailure    // Catchall (might be missing data handler, cosmic ray, etc)
    };

    template <typename T>
    struct QueryData
    {
        std::optional<T*> value;
        QueryResult status;
    };


    template <typename T>
    QueryData<T> GetFormFromString(const std::string& a_str) {
        constexpr bool supportsEDID = SupportsEDIDWithoutTweaks(form_type<T>::value);

        auto response = QueryData<T>{ std::nullopt, QueryResult::Success };

        static auto* dh = RE::TESDataHandler::GetSingleton();
        if (!dh) {
            response.status = QueryResult::GenericFailure;
            return response;
        }

        static auto* tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks.dll");

        auto parts = clib_util::string::split(a_str, "|");
        std::string modName = "";
        RE::FormID formID = 0;
        RE::TESForm* form = nullptr;
        T* castForm = nullptr;

        switch (parts.size()) {
        case 1:
            // EDID
            if constexpr (!supportsEDID) {
                if (!tweaks) {
                    response.status = QueryResult::MissingPo3Tweaks;
                    return response;
                }
            }
            form = RE::TESForm::LookupByEditorID(a_str);
            if (!form) {
                response.status = QueryResult::NoForm;
                return response;
            }
            castForm = form->As<T>();
            if (!castForm) {
                response.status = QueryResult::WrongFormtype;
                return response;
            }
            response.value = castForm;
            return response;
        case 2:
            // FormID
            if (clib_util::string::is_only_hex(parts[1])) {
                formID = clib_util::string::to_num<RE::FormID>(parts[1], true);
                modName = parts[0];
            }
            else if (clib_util::string::is_only_hex(parts[0])) {
                formID = clib_util::string::to_num<RE::FormID>(parts[0], true);
                modName = parts[1];
            }
            else {
                response.status = QueryResult::FormatError;
                return response;
            }
            if (!dh->LookupModByName(modName)) {
                response.status = QueryResult::FileNotFound;
                return response;
            }

            form = dh->LookupForm(formID, modName);
            if (!form) {
                response.status = QueryResult::FormNotInFile;
                return response;
            }
            castForm = form->As<T>();
            if (!castForm) {
                response.status = QueryResult::WrongFormtype;
                return response;
            }
            response.value = castForm;
            return response;
        default:
            response.status = QueryResult::FormatError;
            break;
        }
        return response;
    }

    enum class ValueStatus
    {
        Success,

        Empty,
        FormatError,
        InvalidForm,
        GenericFailure
    };

    std::string ValueResultToString(ValueStatus val) {
        switch (val) {
        case ValueStatus::Success: return "Success";
        case ValueStatus::Empty: return "Empty";
        case ValueStatus::FormatError: return "FormatError";
        case ValueStatus::InvalidForm: return "InvalidForm";
        case ValueStatus::GenericFailure: return "GenericFailure";
        default: return "GenericFailure";
        }
    }

    template <typename T>
    struct ValueResult
    {
        std::optional<std::vector<T*>> _data   = std::nullopt;
        ValueStatus                    _status = ValueStatus::Success;
    };

    template <typename T>
    ValueResult<T> GetFormsFromValue(const Json::Value& from) {
        ValueResult<T> result;
        std::vector<T*> foundForms;
        result._data = std::nullopt;
        result._status = ValueStatus::Success;

        if (from.isString()) {
            QueryData<T> query = GetFormFromString<T>(from.asString());
            if (query.status == QueryResult::Success) {
                if (query.value.has_value()) {
                    foundForms.emplace_back(query.value.value());
                }
            }
            else {
                switch (query.status) {
                case QueryResult::NoForm:
                case QueryResult::FileNotFound:
                    break;
                case QueryResult::FormatError:
                case QueryResult::MissingPo3Tweaks:
                    if (result._status < ValueStatus::FormatError) {
                        result._status = ValueStatus::FormatError;
                    }
                    break;
                case QueryResult::WrongFormtype:
                case QueryResult::FormNotInFile:
                    if (result._status < ValueStatus::InvalidForm) {
                        result._status = ValueStatus::InvalidForm;
                    }
                    break;
                default:
                    if (result._status < ValueStatus::GenericFailure) {
                        result._status = ValueStatus::GenericFailure;
                    }
                    break;
                }
            }
        }
        else if (from.isArray()) {
            for (const auto& arrVal : from) {
                if (arrVal.isString()) {
                    QueryData<T> query = GetFormFromString<T>(arrVal.asString());
                    if (query.status == QueryResult::Success) {
                        if (query.value.has_value()) {
                            foundForms.emplace_back(query.value.value());
                        }
                    }
                    else {
                        switch (query.status) {
                        case QueryResult::NoForm:
                        case QueryResult::FileNotFound:
                            break;
                        case QueryResult::FormatError:
                        case QueryResult::MissingPo3Tweaks:
                            if (result._status < ValueStatus::FormatError) {
                                result._status = ValueStatus::FormatError;
                            }
                            break;
                        case QueryResult::WrongFormtype:
                        case QueryResult::FormNotInFile:
                            if (result._status < ValueStatus::InvalidForm) {
                                result._status = ValueStatus::InvalidForm;
                            }
                            break;
                        default:
                            if (result._status < ValueStatus::GenericFailure) {
                                result._status = ValueStatus::GenericFailure;
                            }
                            break;
                        }
                    }
                }
                else {
                    if (result._status < ValueStatus::FormatError) {
                        result._status = ValueStatus::FormatError;
                    }
                    return result;
                }

                if (result._status > ValueStatus::Success) {
                    return result;
                }
            }
        }
        else {
            if (result._status < ValueStatus::FormatError) {
                result._status = ValueStatus::FormatError;
            }
            return result;
        }

        if (result._status != ValueStatus::Success) {
            return result;
        }
        if (foundForms.empty()) {
            result._status = ValueStatus::Empty;
            return result;
        }

        result._status = ValueStatus::Success;
        result._data = std::move(foundForms);
        return result;
    }
}