#include "fileio.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "engine/internal_libs.hpp"
#include "engine/utility/clock.hpp"
#include "engine/utility/formatted_error.hpp"
#include "engine/engine.hpp"
#include "engine/asset/fonts.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#define MINIMP3_IMPLEMENTATION
#include <minimp3_ex.h>



// ignore external warnings
#ifdef __clang__
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wnullability-extension"
#elif __GNUC__
#endif

// #include <miniz/miniz_zip.h>
#include <zip.h>

// ignore external warnings
#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#endif



constexpr int stb_i_format(oe::formats format) {
	switch (format)
	{
	case oe::formats::rgba:
		return STBI_rgb_alpha;
	case oe::formats::rgb:
		return STBI_rgb;
	case oe::formats::mono:
		return STBI_grey;
	}
	return 0;
}

constexpr int stb_i_channels(oe::formats format) {
	switch (format)
	{
	case oe::formats::rgba:
		return 4;
	case oe::formats::rgb:
		return 3;
	case oe::formats::mono:
		return 1;
	}
	return 0;
}

namespace oe::utils
{
	image_data::image_data(oe::formats _format, int _width, int _height)
		: format(_format)
		, width(_width), height(_height)
		, size(_width* _height* stb_i_channels(_format))
	{
		data = new uint8_t[size];
	}

	image_data::image_data(fs::path path, oe::formats _format)
	{
		int channels;
		data = stbi_load(path.string().c_str(), &width, &height, &channels, stb_i_format(_format));
		
		if (!data)
			throw oe::utils::formatted_error("Failed to load imagefile \"{}\"", path.string().c_str());

		format = _format;
		size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
	}

