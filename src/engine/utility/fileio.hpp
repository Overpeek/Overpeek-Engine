#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "engine/enum.hpp"
#include "engine/engine.hpp"



namespace oe::utils
{
	using byte_string = std::vector<uint8_t>;

	struct image_data
	{
		uint8_t* data;
		oe::formats format;
		int width, height;
		size_t size;

		image_data(oe::formats format, int width, int height); // allocates space for uint8_t*data
		image_data(fs::path path, oe::formats format = oe::formats::rgba); // load from file
		image_data(const uint8_t* data, size_t data_size, oe::formats format = oe::formats::rgba); // load from memory
		image_data(const uint8_t* data, oe::formats format, int width, int height);
		image_data(const image_data& copied);
		image_data(image_data&& move);
		~image_data();

		byte_string save() const;
	};

	struct audio_data
	{
		int16_t* data;
		int format, size, channels, sample_rate;
		const bool stolen = false;

		audio_data(int format, int size, int channels, int sample_rate); // allocates space for uint16_t*data
		audio_data(fs::path path); // load from file
		audio_data(const uint8_t* data, size_t data_size); // load from memory
		audio_data(const int16_t* data, int format, int size, int channels, int sample_rate);
		audio_data(const audio_data& copied);
		audio_data(audio_data&& move);
		~audio_data();
	};

	// workaround for msvc bug
	class FileIO;
	template<typename T>
	struct FileIOInternal
	{
		static T read(const FileIO&);
		static void write(const FileIO&, const T&);
	};
	template<>
	struct FileIOInternal<byte_string>
	{
		static byte_string read(const FileIO& path);
		static void write(const FileIO& path, const byte_string& data);
	};
	template<>
	struct FileIOInternal<image_data>
	{
		static image_data read(const FileIO& path, const oe::formats& format = oe::formats::rgba)
		{
			auto data = FileIOInternal<byte_string>::read(path);
			return { data.data(), data.size(), format };
		};

		static void write(const FileIO& path, const image_data& data)
		{
			return FileIOInternal<byte_string>::write(path, data.save());
		}
	};
	template<>
	struct FileIOInternal<audio_data>
	{
		static audio_data read(const FileIO& path)
		{
			auto data = FileIOInternal<byte_string>::read(path);
			return { data.data(), data.size() };
		};

		static void write(const FileIO& path, const audio_data& data)
		{
			return oe_error_terminate("No write fn for audio_data");
		}
	};
	template<>
	struct FileIOInternal<std::string>
	{
		static std::string read(const FileIO& path)
		{
			auto data = FileIOInternal<byte_string>::read(path);
			return { data.data(), data.data() + data.size() };
		};

		static void write(const FileIO& path, const std::string& data)
		{
			return FileIOInternal<byte_string>::write(path, { data.begin(), data.end() });
		}
	};

	// std fstream and libzip wrapper
	// currently supports zip nesting depth just up to max 1
	class FileIO
	{
	private:
		fs::path m_current_path;

	public:
		FileIO(const fs::path& path = std::filesystem::current_path());
		FileIO(const std::string& path) : FileIO(fs::path(path)) {}
		FileIO(const std::string_view& path) : FileIO(fs::path(path)) {}
		FileIO(const char* cstr) : FileIO(fs::path(cstr)) {}

		operator fs::path() const
		{
			return m_current_path;
		}
		
		template<typename T>
		FileIO operator/(const T& right) const // open directory or directories
		{
			FileIO copy = *this;
			copy.open(right);
			return copy;
		}
		
		template<typename T>
		FileIO operator+(const T& right) const // open directory or directories
		{
			FileIO copy = *this;
			copy.open(right);
			return copy;
		}

		template<typename T>
		FileIO& open(const T& file_or_directory_name) // open directory or directories
		{
			m_current_path /= file_or_directory_name;
			return *this;
		}
		FileIO& close(size_t n = 1); // go back or close directories

		std::vector<fs::path> items() const;
		
		const fs::path& getPath() const;
		bool isDirectory() const;
		bool isFile() const;
		bool exists() const;

		void remove() const;

		// read/write any
		template<typename T, typename ... Args> inline T read(const Args&& ... args) const { return FileIOInternal<T>::read(m_current_path, args...); }
		template<typename T, typename ... Args> inline void write(const T& d, const Args&& ... args) const { return FileIOInternal<T>::write(m_current_path, d, args...); }
	};
	
	class FontFile
	{
	private:
		size_t id;

	public:
		template<typename T, typename std::enable_if_t<!std::is_same_v<T, oe::utils::FileIO>>>
		FontFile(T&& path) : FontFile(oe::utils::FileIO(std::forward<T>(path))) {}
		FontFile(const oe::utils::FileIO& path);
		FontFile() : id(0) {}

		[[nodiscard]] constexpr inline size_t getID() const noexcept { return id; }
		[[nodiscard]] const oe::utils::byte_string& fontFile() const;
		[[nodiscard]] const static oe::utils::byte_string& getFontFile(const size_t id);
		[[nodiscard]] constexpr inline bool operator==(const FontFile &other) const noexcept { return id == other.id; }
	};
}

namespace std
{
    template<>
	struct hash<oe::utils::FontFile>
    {
        [[nodiscard]] constexpr inline std::size_t operator()(const oe::utils::FontFile& f) const noexcept { return f.getID(); }
    };
}