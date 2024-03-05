#pragma once

#define DEBUG_LOG(flag, message) \
if (flag) { std::cerr << message; }

#define DEBUG_CALL(flag, f) \
if (flag) { f; }

#define DECLARE_STR_TYPE(classname) \
std::string type() { return #classname; } 


#define BWAPI_LOG_IF_ERROR() \
if (BWAPI::Broodwar->getLastError() != BWAPI::Errors::None) {\
    std::cerr << "error from bwapi " << BWAPI::Broodwar->getLastError() << std::endl;\
}