	image_data::image_data(const uint8_t* _data, size_t data_size, oe::formats _format)
	{
		int channels;
		data = stbi_load_from_memory(_data, data_size, &width, &height, &channels, stb_i_format(_format));
		
		if (!data)
			throw oe::utils::formatted_error("Failed to load imagedata {}:{}", (size_t)_data, data_size);

		format = _format;
		size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels);
	}

	image_data::image_data(const uint8_t* _data, oe::formats _format, int _width, int _height)
		: format(_format)
		, width(_width), height(_height)
		, size(_width* _height* stb_i_channels(_format))
	{
		data = new uint8_t[size];
		std::memcpy(data, _data, size);
	}

	image_data::image_data(const image_data& _copied)
		: format(_copied.format)
		, width(_copied.width), height(_copied.height)
		, size(_copied.width* _copied.height* stb_i_channels(_copied.format))
	{
		data = new uint8_t[size];
		std::memcpy(data, _copied.data, size);
	}

	image_data::image_data(image_data&& move)
		: data(move.data)
		, format(move.format)
		, width(move.width), height(move.height)
		, size(move.size)
	{
		move.format = oe::formats::mono;
		move.width = 0; move.height = 0;
		move.size = 0;
		move.data = 0;
	}

	image_data::~image_data()
	{
		if(data) delete data;
	}
	
	image_data& image_data::operator=(const image_data& copy_assign)
	{
		this->~image_data();
		
		format = copy_assign.format;
		width = copy_assign.width; height = copy_assign.height;
		size = copy_assign.size;
		data = new uint8_t[size];
		std::memcpy(data, copy_assign.data, size);

		return *this;
	}

	image_data& image_data::operator=(image_data&& move_assign)
	{
		this->~image_data();

		format = move_assign.format;
		width = move_assign.width; height = move_assign.height;
		size = move_assign.size;
		data = move_assign.data;

		move_assign.format = oe::formats::mono;
		move_assign.width = 0; move_assign.height = 0;
		move_assign.size = 0;
		move_assign.data = 0;

		return *this;
	}

	byte_string image_data::save() const
	{
		int channels = stb_i_channels(format);
		int size;
		std::unique_ptr<uint8_t[]> data_out(stbi_write_png_to_mem(data, width * channels, width, height, channels, &size));

		return { data_out.get(), data_out.get() + size };
	}

	audio_data::audio_data(int _format, int _size, int _channels, int _sample_rate)
		: format(_format)
		, size(_size)
		, channels(_channels)
		, sample_rate(_sample_rate)
	{
		data = new int16_t[size];
	}

	audio_data::audio_data(fs::path path)
	{
		mp3dec_t mp3d;
		mp3dec_file_info_t info;
		if (mp3dec_load(&mp3d, path.string().c_str(), &info, NULL, NULL))
			throw oe::utils::formatted_error("Failed to load audiofile \"{}\"", std::string(path.string().c_str()));

		size = info.samples * sizeof(int16_t);
		data = info.buffer;
		channels = info.channels;
		sample_rate = info.hz;

		// Format
		format = -1;
		if (info.channels == 1) {
			format = -1/* mono16 */;
		}
		else if (info.channels == 2) {
			format = -1/* stereo16 */;
		}
	}

	audio_data::audio_data(const uint8_t* _data, size_t data_size)
	{
		mp3dec_t mp3d;
		mp3dec_file_info_t info;
		mp3dec_load_buf(&mp3d, _data, data_size, &info, NULL, NULL);
		if (!info.buffer)
			throw oe::utils::formatted_error("Failed to load audiodata {}:{}", (size_t)_data, data_size);
		

		size = info.samples * sizeof(int16_t);
		data = info.buffer;
		channels = info.channels;
		sample_rate = info.hz;

		// Format
		format = -1;
		if (info.channels == 1) {
			format = -1/* mono16 */;
		}
		else if (info.channels == 2) {
			format = -1/* stereo16 */;
		}
	}

	audio_data::audio_data(const int16_t* _data, int _format, int _size, int _channels, int _sample_rate)
		: format(_format)
		, size(_size)
		, channels(_channels)
		, sample_rate(_sample_rate)
	{
		data = new int16_t[size];
		std::memcpy(data, _data, size);
	}

	audio_data::audio_data(const audio_data& _copied)
		: format(_copied.format)
		, size(_copied.size)
		, channels(_copied.channels)
		, sample_rate(_copied.sample_rate)
	{
		data = new int16_t[size];
		std::memcpy(data, _copied.data, size);
	}

	audio_data::audio_data(audio_data&& move)
		: data(move.data)
		, format(move.format)
		, size(move.size)
		, channels(move.channels)
		, sample_rate(move.sample_rate)
	{
		move.format = 0;
		move.size = 0;
		move.channels = 0;
		move.sample_rate = 0;
		move.data = 0;
	}

	audio_data::~audio_data()
	{
		if(data) delete data;
	}
	
	audio_data& audio_data::operator=(const audio_data& copy_assign)
	{
		this->~audio_data();
		
		format = copy_assign.format;
		size = copy_assign.size;
		channels = copy_assign.channels;
		sample_rate = copy_assign.sample_rate;
		data = new int16_t[size];
		std::memcpy(data, copy_assign.data, size);

		return *this;
	}

	audio_data& audio_data::operator=(audio_data&& move_assign)
	{
		this->~audio_data();

		format = move_assign.format;
		size = move_assign.size;
		channels = move_assign.channels;
		sample_rate = move_assign.sample_rate;
		data = move_assign.data;

		move_assign.format = 0;
		move_assign.size = 0;
		move_assign.channels = 0;
		move_assign.sample_rate = 0;
		move_assign.data = 0;

		return *this;
	}
}

namespace oe::utils
{
	struct FontFileMapSingleton
	{
		static FontFileMapSingleton* singleton;
		
		//
		std::unordered_map<size_t, oe::utils::byte_string> font_file_map;
		
		FontFileMapSingleton()
			: font_file_map()
		{
			font_file_map[0] = { 0 };
		};
		static FontFileMapSingleton& get()
		{
			if(!singleton)
				singleton = new FontFileMapSingleton();
			return *singleton;
		}
	};

	FontFileMapSingleton* FontFileMapSingleton::singleton = nullptr;



	FontFile::FontFile(const oe::utils::FileIO& path)
	{
		auto& font_file_map = FontFileMapSingleton::get().font_file_map;

		id = std::hash<std::string>{}(path.getPath().generic_string());
		if(font_file_map.find(id) == font_file_map.end())
			font_file_map.emplace(id, std::move(path.read<oe::utils::byte_string>()));
	}

