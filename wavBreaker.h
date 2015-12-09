#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>

struct header {
	char chunk_id[4];
	int chunk_size;
	char format[4];
	char subchunk1_id[4];
	int subchunk1_size;
	short int audio_format;
	short int num_channels;
	int sample_rate;
	int byte_rate;
	short int block_align;
	short int bits_per_sample;
	char subchunk2_id[4];
	int subchunk2_size;
};

int durationInSeconds(struct header* hdr);
void parse_args(int argc, char** args);
int main(int argc, char** args);

int interval_minutes = 0;
int bitrate = 0;
std::string filename;