#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <gsl/span>

#include "engine/enum.hpp"
#include "engine/engine.hpp"



namespace oe::utils
{
	using byte_string = std::vector<uint8_t>;
	using byte_string_view = gsl::span<uint8_t>;

	/* byte_string compress(byte_string_view bytes);
	byte_string uncompress(byte_string_view bytes); */

	constexpr int stb_i_channels(oe::formats format) {
		switch (format)
		{
		case oe::formats::rgba:
			return 4;
		case oe::formats::rgb:
			return 3;
		case oe::formats::mono:
			return 1;
		case oe::formats::none:
			return 0;
		}
		return 0;
	}

	static constexpr inline uint8_t u8max = std::numeric_limits<uint8_t>::max();
	inline float to_float(uint8_t byte)
	{ return static_cast<float>(byte) / static_cast<float>(u8max); }

	inline glm::vec<4, uint8_t> to_rgba(const glm::vec<4, uint8_t>& rgba)
	{ return rgba; }
	inline glm::vec<4, uint8_t> to_rgba(const glm::vec<3, uint8_t>& rgb)
	{ return { rgb, u8max }; }
	inline glm::vec<4, uint8_t> to_rgba(const glm::vec<1, uint8_t>& mono)
	{ return { mono, mono, mono, u8max }; }
	
	inline glm::vec<3, uint8_t> to_rgb(const glm::vec<4, uint8_t>& rgba)
	{ return { rgba.r * to_float(rgba.a), rgba.g * to_float(rgba.a), rgba.b * to_float(rgba.a) }; }
	inline glm::vec<3, uint8_t> to_rgb(const glm::vec<3, uint8_t>& rgb)
	{ return rgb; }
	inline glm::vec<3, uint8_t> to_rgb(const glm::vec<1, uint8_t>& mono)
	{ return { mono, mono, mono }; }
	
	inline glm::vec<1, uint8_t> to_mono(const glm::vec<4, uint8_t>& rgba)
	{ return glm::vec<1, uint8_t>{ static_cast<uint8_t>(glm::length(static_cast<glm::vec<4, float>>(rgba)) * to_float(rgba.a)) }; }
	inline glm::vec<1, uint8_t> to_mono(const glm::vec<3, uint8_t>& rgb)
	{ return glm::vec<1, uint8_t>{ static_cast<uint8_t>(glm::length(static_cast<glm::vec<3, float>>(rgb))) }; }
	inline glm::vec<1, uint8_t> to_mono(const glm::vec<1, uint8_t>& mono)
	{ return mono; }


	struct image_data_base;
	template<oe::formats _format, int _width, int _height>
	struct image_datac;
	struct image_data;

	struct image_data_base
	{
		uint8_t* data = nullptr;
		oe::formats format = oe::formats::rgba;
		int width = 0, height = 0;
		size_t size = 0;

		constexpr image_data_base() = default;
		constexpr image_data_base(uint8_t* _data, oe::formats _format, int _width, int _height)
			: data(_data), format(_format), width(_width), height(_height), size(width * height * stb_i_channels(format))
		{}
		image_data cast(oe::formats format = oe::formats::none, int width = -1, int height = -1) const;
		image_data crop(int x = 0, int y = 0, int width = -1, int height = -1) const;
		byte_string save() const;
	};

	struct image_data : image_data_base
	{
		image_data() = default;
		image_data(oe::formats format, int width, int height); // allocates space for uint8_t*data
		image_data(fs::path path, oe::formats format = oe::formats::rgba); // load from file
		image_data(const uint8_t* data, size_t data_size, oe::formats format = oe::formats::rgba); // load from memory
		image_data(const uint8_t* data, oe::formats format, int width, int height);
		image_data(const image_data_base& copied);
		image_data(const image_data& copied) : image_data(static_cast<const image_data_base&>(copied)) {}
		image_data(image_data&& move);
		~image_data();

		image_data& operator=(const image_data_base& copy_assign);
		image_data& operator=(image_data&& move_assign);
	};

	template<oe::formats _format, int _width, int _height>
	struct image_datac : image_data_base
	{
		static_assert(_width >= 0 && _height >= 0, "Dimensions must be >= 0");
		std::array<uint8_t, static_cast<size_t>(_width) * static_cast<size_t>(_height) * static_cast<size_t>(stb_i_channels(_format))> stack_data{};

		inline constexpr image_datac() noexcept
			: image_data_base(nullptr, _format, _width, _height)
		{
			data = stack_data.data();
		}
		inline constexpr image_datac(const uint8_t* data)
			: image_datac()
		{
			for (size_t i = 0; i < stack_data.size(); i++)
				stack_data[i] = data[i];
		}
		inline image_datac(const image_data_base& copy)
			: image_datac()
		{
			if (copy.width != _width || copy.height != _height || copy.format != _format)
				throw std::runtime_error("Widths, heights or formats do not match");
			std::copy(copy.data, copy.data + stack_data.size(), stack_data.front());
		}
		inline constexpr image_datac(const image_datac& copy)
			: image_datac()
		{
			for (size_t i = 0; i < copy.stack_data.size(); i++)
				stack_data[i] = copy.stack_data[i];
		}
	};

	struct audio_data
	{
		int16_t* data = nullptr;
		int format = 0, size = 0, channels = 0, sample_rate = 0;
		
		constexpr audio_data() = default;
		audio_data(int format, int size, int channels, int sample_rate); // allocates space for uint16_t*data
		audio_data(fs::path path); // load from file
		audio_data(const uint8_t* data, size_t data_size); // load from memory
		audio_data(const int16_t* data, int format, int size, int channels, int sample_rate);
		audio_data(const audio_data& copied);
		audio_data(audio_data&& move);
		virtual ~audio_data();

		audio_data& operator=(const audio_data& copy_assign);
		audio_data& operator=(audio_data&& move_assign);
	};



	class FileIO;
	// workaround for msvc bug
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

		static void write(const FileIO&, const audio_data&)
		{
			spdlog::warn("No write fn for audio_data");
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
}

namespace std
{
    template<>
	struct hash<oe::utils::FontFile>
    {
        [[nodiscard]] constexpr inline std::size_t operator()(const oe::utils::FontFile& f) const noexcept { return f.getID(); }
    };
}