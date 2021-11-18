#include "common.hpp"
#include <openssl/sha.h>
#include <boost/beast/core/detail/base64.hpp>

namespace launcher
{
	std::string common::hash_string(std::string& input)
	{
		std::string output;
		output.reserve(SHA512_DIGEST_LENGTH);

		SHA512_CTX sha512;
		SHA512_Init(&sha512);
		SHA512_Update(&sha512, input.data(), input.size());
		SHA512_Final(reinterpret_cast<uint8_t*>(output.data()), &sha512);

		for (int j = 0; j < SHA512_DIGEST_LENGTH; j++)
		{
			output[j] += j * 3; // simple static salt
		}

		return output;
	}

	std::ostream& operator<< (std::ostream& os, const messages::status_codes& obj)
	{
		os << static_cast<std::underlying_type<messages::status_codes>::type>(obj);
		return os;
	}

	std::istream& operator>> (std::istream& os, messages::request_ids& obj)
	{
		int i;
		os >> i;
		obj = (messages::request_ids)(i);
		return os;
	}

	std::pair<std::string, std::string> messages::request::get_login_pass()
	{
		return std::make_pair(request_content[0], request_content[1]);
	}

	std::string& messages::request::get_general_hash()
	{
		return request_content[0];
	}

	std::vector<std::pair<std::string, std::string>> messages::request::get_files_hash()
	{
		std::vector<std::pair<std::string, std::string>> file_list;
		
		uint32_t files_num = request_content.size() / 2;

		for (int j = 0; j < files_num; j++)
		{
			file_list.emplace_back(std::move(request_content[j * 2]), std::move(request_content[j * 2 + 1]));
		}

		return file_list;
	}

	messages::request::request(request&& obj) : request_id{obj.request_id}, request_content{std::move(obj.request_content)}
	{}

	messages::request::request(messages::request_ids request_id_, std::vector<std::string> request_content_) : request_id{ request_id_ }, request_content{ std::move(request_content_) }
	{}

	/// <summary>
	/// Perform status code translation to text form.
	/// </summary>
	/// <param name="str">Output string reference.</param>
	/// <param name="code">Status code to translate.</param>
	void translate_err_code_to_msg(std::string& str, messages::status_codes code)
	{
		switch (code)
		{
			case messages::status_codes::incorrect_input:
			{
				str = "incorrect input";
				break;
			}
			case messages::status_codes::hash_miss:
			{
				str = "hash miss";
				break;
			}
			case messages::status_codes::not_authorized:
			{
				str = "not authorized";
				break;
			}
			case messages::status_codes::already_authorized:
			{
				str = "already authorized";
				break;
			}
			case messages::status_codes::fail:
			{
				str = "fail";
				break;
			}
			default:
			{
				str = "unknown status code";
			}
		}
	}

	/// <summary>
	/// Removal of boilerplate code.
	/// </summary>
	/// <param name="os"></param>
	/// <param name="obj"></param>
	/// <returns></returns>
	std::ostream& response_stream_handler(std::ostream& os, auto&& obj)
	{
		std::string status_code;
		translate_err_code_to_msg(status_code, obj.status);

		if (obj.status != messages::status_codes::success)
		{
			os << "Operation failed. Status code: " + status_code;

			if (obj.message.size())
			{
				os << ". Message from the server: " + obj.message << "\n\n";
			}
			else
			{
				os << "\n\n";
			}
		}
		else
		{
			os << "Operation successfully completed." << "\n\n";
		}

		return os;
	}

	std::ostream& operator<< (std::ostream& os, const messages::response& obj)
	{
		return response_stream_handler(os, obj);
	}

	std::ostream& operator<< (std::ostream& os, messages::response&& obj)
	{
		return response_stream_handler(os, obj);
	}

	uint32_t common::get_stream_size(std::stringstream& ss)
	{
		ss.seekg(0, std::ios::end);
		uint32_t message_size = ss.tellg();
		ss.seekg(0, std::ios::beg);

		return message_size;
	}

	std::string common::get_base64_from_sha512(std::string& input)
	{
		std::string base64;

		base64.resize(common::consts::SHA512_in_base64_size);
		boost::beast::detail::base64::encode(base64.data(), input.data(), 64);

		return base64;
	}

	std::string common::find_source_directory()
	{
		static bool found = false;
		static auto path = std::filesystem::path{ std::filesystem::current_path() };

		if (found)
		{
			return path.string();
		}

		std::filesystem::path prev_path = {};

		while (true)
		{
			if (prev_path == path)
			{
				break;
			}

			for (auto&& dir_entry : std::filesystem::directory_iterator{ path })
			{
				if (dir_entry.is_regular_file())
				{
					std::string full_path = std::filesystem::absolute(dir_entry).string();
					std::string file_name = std::filesystem::path{ full_path }.filename().string();

					if (file_name.compare("user.key") ? false : true)
					{
						found = true;
					}
				}
			}

			if (found)
			{
				break;
			}

			prev_path = path;
			path = path.parent_path();
		}

		if (found)
		{
			return path.string();
		}

		throw std::runtime_error{ "Failed to find source directory" };
	}
}