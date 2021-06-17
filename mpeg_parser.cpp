
#include "ts_to_xml_main_app.hpp"
unsigned short int m_program_map_pid, m_program_number;
extern FILE* outputFile;

int read_ts_file(char* filename)
{
	unsigned char packetData[188];
	FILE* tsFile;
	
	//open the input ts file.
	tsFile = fopen(filename, "rb");
	while(!feof(tsFile))
	{
		//read 188 bytes (1 packet) each time from ts file
		fread(packetData,sizeof(char),188,tsFile);
		//process each packet
		process_packet(packetData);
	}

	return 0;
}

void process_packet(unsigned char* packetData)
{
	unsigned char* p = packetData;
	//check start byte is equal to 0x47 or not
	if(p[0] == 0x47 || p[4] == 0x47)
	{
		//go to next location
		p++;
		//read pid from packet. Byte 2 and byte 3
		unsigned short int pid = read_2_bytes(p);
		p = p+2;
		
		//read payload start indicator
		unsigned char payload_unit_start_indicator = (pid & 0x4000) >> 14;
		pid &= 0x1FFF;
		
    	p++;
		
		//write pid to xml file
    	fprintf(outputFile, "<pid>0x%x</pid>\n", pid);
		
		//check for payload start
    	bool payload_unit_start = (1==payload_unit_start_indicator);
		
		//process each pid
    	process_pid(pid, p, payload_unit_start);	
	}
	
	else
	{
		//start bit is not equal to 0x47
		//write error message to xml file
		fprintf(outputFile, "<error>Packet does not start with 0x47</error>\n");
		fprintf(outputFile, "</packet>\n");
	}
}

void process_pid(unsigned short int pid, unsigned char *p, bool payload_unit_start)
{
	//if pid == 0, calculate PAT
	if(pid == 0x00)
	{
		static bool wantPat = true;
		if(wantPat)
		{
			//write payload start indicator to xml file
			fprintf(outputFile, "<payload_unit_start_indicator>0x%x</payload_unit_start_indicator>\n", payload_unit_start ? 1 : 0);
			//form PAT table
			read_pat(p, payload_unit_start);
			fprintf(outputFile, "</packet>\n");
		}
	}
	
	// pid is equal to program map variable
	else if(pid == m_program_map_pid)
    {
        fprintf(outputFile, "<pid>0x%x</pid>\n", pid);
        fprintf(outputFile, "<payload_unit_start_indicator>0x%x</payload_unit_start_indicator>\n", payload_unit_start ? 1 : 0);

        read_pmt(p, payload_unit_start);
        fprintf(outputFile, "</packet>\n");

    }
	else if(pid >= 0x10 && pid <= 0x1FFE)
	{
		char stream_type = *p;
        p++;
        
		//write input stream type to xml file.
        if(stream_type == 0x2)
        	fprintf(outputFile, "<type_name>%s</type_name>\n", "video");
        if(stream_type == 0x4)
        	fprintf(outputFile, "<type_name>%s</type_name>\n", "audio");
	}

}

unsigned short int read_2_bytes(unsigned char *p)
{
    unsigned short int ret = *(p++);
    ret <<= 8;
    ret |= *(p++);
    return ret;
}

void read_pat(unsigned char *p, bool payload_unit_start)
{
	char payload_start_offset = 0;
	//calculate PAT table id
	if(payload_unit_start)
    {
        payload_start_offset = *p;
        p++;
        p = p+payload_start_offset;
    }
	// read table id
    char table_id = *p;
	p++;
	
	//read section length
	unsigned short int section_length = read_2_bytes(p);
	section_length &= 0xFFF;

	unsigned char *p_section_start = p;
	
	// write PAT info to xml file.
	fprintf(outputFile, "PAT\n");
	fprintf(outputFile, "<table_id>0x%x</table_id>\n", table_id);

	while ((p - p_section_start) < (section_length - 4))
	{
	    m_program_number = read_2_bytes(p);
	    p = p+2;
	    m_program_map_pid = read_2_bytes(p);
	    p = p+2;
	    m_program_map_pid &= 0x1FFF;
		
		
		//write program info to xml file
	    fprintf(outputFile, "<program>\n");
	    fprintf(outputFile, "<number>%d</number>\n", m_program_number);
	    fprintf(outputFile, "<program_map_pid>0x%x</program_map_pid>\n", m_program_map_pid);
	    fprintf(outputFile, "</program>\n");
	}

    fprintf(outputFile, "</program_association_table>\n");

}

void read_pmt(unsigned char *p, bool payload_unit_start)
{
	char payload_start_offset = 0;
	//calculate PMT table id
    if(payload_unit_start)
    {
        payload_start_offset = *p;
        p++;
        p = p+payload_start_offset;
    }
	
	//read table id
    char table_id = *p;
    p++;
    unsigned short int section_length = read_2_bytes(p);
    section_length &= 0xFFF;

    unsigned char *p_section_start = p;
	
	//read program number
    unsigned short int program_number = read_2_bytes(p);
    
	//write PMT info to xml file
    fprintf(outputFile, "<program_map_table>\n");
    fprintf(outputFile, "<table_id>0x%x</table_id>\n", table_id);
    fprintf(outputFile, "<program_number>%d</program_number>\n", program_number);
    
    int stream_count = 0;

    while((p - p_section_start) < (section_length - 4))
    {
        char stream_type = *p;
        p++;

        unsigned short elementary_pid = read_2_bytes(p);
        p=p+2;
        elementary_pid &= 0x1FFF;

        unsigned short es_info_length = read_2_bytes(p);
        p=p+2;
        es_info_length &= 0xFFF;

        p += es_info_length;

		//write stream info to xml file
        fprintf(outputFile, "<stream>\n");
        fprintf(outputFile, "<pid>0x%x</pid>\n", elementary_pid);
        fprintf(outputFile, "<type_number>0x%x</type_number>\n", stream_type);
        if(stream_type == 0x2)
        	fprintf(outputFile, "<type_name>%s</type_name>\n", "video");
        if(stream_type == 0x4)
        	fprintf(outputFile, "<type_name>%s</type_name>\n", "audio");
        fprintf(outputFile, "</stream>\n");

        stream_count++;
    }

}
