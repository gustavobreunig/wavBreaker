#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include "wavBreaker.h"

using namespace std;

int main(int argc, char** args) {
   if (argc != 8)
   {
     cout << "Usage:" << endl;
     cout << "wavBreaker -i interval_minutes -b bitrate -o filename file.wav" << endl;
     cout << "example: wavBreaker -i 5 -b 256 -o music music.wav" << endl;
     return 0;
   }

   parse_args(argc, args);

   fstream wav_file(args[argc - 1], ios::binary|ios::in );

   if (!wav_file.is_open())
   {
	   cout << "Unable to open file" << endl;
	   return 0;
   }
   
   header hdr;
   wav_file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));

   wav_file.close();

   int duration = durationInSeconds(&hdr);
   int chunkSize = interval_minutes * 60;
   int chunks = duration / chunkSize;
   int lastChunk = duration % chunkSize;
   
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

int durationInSeconds(struct header* hdr)
{
    int numSamples = hdr->subchunk2_size /
                         (hdr->num_channels * (hdr->bits_per_sample/8));
    int durationSeconds = numSamples / hdr->sample_rate;
    return durationSeconds;
}

void parse_args(int argc, char** args)
{
  for (int i = 0; i < argc; i++)
  {
      if (string(args[i]) == "-i")
      {
        interval_minutes = atoi(args[i + 1]);
      }

      if (string(args[i]) == "-b")
      {
        bitrate = atoi(args[i + 1]);
      }

      if (string(args[i]) == "-o")
      {
        filename = string(args[i + 1]);
      }
  }
}