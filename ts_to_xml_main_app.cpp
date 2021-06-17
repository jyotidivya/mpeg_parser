
#include "ts_to_xml_main_app.hpp"

FILE* outputFile;

int main(int argc, char* argv[])
{
	if(argc <3)
	{
		printf("Usage: ts_to_xml_app <input ts filepath> <output xml filepath>\n");
		return 1;
	}
	char *filename = argv[1];
	char *outputFilename = argv[2];
	puts(filename);
	
	//open the output xml file.
	outputFile = fopen(outputFilename, "w+");
	
	//read input ts file
	read_ts_file(filename);
	
	return 0;
}

