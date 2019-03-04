#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum kernel_type { mean, gaussian, sharpen };
enum convolve_edge_type { zero, extend, mirror, crop, kernel_crop };


class Kernel {
public:
	int width, height;
	std::vector<std::vector<int> > kernel_matrix;
	Kernel() : width(1), height(1), kernel_matrix({ 1 }) {};
	Kernel(int width, int height, std::vector<std::vector<int> > n_kernel) {
		width = width;
		height = height;
		kernel_matrix = n_kernel;
	}
	Kernel(kernel_type type, int n_width, int n_height) {
		width = n_width;
		height = n_height;
		for (int i = 0; i < height; i++) {
			std::vector<int> temp;
			for (int j = 0; j < width; j++) {
				temp.push_back(0);
			}
			kernel_matrix.push_back(temp);
		}
		switch (type) {
		case mean:
			kernel_matrix = std::vector<std::vector<int>>(height, std::vector<int>(width, 1));
			break;
		case sharpen:
			kernel_matrix = std::vector<std::vector<int>>(3, std::vector<int>(3, 0));
			kernel_matrix[1][1] = 5;
			kernel_matrix[0][1] = -1;
			kernel_matrix[1][0] = -1;
			kernel_matrix[1][2] = -1;
			kernel_matrix[2][1] = -1;
		}
	}
};

class Image {
public:
	std::string filename;
	int width, height, channels;
	unsigned char* data;
	Image() : filename(""), width(0), height(0), channels(0), data(nullptr){};
	Image(std::string n_filename, int n_width, int n_height, int n_channels, unsigned char* n_data) {
		filename = n_filename;
		width = n_width;
		height = n_height;
		channels = n_channels;
		data = n_data;
		whiteout();
	}
	Image(std::string n_filename, int n_width, int n_height, int n_channels) {
		filename = n_filename;
		width = n_width;
		height = n_height;
		channels = n_channels;
		data = new unsigned char[width * height * channels];
		whiteout();
	}
	Image copy_from(Image im) {
		Image temp("n_" + filename, width, height, channels);
		for (int i = 0; i < width * height * channels; i++) {
			temp.data[i] = im.data[i];
		}
		return temp;
	}
	void whiteout() {
		for (int i = 0; i < width * height * channels; i++) {
			data[i] = 255;
		}
	}
	int load_image(std::string n_filename) {
		filename = n_filename;
		data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
		if (stbi_failure_reason() != NULL)
			std::cout << stbi_failure_reason() << std::endl;
		return 1;
	}
	int write_image(std::string filename) {
		int output = stbi_write_jpg(filename.c_str(), width, height, channels, data, 100);
		if (output == 0)
			std::cout << "operation failed" << std::endl;
		else
			std::cout << "operation success" << std::endl;
		return output;
	}
	int* get_pixel(int x, int y) {
		int pixel_index = channels * (y * width + x);
		int* pixel = new int[channels];
		for (int i = 0; i < channels; i++) {
			pixel[i] = data[pixel_index + i];
		}
		return pixel;
	}
	void set_pixel(int x, int y, int pixel[]) {
		int pixel_index = channels * (y * width + x);
		for (int i = 0; i < channels; i++) {
			data[pixel_index + i] = pixel[i];
		}
	}
	int get_value(int x, int y, int c) {
		int value_index = channels * (y * width + x) + c;
		return data[value_index];
	}
	void set_value(int x, int y, int c, int value) {
		int value_index = channels * (y * width + x) + c;
		data[value_index] = value;
	}
	Image greyscale() {
		Image temp = copy_from(*this);
		for (int i = 0; i < width * height * channels; i++) {
			temp.data[i] = data[i];
		}
		for (int i = 0; i < width * height * channels; i += channels) {
			int temp_sum = 0;
			for (int j = 0; j < channels; j++) {
				temp_sum += data[i + j];
			}
			int average = nearbyint(temp_sum / channels);
			for (int j = 0; j < channels; j++) {
				temp.data[i + j] = average;
			}
		}
		return temp;
	}
	Image convolve(Kernel kernel, convolve_edge_type edge_type) {
		Image temp = copy_from(*this);
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				for (int c = 0; c < channels; c++) {
					int new_value = 0;
					for (int jk = 0; jk < kernel.height; jk++) {
						for (int ik = 0; ik < kernel.width; ik++) {
							int j_offset = (int)ceil(kernel.height / 2);
							int i_offset = (int)ceil(kernel.width / 2);
							if (i - i_offset + ik < 0 || i - i_offset + ik >= width)
								continue;
							if (j - j_offset + jk < 0 || j - j_offset + jk >= height)
								continue;
							if (kernel.kernel_matrix[jk][ik] == 0)
								continue;
							int old_value = get_value(i - i_offset + ik, j - j_offset + jk, c);
							new_value += old_value * kernel.kernel_matrix[jk][ik];
						}
					}
					temp.set_value(i, j, c, std::max(0, std::min(new_value, 255)));
				}
			}
			std::cout << "finished row " << j << "..." << std::endl;
		}
		return temp;
	}
};

int main() {
	Image im;
	im.load_image("plant.jpg");

	//processing here
	//im = im.convolve(Kernel(mean, 10, 10), zero);
	//im = im.convolve(Kernel(mean, 1, 9), zero);
	//im = im.convolve(Kernel(mean, 9, 1), zero);
	//im = im.greyscale();
	im = im.convolve(Kernel(sharpen, 3, 3), zero);

	im.write_image("plant1.jpg");

	std::cin.get();
	//stbi_image_free(data);
	return 0;
}