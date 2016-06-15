#ifndef script_engine_version
#define script_engine_version
#pragma once

#include <string>
#include <vector>

#define SCRIPT_ENGINE_VERSION_MINOR 1
#define SCRIPT_ENGINE_VERSION_MAJOR 7

std::string get_sev_full() 
{
	return std::to_string(SCRIPT_ENGINE_VERSION_MINOR)+"."+std::to_string(SCRIPT_ENGINE_VERSION_MAJOR);
}

std::vector<std::string> get_sev_parts()
{
	std::vector<std::string> parts;
	parts.push_back(std::to_string(SCRIPT_ENGINE_VERSION_MINOR));
	parts.push_back(std::to_string(SCRIPT_ENGINE_VERSION_MAJOR));
	return parts;
}

#endif