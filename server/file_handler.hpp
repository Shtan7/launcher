#pragma once
#include <vector>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <tuple>
#include "common.hpp"

namespace launcher
{
	class file_handler
	{
	private:
		// Key is file name, first in pair is an absolute path, second in pair is a sha512 hash in base64 encoding.
		std::map<std::string, std::pair<std::string, std::string>> file_list;
		std::string folder_name;
		std::string general_file_hash_base64;

	public:
		/// <summary>
		/// Create file handler module.
		/// </summary>
		/// <param name="folder_name_">Working directory for the module.</param>
		file_handler(std::string folder_name_);

		/// <summary>
		/// Compare total hash of all files.
		/// </summary>
		/// <param name="hash">Hash in base64 encoding.</param>
		/// <returns>Result of comparison (true \ false).</returns>
		bool compare_general_hash(std::string& hash);

		/// <summary>
		/// File list getter.
		/// </summary>
		/// <returns>The map with all files inside the working directory.</returns>
		const std::map<std::string, std::pair<std::string, std::string>>& get_file_list();

		/// <summary>
		/// Working folder getter.
		/// </summary>
		/// <returns>The folder name of working directory.</returns>
		std::string get_folder_name();

		/// <summary>
		/// General hash getter.
		/// </summary>
		/// <returns>Hash of all files.</returns>
		std::string get_general_hash();

		/// <summary>
		/// Perform hashing of files in working directory.
		/// </summary>
		void perform_hashing();
	};
}