	FontFile::FontFile()
		: FontFile(oe::asset::Fonts::roboto_regular())
	{}

	const oe::utils::byte_string& FontFile::fontFile() const
	{
		return getFontFile(id);
	}

	const oe::utils::byte_string& FontFile::getFontFile(const size_t id)
	{
		auto& font_file_map = FontFileMapSingleton::get().font_file_map;

		const auto iter = font_file_map.find(id);
		if(iter == font_file_map.end())
			return font_file_map.at(0);

		return iter->second;
	}

	auto zip_open_error(int error)
	{
		zip_error_t ziperror;
		zip_error_init_with_code(&ziperror, error);
		return std::string(zip_error_strerror(&ziperror));
	}

	void write_in_zip(const fs::path& path_to_zip, const fs::path& path_in_zip, const byte_string& data)
	{
		auto generic_to_zip = path_to_zip.generic_string();
		auto generic_in_zip = path_in_zip.generic_string();

		int error;
		auto zipper = zip_open(generic_to_zip.c_str(), ZIP_CREATE, &error);
		if (!zipper)
			throw oe::utils::formatted_error("Failed to open {}, {}", generic_to_zip, zip_open_error(error));

		try
		{
			// create folder structure
			for (fs::path::const_iterator iter = path_in_zip.begin(); ; iter++)
			{
				if (iter == path_in_zip.end() || std::next(iter) == path_in_zip.end())
					break;

				auto filename = iter->generic_string();
				zip_dir_add(zipper, filename.c_str(), ZIP_FL_ENC_UTF_8);
			}

			// add the file
			auto source = zip_source_buffer(zipper, data.data(), data.size(), 0);
			if (!source)
				throw oe::utils::formatted_error("Failed to create source file from data, {}", zip_strerror(zipper));

			auto size = zip_file_add(zipper, generic_in_zip.c_str(), source, ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE);
			if (size < 0)
			{
				zip_source_free(source);
				throw oe::utils::formatted_error("Failed to add file {} to zip, {}", generic_in_zip, zip_strerror(zipper));
			}
		}
		catch (const std::exception& e)
		{
			zip_close(zipper);
			throw e;
		}

		zip_close(zipper);
	}

	void read_from_zip(const fs::path& path_to_zip, const fs::path& path_in_zip, byte_string& data)
	{
		auto generic_to_zip = path_to_zip.generic_string();
		auto generic_in_zip = path_in_zip.generic_string();

		int error;
		auto zipper = zip_open(generic_to_zip.c_str(), 0, &error);
		if (!zipper)
			throw oe::utils::formatted_error("Failed to open {}, {}", path_to_zip, zip_open_error(error));

		try
		{
			zip_stat_t stats;
			zip_stat(zipper, generic_in_zip.c_str(), 0, &stats);

			auto file = zip_fopen(zipper, generic_in_zip.c_str(), 0);
			if (!file)
				throw oe::utils::formatted_error("Failed to open file {} from zip, {}", generic_in_zip.c_str(), zip_strerror(zipper));

			data.resize(stats.size);
			auto read_size = zip_fread(file, data.data(), data.size());
			if (read_size < 0)
			{
				zip_fclose(file);
				throw oe::utils::formatted_error("Failed to read file {} from zip, {}", generic_in_zip.c_str(), zip_strerror(zipper));
			}
			zip_fclose(file);
		}
		catch (const std::exception& e)
		{
			zip_close(zipper);
			throw e;
		}

		zip_close(zipper);
	}

	void zip_paths(const std::vector<fs::path::const_iterator>& iter, const fs::path& current_path, fs::path& path_to_zip, fs::path& path_in_zip)
	{
		for (auto left_iter = current_path.begin(); left_iter != std::next(iter[0]); left_iter++)
			path_to_zip.append(left_iter->generic_string());

		for (auto right_iter = std::next(iter[0]); right_iter != current_path.end(); right_iter++)
			path_in_zip.append(right_iter->generic_string());

		if (!fs::is_regular_file(path_to_zip) && fs::exists(path_to_zip))
			throw oe::utils::formatted_error("Path: '{}' already exists, but is not a file", path_to_zip.generic_string());
	}

