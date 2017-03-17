#include "wavBreaker.h"

void printUsage()
{
     std::cout << "Usage:" << std::endl;
     std::cout << "wavBreaker -i interval_minutes -b bitrate [-h hour_creation] -o filename file.wav" << std::endl;
     std::cout << "example: wavBreaker -i 5 -b 256 -h 13 -o music music.wav" << std::endl;	
}

void createWavTempFile(std::string original)
{
	//convert whatever the file is, to default WAV
	std::string ffmpeg_to_wav = "ffmpeg -i ";
	ffmpeg_to_wav += "\"";
	ffmpeg_to_wav += original;
	ffmpeg_to_wav += "\"";
	ffmpeg_to_wav += " ";
	ffmpeg_to_wav += TEMP_WAV_FILE;
	system(ffmpeg_to_wav.c_str());
}

int getWavDuration(std::string wav)
{
	//parse and return wav duration in seconds
	//call ffmpeg with the wav file and store output in ffmpeg_duration_command_output
	std::string ffmpeg_duration_command = "ffmpeg -i ";
	ffmpeg_duration_command += "\"";
	ffmpeg_duration_command += wav;
	ffmpeg_duration_command += "\"";
	ffmpeg_duration_command += " 2>&1"; // redirect stderr to stdout, ffmpeg use stderr when there is no output file
	std::string ffmpeg_duration_command_output = exec_get_return(ffmpeg_duration_command.c_str());

	int seconds = 0;
	//process ffmpeg output to find duration of file
	std::istringstream f(ffmpeg_duration_command_output);
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

int main(int argc, char** args) {

   if (!parse_args(argc, args))
   {
	   printUsage();
	   return 0;
   }

   deleteTempWav();

   createWavTempFile(input_filename);
   
   //calculate duration, chunk size, number of chunks, etc..   
   int duration = getWavDuration(TEMP_WAV_FILE);
   int chunkSize = interval_minutes * 60;
   int chunks = duration / chunkSize;
   int lastChunk = duration % chunkSize;
   
   SYSTEMTIME thesystemtime;
   GetSystemTime(&thesystemtime); //will use later
   thesystemtime.wSecond = 0;
   thesystemtime.wHour = hour_creation;
   
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
	 {
		 wavOutputss << 0;
	 }
     wavOutputss << i << ". " << output_filename;
     std::string wavOutput(wavOutputss.str());

     //split
     std::stringstream ffmpegCommand;
     ffmpegCommand << "ffmpeg -i " << "\"" << TEMP_WAV_FILE << "\"" << " -ss " << start_offset << " -t " << chunkSize << " " << "\"" << wavOutput << ".wav\"";
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
	 
	 //every chunk, increase 1 minute
	 thesystemtime.wMinute = i;
	 if (i > 0 && i % 60 == 0)
	 {
		 thesystemtime.wHour += 1;
	 }
	 
	 SystemTimeToFileTime(&thesystemtime,&creation);
	 SystemTimeToFileTime(&thesystemtime,&lastAccess);
	 SystemTimeToFileTime(&thesystemtime,&lastWrite);
	 
	 if (!SetFileTime(h_mp3_file, &creation, &lastAccess, &lastWrite))
	 {
		 std::cout << "cannot change file attributes: error " << GetLastError() << std::endl;
		 return -1;
	 }
	 
	 CloseHandle(h_mp3_file);
   }

   deleteTempWav();

   return 0;
}

void deleteTempWav()
{
	std::string del_comm = "del ";
	del_comm += TEMP_WAV_FILE;
	system(del_comm.c_str());
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

//true if all required args are passed
bool parse_args(int argc, char** args)
{
	input_filename = std::string(args[argc - 1]);
	bool iArg = false;
	bool bArg = false;
	bool oArg = false;
	
  for (int i = 0; i < argc; i++)
  {
      if (std::string(args[i]) == "-i")
      {
        interval_minutes = atoi(args[i + 1]);
		iArg = true;
      }

      if (std::string(args[i]) == "-b")
      {
        bitrate = atoi(args[i + 1]);
		bArg = true;
      }

      if (std::string(args[i]) == "-o")
      {
        output_filename = std::string(args[i + 1]);
		oArg = true;
      }
	  
	  if (std::string(args[i]) == "-h")
      {
        hour_creation = atoi(args[i + 1]);
      }
  }
  
  return iArg && bArg && oArg;
  
}