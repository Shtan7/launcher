#pragma once
#include <stdint.h>
#include <string>
#include <coroutine>
#include <type_traits>
#include <utility>
#include <exception>
#include <iterator>
#include <boost/asio.hpp>
#include <variant>
#include <regex>
#include <string>
#include <filesystem>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

namespace asio = boost::asio;

namespace launcher
{
  namespace messages
  {
    enum class request_ids
    {
      authorization,
      registration,
      ping,
      check_general_hash,
      get_update,
    };

    enum class status_codes
    {
      success,
      incorrect_input,
      hash_miss,
      not_authorized,
      already_authorized,
      fail,
    };

    struct response
    {
      status_codes status;
      std::string message;

      /// <summary>
      /// Function required by boost for serializing of struct
      /// fields.
      /// </summary>
      /// <param name="ar">Boost archive.</param>
      /// <param name="version"></param>
      void serialize(auto& ar, const unsigned int version)
      {
        ar& status;
        ar& message;
      }
    };

    struct request
    {
      messages::request_ids request_id;
      std::vector<std::string> request_content;

      /// <summary>
      /// Extract user's data from request struct.
      /// </summary>
      /// <returns>Pair with login and password.</returns>
      std::pair<std::string, std::string> get_login_pass();

      /// <summary>
      /// Extract hash of all files from request struct.
      /// </summary>
      /// <returns>Hash in base64 encoding.</returns>
      std::string& get_general_hash();

      /// <summary>
      /// Extract file hash in pairs where first - file name, second
      /// - file hash in base64 encoding.
      /// </summary>
      /// <returns>Vector of pairs with file hash data.</returns>
      std::vector<std::pair<std::string, std::string>> get_files_hash();

      /// <summary>
      /// Construct request with given or default parameters.
      /// </summary>
      /// <param name="request_id_">Enum variable with request number.</param>
      /// <param name="request_content_">Vector with additional request content.</param>
      request(messages::request_ids request_id_ = {}, std::vector<std::string> request_content_ = {});

      /// <summary>
      /// Request move constructor.
      /// </summary>
      /// <param name="obj"></param>
      request(request&& obj);

      /// <summary>
      /// Function required by boost for serializing of struct
      /// fields.
      /// </summary>
      /// <param name="ar">Boost archive.</param>
      /// <param name="version"></param>
      void serialize(auto& ar, const unsigned int version)
      {
        ar& request_id;
        ar& request_content;
      }
    };

    struct file_list
    {
      // first is file name, second is file size
      std::vector<std::pair<std::string, uint32_t>> files;

      /// <summary>
      /// Function required by boost for serializing of struct
      /// fields.
      /// </summary>
      /// <param name="ar">Boost archive.</param>
      /// <param name="version"></param>
      void serialize(auto& ar, const unsigned int version)
      {
        ar& files;
      }
    };
  }

  /// <summary>
  /// Adapter for the enum class variable.
  /// </summary>
  /// <param name="os"></param>
  /// <param name="obj"></param>
  /// <returns></returns>
  std::istream& operator>> (std::istream& os, messages::request_ids& obj);

  /// <summary>
  /// Adapter for the enum class variable.
  /// </summary>
  /// <param name="os"></param>
  /// <param name="obj"></param>
  /// <returns></returns>
  std::ostream& operator<< (std::ostream& os, const messages::status_codes& obj);

  /// <summary>
  /// Provide information about server response. This version takes
  /// reponse object with lvalue reference.
  /// </summary>
  std::ostream& operator<< (std::ostream& os, const messages::response& obj);

  /// <summary>
  /// Provide information about server response. This version takes
  /// reponse object with rvalue reference.
  /// </summary>
  std::ostream& operator<< (std::ostream& os, messages::response&& obj);

  namespace common
  {

    /// <summary>
    /// Get content size of stringstream in bytes.
    /// </summary>
    /// <param name="ss">Reference to stream.</param>
    /// <returns>Number of bytes.</returns>
    uint32_t get_stream_size(std::stringstream& ss);

    namespace regex
    {
      inline std::regex log_pass_regex{ R"([a-zA-Z0-9_\-\/*()\\]+)" };
    }

    namespace consts
    {
      inline constinit uint32_t MiB = 0x100000;
      inline constinit uint32_t SHA512_in_base64_size = 88;
    }

    /// <summary>
    /// Get hash (sha512) of string.
    /// </summary>
    /// <param name="input">Input string.</param>
    /// <returns>String with raw hash.</returns>
    std::string hash_string(std::string& input); // string hash with salt  

    /// <summary>
    /// Convert raw sha512 hash to base64 encoding.
    /// </summary>
    /// <param name="input">Raw hash.</param>
    /// <returns>Base64 string.</returns>
    std::string get_base64_from_sha512(std::string& input);

    /// <summary>
    /// Get the source directory containing 
    /// the files required for the application.
    /// </summary>
    std::string find_source_directory();
  }
}