	std::vector<fs::path::const_iterator> first_zip_loc(const FileIO& fileio)
	{
		std::vector<fs::path::const_iterator> vec;

		for (fs::path::const_iterator iter = fileio.getPath().begin(); iter != fileio.getPath().end(); iter++)
		{
			if(iter->extension() == ".zip")
				vec.push_back(iter);
		}

		return vec;
	}

	FileIO::FileIO(const fs::path& path)
		: m_current_path(path)
	{}

	FileIO& FileIO::close(size_t n)
	{
		for(size_t i = 0; i < n; i++)
			m_current_path = m_current_path.parent_path();
		return *this;
	}
	
	std::vector<fs::path> FileIO::items() const
	{
		std::vector<fs::path> items;
		
		auto iter = first_zip_loc(*this);
		if (!iter.empty())
		{
			fs::path path_to_zip, path_in_zip;
			zip_paths(iter, m_current_path, path_to_zip, path_in_zip);

			auto generic_to_zip = path_to_zip.generic_string();
			auto generic_in_zip = path_in_zip.generic_string();

			int error;
			auto zipper = zip_open(generic_to_zip.c_str(), 0, &error);
			if (!zipper)
				throw oe::utils::formatted_error("Failed to open {}, {}", path_to_zip, zip_open_error(error));

			auto n = zip_get_num_entries(zipper, 0);
			std::string last_folder;
			for(decltype(n) ni = 0; ni < n; ni++)
			{
				zip_stat_t stats;
				zip_stat_index(zipper, ni, 0, &stats);
				
				fs::path parent_path = fs::path(stats.name).parent_path();
				if(path_in_zip == parent_path)
					items.push_back(path_to_zip / fs::path(stats.name));

				if(last_folder != parent_path && parent_path.parent_path() == path_in_zip)
				{
					last_folder = parent_path.generic_string();
					items.push_back(path_to_zip / last_folder);
				}
			}
		}
		else
		{
			for(auto& iter : fs::directory_iterator(m_current_path))
				items.push_back(iter);
		}
		return items;
	}
	
	const fs::path& FileIO::getPath() const
	{
		return m_current_path;
	}

	bool FileIO::isDirectory() const
	{
		return fs::is_directory(m_current_path);
	}

	bool FileIO::isFile() const
	{
		return fs::is_regular_file(m_current_path);
	}
	
	bool FileIO::exists() const
	{
		return fs::exists(m_current_path);
	}

	void FileIO::remove() const
	{
		fs::remove_all(m_current_path);
	}

	byte_string FileIOInternal<byte_string>::read(const FileIO& path)
	{
		auto iter = first_zip_loc(path);
		if (!iter.empty())
		{
			fs::path path_to_zip, path_in_zip;
			zip_paths(iter, path.getPath(), path_to_zip, path_in_zip);

			byte_string data;
			read_from_zip(path_to_zip, path_in_zip, data);
			return data;
		}
		else
		{
			std::ifstream input_stream(path.getPath(), std::ios_base::binary);
			if (!input_stream.is_open())
				throw oe::utils::formatted_error("Could not open file: '{}'", path.getPath().generic_string());

			return { std::istreambuf_iterator<char>(input_stream), {} };
		}
	}

	void FileIOInternal<byte_string>::write(const FileIO& path, const byte_string& string)
	{
		auto iter = first_zip_loc(path);
		if (!iter.empty())
		{
			fs::path path_to_zip, path_in_zip;
			zip_paths(iter, path.getPath(), path_to_zip, path_in_zip);

			write_in_zip(path_to_zip, path_in_zip, string);
		}
		else
		{
			if (!path.isFile() && path.exists())
				throw oe::utils::formatted_error("Path: '{}' already exists, but is not a file", path.getPath().generic_string());

			if (!path.exists())
				fs::create_directories(path.getPath().parent_path());

			std::ofstream output_stream(path.getPath());
			if (!output_stream.is_open())
				throw oe::utils::formatted_error("Could not open file: '{}'", path.getPath().generic_string());
			
    		std::copy(string.begin(), string.end(), std::ostreambuf_iterator<char>(output_stream));
		}
	}
}