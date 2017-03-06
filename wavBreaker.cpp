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

   //call ffmpeg with original file and store output in ffmpeg_duration_command_output
   std::string ffmpeg_duration_command = "ffmpeg -i ";
   ffmpeg_duration_command += input_filename;
   ffmpeg_duration_command += " 2>&1"; // redirect stderr to stdout, ffmpeg use stderr when there is no output file
   std::string ffmpeg_duration_command_output = exec_get_return(ffmpeg_duration_command.c_str());
   
   //calculate duration, chunk size, number of chunks, etc..
   int duration = getDurationInSeconds(ffmpeg_duration_command_output);
   int chunkSize = interval_minutes * 60;
   int chunks = duration / chunkSize;
   int lastChunk = duration % chunkSize;
   
   int start_offset = 0;
   for (int i = 0; i < chunks; i++)
   {
     if (i == chunks - 1)
     {
       chunkSize += lastChunk; //the last chunk will be bigger
     }

	 //generate filename with number
     std::ostringstream wavOutputss;
	 if (i < 10)
	 wavOutputss << 0;
     wavOutputss << i << ". " << output_filename;
     std::string wavOutput(wavOutputss.str());

     //split
     std::stringstream ffmpegCommand;
     ffmpegCommand << "ffmpeg -i " << "\"" << input_filename << "\"" << " -ss " << start_offset << " -t " << chunkSize << " " << "\"" << wavOutput << ".wav\"";
	 start_offset += chunkSize;
     system(ffmpegCommand.str().c_str());

     //encode
     std::stringstream lameCommand;
     lameCommand << "lame -b " << bitrate << " -q0 --tn " << i + 1 << " --ta " << "\"" << wavOutput << "\"" << " " << "\"" << wavOutput << ".wav\"" << " " << "\"" << wavOutput << ".mp3\"";
     system(lameCommand.str().c_str());

     //delete wav file
	 std::stringstream delCommand;
	 delCommand << "del " << "\"" << wavOutput << ".wav\"";
     system(delCommand.str().c_str());
	 
	 //change mp3 file attributes, creation time, last access time and last write
	 //to increment one minute each file
	 //this is needed because some mp3 readers order by these attributes
	 std::string mp3_filename(wavOutput);
	 mp3_filename += ".mp3";
	 HANDLE h_mp3_file = CreateFile(mp3_filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	 FILETIME creation;
	 FILETIME lastAccess;
	 FILETIME lastWrite;
	 if (!GetFileTime(h_mp3_file, &creation, &lastAccess, &lastWrite))
	 {
		 std::cout << "cannot retrieve file attributes: error " << GetLastError() << std::endl;
		 return -1;
	 }
	 
	 //this will create files in the past and future
	 creation.dwHighDateTime -= chunks - i;
	 lastAccess.dwHighDateTime -= chunks - i;
	 lastWrite.dwHighDateTime -= chunks - i;
	 
	 if (!SetFileTime(h_mp3_file, &creation, &lastAccess, &lastWrite))
	 {
		 std::cout << "cannot change file attributes: error " << GetLastError() << std::endl;
		 return -1;
	 }
	 
	 CloseHandle(h_mp3_file);
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
	//process ffmpeg output to find duration of file
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
	input_filename = std::string(args[argc - 1]);
	
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
        output_filename = std::string(args[i + 1]);
      }
  }
}