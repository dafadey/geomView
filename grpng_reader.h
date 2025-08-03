#pragma once
#include "png.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>

struct color
{
  color() : red(0.), green(0.), blue(0.) {}
  color(double r, double g, double b) : red(r), green(g), blue(b) {}
  double red;
  double green;
  double blue;
  double distance(color c) {
    return std::sqrt(std::pow(this->red - c.red,2)
              + std::pow(this->green - c.green,2)
              + std::pow(this->blue - c.blue,2));
  }
};

static bool operator<(const color& c1, const color& c2) {
  if(c1.red!=c2.red)
    return c1.red < c2.red;

  if(c1.green!=c2.green)
    return c1.green < c2.green;
    
  if(c1.blue!=c2.blue)
    return c1.blue < c2.blue;
  
  return false;
}

static std::ostream& operator<<(std::ostream& o, const color& c) {
  return o << '(' << c.red << ", " << c.green << ", " << c.blue << ')';
}

struct image
{
  int w;
  int h;
  color* data;
  //png specific
  png_byte color_type;
  png_byte bit_depth;
  
  void clone_png_specific(const image* other) {
    color_type = other->color_type;
    bit_depth = other->bit_depth;
  }
  
  color get(int x, int y)
  {
    return data[x*h+y];
  }
  void set(color c, int x, int y)
  {
     data[x*h+y]=c;
  }
  image() = delete;
  image(int _w, int _h): w(_w), h(_h)
  {
    data = new color[w*h];
  }
  ~image()
  {
    delete[] data;
  }
};

struct my_stream {
  my_stream(const char* data, int size) : buff(data), eof(data + size) {}
  const char* buff;
  const char* eof;
};

static void my_read_data_fn(png_struct_def* png_ptr, unsigned char* data, long unsigned int bytes_count ) {
  png_voidp io_ptr = png_get_io_ptr(png_ptr);
  auto strm = (my_stream*) io_ptr;

  for(int i=0; i<bytes_count; i++)
  {
    data[i] = *(strm->buff);
    if(strm->buff < strm->eof)
      strm->buff++;
  }
}

static image* read_png_file(const char* buff, int size)
{
  if(size < 8)
    return nullptr;
  my_stream file(buff, size);
  int x, y;

  int width, height, channels;
  png_byte color_type;
  png_byte bit_depth;
  png_structp png_ptr{};
  png_infop info_ptr{};
  int number_of_passes;
  png_bytep * row_pointers;

  if (png_sig_cmp((png_const_bytep) file.buff, 0, 8))
    std::cerr << "[read_png_file] input is not recognized as a PNG file" << std::endl;
  file.buff += 8;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    std::cerr << "[read_png_file] png_create_read_struct failed" << std::endl;
    
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    std::cerr << "[read_png_file] png_create_info_struct failed" << std::endl;
    
  if (setjmp(png_jmpbuf(png_ptr)))
    std::cerr << "[read_png_file] Error during init_io" << std::endl;
  
  png_set_read_fn(png_ptr, (png_voidp) &file, (png_rw_ptr) &my_read_data_fn);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  number_of_passes = png_set_interlace_handling(png_ptr);
  channels = png_get_channels(png_ptr, info_ptr);
  png_read_update_info(png_ptr, info_ptr);

  int num_palette{};
  png_colorp palette{};
  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
  
  //std::cout << "read file" << std::endl;
  if (setjmp(png_jmpbuf(png_ptr)))
          std::cerr << "[read_png_file] Error during read_image" << std::endl;

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (y=0; y<height; y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, row_pointers);

  image* img=new image(width,height);
  img->color_type = color_type;
  img->bit_depth = bit_depth;
  
  for(int j=0;j<height;j++)
  {
    for(int i=0;i<width;i++)
    {
      png_color png_c;
      if(palette) {
        int cidx = row_pointers[j][i*channels];
        png_c = palette[cidx];
      } else {
        png_c.red = row_pointers[j][i*channels+0];
        png_c.green = row_pointers[j][i*channels+1];
        png_c.blue = row_pointers[j][i*channels+2];
      }
      color c;
      c.red = double(png_c.red)/255.0;
      c.green = double(png_c.green)/255.0;
      c.blue = double(png_c.blue)/255.0;
      img->set(c,i,j);
    }
  }
  
  for (y=0; y<height; y++)
    free(row_pointers[y]);
  free(row_pointers);
  
  return img;
}


static void my_write_data_fn(png_struct_def* png_ptr, unsigned char* data, long unsigned int bytes_count ) {
  png_voidp io_ptr = png_get_io_ptr(png_ptr);
  auto strm = (std::vector<unsigned char>*) io_ptr;
  for(int i=0; i<bytes_count; i++)
    strm->push_back(data[i]);
}

static void my_flush_data_fn(png_structp png_ptr) {}

static std::vector<unsigned char> write_png_file(image* img)
{
  std::vector<unsigned char> output_buff;
	png_structp png_ptr{};
	png_infop info_ptr{};
	
	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		std::cerr << "ERROR: png cannot allocate write struct\n";
    return output_buff;
  }
  
	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		std::cerr << "ERROR: png cannot allocate info struct\n";
    return output_buff;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		std::cerr << "ERROR: png cannot set error exception handling\n";
    return output_buff;
	}

  png_set_write_fn(png_ptr, (png_voidp) &output_buff, (png_rw_ptr) my_write_data_fn, (png_flush_ptr) my_flush_data_fn);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, img->w, img->h,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
  std::vector<png_byte> row(3 * img->w * sizeof(png_byte));

	// Write image data
	for (int j = 0; j < img->h; j++) {
		for (int i = 0; i < img->w; i++) {
      row[i*3] = img->get(i,j).red;
      row[i*3+1] = img->get(i,j).green;
      row[i*3+2] = img->get(i,j).blue;
    }
		png_write_row(png_ptr, (png_bytep) row.data());
	}

	// End write
	png_write_end(png_ptr, NULL);

	if (info_ptr)
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr)
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  
  return output_buff;
}
