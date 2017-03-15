#ifndef WAVBREAKER_H
#define WAVBREAKER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <stdexcept>
#include <windows.h>

bool parse_args(int argc, char** args);
int main(int argc, char** args);
int getDurationInSeconds(std::string ffmpeg_out);
std::string exec_get_return(const char* cmd);
void printUsage();

int interval_minutes = 0;
int bitrate = 0;
int hour_creation = 0;
std::string input_filename;
std::string output_filename;

#endif //WAVBREAKER_H