#pragma once

#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <filesystem>
#else
#include <experimental/filesystem>
#endif


#ifndef __has_include
static_assert(false, "__has_include not supported");
#else
#  if __has_include(<filesystem>)
#    include <filesystem>
namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#  elif __has_include(<boost/filesystem.hpp>)
#    include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#  endif
#endif



namespace oe::utils {

	struct image_data {
		unsigned char* data;
		int width, height, channels;

		image_data(unsigned char* _data, int _width, int _height, int _channels) {
			data = _data;
			width = _width;
			height = _height;
			channels = _channels;
		}

		image_data() : image_data(nullptr, 0, 0, 0) {}
	};

	struct audio_data {
		short* data;
		int format, size, channels, sample_rate;

		audio_data(short* _data, int _size, int _channels, int _sample_rate, int _format) {
			data = _data;
			size = _size;
			channels = _channels;
			sample_rate = _sample_rate;
			format = _format;
		}

		audio_data() : audio_data(nullptr, 0, 0, 0, 0) {}
	};

	// Read contents of file to string
	std::string readFile(fs::path path);

	// Should save image to file
	// Not tested
	void saveImage(fs::path path, const image_data& image);

	// Load image_data from file
	// Remember to freeImage() after use
	const image_data loadImage(fs::path path);

	// Load audio_data from file
	// Remember to freeAudio() after use
	const audio_data loadAudio(std::filesystem::path path);

	// Free image data
	void freeImage(const image_data& image);

	// Free audio data
	void freeAudio(const audio_data& audio);

}