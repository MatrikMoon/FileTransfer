#include "fileTransfer.h"
#include "connection.h"

int testThing(Connection * c, std::string s) {
    c->sendTCP(s);
}

int parseFile(Server *c, char * buf, int length) {
    //Ensure we don't waste processing time if it's not a file
    if (strncmp(buf, "/file", 5) != 0) {
        return 0;
    }
    if (strncmp(buf, "/file-data:", 11) == 0) {
        if (strncmp(buf, "/file-data:start:", 17) == 0) {
            char * parse = &buf[19];

            printf("PARSE: %s\n", parse);

            char * pch = strchr(parse, ':');
            for (int i = 0; pch != 0; i++) {
                printf("DATA %d: %s", (int)(pch - parse + 1), pch);
            }
        }
        printf("%s\n", buf);
    }
    if (strncmp(buf, "/file-test", 10) == 0) {
        FILEPARTS f;
        //printf("TEST:%s\n", build_chunk_header("client", "eee", 1, f));
        send_file(c, "test.txt");
    }
    else if (strncmp(buf, "/file-end", 9) == 0) {
        testThing(c, "hi");
    }
    return 0;
}

int send_file(Server *c, std::string file) {
    FILESTATS f;
    std::string fh = build_file_header(file, f);
    c->sendUDP(fh);
    printf("%s\n", fh.c_str());
    for (int i = 0; i < calculate_chunk_number(file); i++) {
        send_chunk_patch(c, file, f.md5, i);
    }
}

int send_chunk_patch(Server *c, std::string file, std::string file_md5, int chunk_number) {
    FILEPARTS f;
    char * header = build_chunk_header(file, file_md5, chunk_number, f);
    c->sendUDP(header, f.full_size);
    //append_to_file("testttt", f.data, f.data_size);
    printf("Appending %d bytes: %s\n", f.full_size, f.full_header);
}

std::string build_file_header(std::string file, FILESTATS &f) {
    f.md5 = md5_file(file);
    f.size = get_file_size(file);
    
    std::ostringstream ret;
    ret << "/file-data:start:{";
    ret << f.md5;
    ret << ":";
    ret << calculate_chunk_number(file);
    ret << ":";
    ret << f.size;
    ret << "}";

    return ret.str();
}

char * build_chunk_header(std::string file, std::string file_md5, int chunk_number, FILEPARTS &f) {
    int data_size = 0;
    unsigned char * data = get_chunk_data(file, chunk_number, data_size);
    
    std::ostringstream ret;
    ret << "/file-data:part:";
    ret << chunk_number;
    ret << ":{";
    ret << file_md5;
    ret << ":";
    ret << md5(data);
    ret << ":";

    int size = data_size + strlen(ret.str().c_str()) + 2; //"2" is for the "}\0" that we'll be appending

    unsigned char * header  = new unsigned char[size];
    memcpy(header, ret.str().c_str(), strlen(ret.str().c_str())); //+1 for null terminator

    int current_pos = strlen(ret.str().c_str());

    memcpy(header + current_pos, data, data_size);

    current_pos += data_size;

    memcpy(header + current_pos, "}", strlen("}") + 1); //+1 for null terminator
    
    if (&f != 0) {
        f.full_size = size;
        f.data_size = data_size;
        f.md5 = md5(data);
        f.data = (char*)data;
        f.full_header = (char*)header;
    }
    return (char *)header;
}

char * get_chunk_from_header(std::string file, char * header, FILEPARTS &f) {

}

unsigned char * get_chunk_data(std::string file, int chunk_number, int &data_size) {
    std::ifstream input_file(file.c_str(), std::ifstream::binary);
    int skip_to = chunk_number * CHUNK_SIZE;
    int total_size = get_file_size(file);

    data_size = CHUNK_SIZE;
    if ((total_size - skip_to) < CHUNK_SIZE) {
        data_size = total_size - skip_to;
    }

    unsigned char * buffer = new unsigned char[data_size];
    
    input_file.seekg((std::streampos)skip_to);

    input_file.read((char *)buffer, data_size);
    input_file.close();
    return buffer;
}

std::string get_chunk_md5(std::string file, int chunk_number) {
    int data_size = 0;
    return md5((unsigned char*)get_chunk_data(file, chunk_number, data_size));
}

int calculate_chunk_number(std::string file) {
    int file_size = get_file_size(file);
    int chunk_number = file_size / CHUNK_SIZE;
    if ((file_size % CHUNK_SIZE) != 0) chunk_number++; //Add one for the last bit of data
    return chunk_number;
}

int get_file_size(std::string file) {
    std::streampos fsize = 0;
    std::ifstream input_file(file.c_str(), std::ifstream::binary);

    fsize = input_file.tellg();
    input_file.seekg( 0, std::ios::end );
    fsize = input_file.tellg() - fsize;
    input_file.close();

    return (int)fsize;
}

int append_to_file(std::string file, char * buf, int length) {
    std::ofstream out(file.c_str(), std::ofstream::binary | std::ofstream::app);
    out.seekp(std::ios::end);
    out.write(buf, length);
    return 0;
}

std::string md5_file(std::string file) {
    std::string accumulated = "";
    unsigned char buffer[CHUNK_SIZE];
    std::ifstream input_file(file.c_str(), std::ifstream::binary);
    while (input_file.read((char *)buffer, CHUNK_SIZE))
    {
        accumulated.append(md5(buffer));
        bzero(buffer, CHUNK_SIZE);
        //output_file.write((char *)buffer, CHUNK_SIZE);
    }
    input_file.close();

    return md5((unsigned char*)accumulated.c_str());
}

std::string md5(unsigned char * buffer) {
    unsigned char result[MD5_DIGEST_LENGTH];

    MD5(buffer, sizeof(buffer), result);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for (int i = 0; i < sizeof(result); i++)
    {
        sout<<std::setw(2)<<(long long)result[i];
    }
    
    return sout.str();
}