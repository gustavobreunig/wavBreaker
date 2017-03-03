#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <stdexcept>
#include "wavBreaker.h"

int main(int argc, char** args) {
   if (argc != 8)
   {
     std::cout << "Usage:" << std::endl;
     std::cout << "wavBreaker -i interval_minutes -b bitrate -o filename file.wav" << std::endl;
     std::cout << "example: wavBreaker -i 5 -b 256 -o music music.wav" << std::endl;
     return 0;
   }

   parse_args(argc, args);

   //read wav duration
   std::string ffmpeg_duration_command = "ffmpeg -i ";
   ffmpeg_duration_command += args[argc - 1];
   ffmpeg_duration_command += " 2>&1"; // redirect stderr to stdout, ffmpeg use stderr
   std::string ffmpeg_duration_command_output = exec_get_return(ffmpeg_duration_command.c_str());
   
   int duration = getDurationInSeconds(ffmpeg_duration_command_output);
   int chunkSize = interval_minutes * 60;
   int chunks = duration / chunkSize;
   int lastChunk = duration % chunkSize;
   
   std::cout << "chunks: " << chunks << std::endl;
   
   int acc = 0;
   for (int i = 0; i < chunks; i++)
   {
     if (i == chunks - 1)
     {
       chunkSize += lastChunk;
     }

     std::ostringstream wavOutputss;
     wavOutputss << filename << i << ".wav";
     std::string wavOutput(wavOutputss.str());

     //split
     std::stringstream ffmpegCommand;
     ffmpegCommand << "ffmpeg -i " << "\"" << args[argc - 1] << "\"" << " -ss " << acc << " -t " << chunkSize << " " << wavOutput;
     system(ffmpegCommand.str().c_str());
     acc += chunkSize;

     //encode
     std::stringstream lameCommand;
     lameCommand << "lame -b " << bitrate << " " << wavOutput;
     system(lameCommand.str().c_str());

     //delete wav file
     remove(wavOutput.c_str());
   }

   return 0;
}

std::string exec_get_return(const char* cmd) { //from here: http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
	char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int getDurationInSeconds(std::string ffmpeg_out)
{
	int seconds = 0;
	//process ffmpeg output to find duration
	std::istringstream f(ffmpeg_out);
	std::string line;
	while (std::getline(f, line)) {
		if (line.substr(2, 8) == "Duration")
		{
			//get hours, minutes and seconds from line
			//ex:   Duration: 00:59:14.74, bitrate: 1411 kb/s		
			seconds += atoi(line.substr(12, 2).c_str()) * 60 * 60;
			seconds += atoi(line.substr(15, 2).c_str()) * 60;
			seconds += atoi(line.substr(18, 2).c_str());
		}
    }
	
	return seconds;
}

void parse_args(int argc, char** args)
{
  for (int i = 0; i < argc; i++)
  {
      if (std::string(args[i]) == "-i")
      {
        interval_minutes = atoi(args[i + 1]);
      }

      if (std::string(args[i]) == "-b")
      {
        bitrate = atoi(args[i + 1]);
      }

      if (std::string(args[i]) == "-o")
      {
        filename = std::string(args[i + 1]);
      }
  }
}