#include "file_handler.hpp"
#include <filesystem>
#include <openssl/sha.h>
#include <fstream>
#include <iostream>
#include <boost/beast/core/detail/base64.hpp>

namespace launcher
{
	file_handler::file_handler(std::string folder_name_) : folder_name{ std::move(folder_name_) }
	{
		std::filesystem::create_directory(folder_name); // create directory if it doesn't exists
		general_file_hash_base64.resize(SHA512_DIGEST_LENGTH);
		perform_hashing();
	}

	const std::map<std::string, std::pair<std::string, std::string>>& file_handler::get_file_list()
	{
		return file_list;
	}

	bool file_handler::compare_general_hash(std::string& hash)
	{
		return general_file_hash_base64.compare(hash) ? false : true;
	}

	std::string file_handler::get_folder_name()
	{
		return folder_name;
	}

	std::string file_handler::get_general_hash()
	{
		return general_file_hash_base64;
	}

	void file_handler::perform_hashing()
	{
		for (auto&& dir_entry : std::filesystem::directory_iterator{ folder_name })
		{
			if (dir_entry.is_regular_file())
			{
				std::string hash;
				hash.resize(SHA512_DIGEST_LENGTH);

				std::string full_path = std::filesystem::absolute(dir_entry).string();
				std::string file_name = std::filesystem::path{ full_path }.filename().string();

				uint64_t file_size = std::filesystem::file_size(full_path);

				std::ifstream file_input{ full_path };
				if (!file_input)
				{
					throw std::runtime_error{ ("failed to open file: " + file_name).c_str() };
				}

				SHA512_CTX sha512;
				SHA512_Init(&sha512);

				std::vector<uint8_t> file_buffer(common::consts::MiB);

				do
				{
					if (file_size < file_buffer.size())
					{
						file_buffer.resize(file_size);
					}

					file_input.read(reinterpret_cast<char*>(file_buffer.data()), file_buffer.size());

					SHA512_Update(&sha512, file_buffer.data(), file_buffer.size());

					file_size -= file_buffer.size();

				} while (file_size);

				SHA512_Final(reinterpret_cast<uint8_t*>(hash.data()), &sha512);

				hash = common::get_base64_from_sha512(hash);

				file_list[file_name] = { full_path, hash };
			}
		}

		SHA512_CTX sha512;
		SHA512_Init(&sha512);

		for (auto&& file : file_list)
		{
			SHA512_Update(&sha512, file.second.second.data(), file.second.second.size());
		}

		SHA512_Final(reinterpret_cast<uint8_t*>(general_file_hash_base64.data()), &sha512);

		std::string temp;
		temp.resize(common::consts::SHA512_in_base64_size);
		boost::beast::detail::base64::encode(temp.data(), general_file_hash_base64.data(), 64);

		general_file_hash_base64 = std::move(temp);
	}
}