#pragma once

#define STRING 0
#define LIST 1
#define OLIST 2

#include <string>
#include <unordered_map>

struct setting {
	std::string key;
	int val_type;
};

class ConfigReader {
	public:
		ConfigReader(const std::string &path);
		bool read();
		bool write();
		std::string getSetting(const std::string &setting);
	private:
		const std::string m_path;
		std::unordered_map<std::string, std::string> m_settings;
		bool readString();
		bool readList();
		bool readOList();
};
