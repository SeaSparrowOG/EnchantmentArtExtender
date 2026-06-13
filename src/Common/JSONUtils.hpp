#pragma once

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
}