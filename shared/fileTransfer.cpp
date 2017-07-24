#include "fileTransfer.h"
#include "connection.h"
#include "utils.h"

int last_id = 0;
bool done = false;

//Map of currently in-progress downloads
std::tr1::unordered_map<std::string, FILESTATS*> statlist;

int parseFile(Connection *c, char * buf, int length) {
    //Ensure we don't waste processing time if it's not a file
    if (strncmp(buf, "/file", 5) != 0) {
        return 0;
    }
    if (strncmp(buf, "/file-data:", 11) == 0) {
        if (strncmp(buf, "/file-data:start:", 17) == 0) {
            try {
                FILESTATS * f = get_super_header(buf);
                statlist[f->md5] = f;
                //f->parts = new FILEPARTS[f->parts_number];
                printf("MD5: %s\n", f->md5.c_str());
                printf("PARTS: %d\n", f->parts_number);
                printf("SIZE: %d\n", f->size);
                printf("PART SIZE: %d\n\n", f->chunk_size);
                if (file_exists("received")) {
                    remove("received");
                }
            }
            catch (int e) {
                printf("INVALID START HEADER DATA\n");
            }
        }
        else if (strncmp(buf, "/file-data:part:", 16) == 0) {
            try {
                FILEPARTS * f = get_chunk_from_header(buf, length);

                /*
                if (f->chunk_id % 2 == 0) {
                    write_chunk_to_file("received", f->data, f->data_size, f->chunk_id, CHUNK_SIZE);//f->stats->chunk_size);
                }
                */

                write_chunk_to_file("received", f->data, f->data_size, f->chunk_id, CHUNK_SIZE);//f->stats->chunk_size);

                if (last_id != f->chunk_id - 1) {
                    printf("CHUNK QUESTION %d, %d\n", last_id, f->chunk_id);
                }
                last_id = f->chunk_id;

                //Free up what we won't be using anymore
                delete[] f->data;
                f->data = 0;
            }
            catch (int e) {
                printf("INVALID HEADER DATA\n");
            }
        }
        else if (strncmp(buf, "/file-data:end:", 15) == 0) {
            end_file_transmission(buf);
        }
    }
    else if (strncmp(buf, "/file-test", 10) == 0) {
        printf("SENDING: \"%s\"\n", &buf[11]);
        send_file(c, &buf[11]);
    }
    return 1;
}

//High-level function to send a file.
//First sends the file info through tcp, then
//data through udp. If the Connection is missing
//a tcp or udp connection, returns 1
int send_file(Connection *c, std::string file) {
    if (!check_connection(c)) {
        printf("Client not connected with both TCP and UDP\n");
        return 1;
    }

    FILESTATS f;
    c->sendTCP(build_file_header(file, CHUNK_SIZE, f));
    for (int i = 0; i < calculate_chunk_number(file); i++) {
        send_chunk_patch(c, file, f.md5, i);
    }
    send_end_file(c, f.md5);
}

//This function takes a buf value and
//parses it into the necessary md5. From there
//we can pick out the FILESTATS structure from the
//unordered map statlist and use it to verify
//the integrity of the received file.
//If there's integrity issues, we'll build
//a list of chinks to retransmit and do the
//send it. This function will be called again after
//the retransmission.
void end_file_transmission(char * buf) {
    char * md5 = &buf[16];
    char * bracket = strchr(md5 + 1, '}');

    int md5_size = bracket - md5;

    char md5_s[33];
    bzero(md5_s, 33);
    strncpy(md5_s, md5, md5_size);
    
    printf("TRUE MD5: %s\n", md5_s);

    std::string current_md5 = md5_file("received");

    printf("OUR MD5: \"%s\"\n", current_md5.c_str());

    if (current_md5 == md5_s) {
        printf("FILE VERIFIED.\n");

        //verify_chunks("received", md5_s);

        //Since we're verified, we don't need all those
        //nasty structures anymore
        //free(statlist[md5_s]->parts);
    }
    else {
        printf("FILE HAS MD5: %s\n", current_md5.c_str());

        verify_chunks("received", md5_s);
    }
}

//Verify each individual piece of a file and return a list
//of invalid chunks
std::vector<int> verify_chunks(std::string file, std::string super_md5) {
    std::vector<int> missing;
    unsigned char buffer[CHUNK_SIZE];
    std::ifstream input_file(file.c_str(), std::ifstream::binary);

    int i = 0;
    while (input_file.read((char *)buffer, CHUNK_SIZE))
    {
        //printf("\"%s\" : \"%s\"", statlist[super_md5]->parts.at(i)->md5.c_str(), md5(buffer, input_file.gcount()).c_str());
        if (md5(buffer, input_file.gcount()) != statlist[super_md5]->parts.at(i)->md5) {
            //printf(" <-- !");
            missing.push_back(i);
        }
        //printf("\n");
        i++;
        bzero(buffer, CHUNK_SIZE);
    }
    input_file.close();

    return missing;
}

//Sends a simple end file transmission
void send_end_file(Connection * c, std::string md5) {
    std::ostringstream ret;
    ret << "/file-data:end:{";
    ret << md5;
    ret << "}";

    c->sendTCP(ret.str());
}

int send_chunk_patch(Connection *c, std::string file, std::string file_md5, int chunk_number) {
    FILEPARTS f;
    char * header = build_chunk_header(file, file_md5, chunk_number, f);
    c->sendUDP(header, f.full_size);

    //Free up pointers we won't need
    delete[] f.data;
    delete[] f.full_header;
}

//Takes a file name, desired chunk size, and FILESTATS structure
//Returns a string suitable to send as a file info header
std::string build_file_header(std::string file, int chunk_size, FILESTATS &f) {
    f.md5 = md5_file(file);
    f.size = get_file_size(file);
    f.parts_number = calculate_chunk_number(file);
    
    std::ostringstream ret;
    ret << "/file-data:start:{";
    ret << f.md5;
    ret << ":";
    ret << f.parts_number;
    ret << ":";
    ret << chunk_size;
    ret << ":";
    ret << f.size;
    ret << "}";

    return ret.str();
}

//Takes a filename to read from, md5 of the file, desired chunk number,
//and a reference to a FILEPARTS structure.
//Returns a set of bytes suitable to send as a specific chunk data
//Also fills out necessary info in the FILEPARTS structure
char * build_chunk_header(std::string file, std::string file_md5, int chunk_number, FILEPARTS &f) {
    int data_size = 0;
    unsigned char * data = get_chunk_data(file, chunk_number, data_size);
    
    std::ostringstream ret;
    ret << "/file-data:part:";
    ret << chunk_number;
    ret << ":{";
    ret << file_md5;
    ret << ":";
    ret << md5(data, data_size);
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
        f.md5 = md5(data, data_size);
        f.data = (char*)data;
        f.full_header = (char*)header;
        f.chunk_id = chunk_number;
    }
    return (char *)header;
}

//Parses a file info header and returns a FILESTATS structure
FILESTATS * get_super_header(char * header) {
    char * md5 = &header[18]; //Check usage for magic number explanation
    char * parts = strchr(md5, ':');
    char * chunk_size = strchr(parts + 1, ':');
    char * size = strchr(chunk_size + 1, ':');
    char * bracket = strchr(size + 1, '}');

    int size_size = (bracket - 1) - size;
    int chunk_size_size = (size - 1) - chunk_size;
    int parts_size = (chunk_size - 1) - parts;
    int md5_size = parts - md5; //Christ, just don't ask.

    char * md5_s = new char[md5_size + 1];
    md5_s[md5_size] = '\0'; //Null terminate the string
    strncpy(md5_s, md5, md5_size);

    char * parts_s = new char[parts_size + 1];
    parts_s[parts_size] = '\0';
    strncpy(parts_s, parts + 1, parts_size); //'parts + 1' to avoid the ':' character
    
    char * chunk_size_s = new char[chunk_size_size + 1];
    chunk_size_s[chunk_size_size] = '\0';
    strncpy(chunk_size_s, chunk_size + 1, chunk_size_size); //'chunk_size_size + 1' to avoid the ':' character

    char * size_s = new char[size_size + 1];
    size_s[size_size] = '\0';
    strncpy(size_s, size + 1, size_size); //'size + 1' to avoid the ':' character


    //Build resulting filestats pointer and return
    FILESTATS * f = new FILESTATS;
    f->size = atoi(size_s);
    f->chunk_size = atoi(chunk_size_s);
    f->md5 = std::string(md5_s); //Ensure we only get what we need
    f->parts_number = atoi(parts_s);
    return f;
}

//Parses a data header and returns a FILEPARTS structure
FILEPARTS * get_chunk_from_header(char * header, int length) {
    char * chunk_id = &header[16]; //Check usage for magic number explanation
    char * info_start = strchr(chunk_id, ':'); //There's another colon after the part id, so...
    char * super_md5 = strchr(info_start + 1, '{');
    char * md5 = strchr(super_md5 + 1, ':');
    char * data = strchr(md5 + 1, ':');

    int md5_size = (data - 1) - md5;
    int super_md5_size = (md5 - 1) - super_md5;
    int chunk_id_size = info_start - chunk_id; //Christ, just don't ask.
    int data_size = (&header[length] - 1) - data - 2; //First number is a pointer to the
                                                    //last character recieved (which would
                                                    //be '}', hence a -1). Second is
                                                    //the index of the colon before 'data'
                                                    //begins, (hence another -1). Pulled
                                                    //the third -1 out of my ass.

    char * chunk_id_s = new char[chunk_id_size + 1];
    chunk_id_s[chunk_id_size] = '\0';
    strncpy(chunk_id_s, chunk_id, chunk_id_size);

    char * super_md5_s = new char[super_md5_size + 1];
    super_md5_s[super_md5_size] = '\0';
    strncpy(super_md5_s, super_md5 + 1, super_md5_size);

    char * md5_s = new char[md5_size + 1];
    md5_s[md5_size] = '\0';
    strncpy(md5_s, md5 + 1, md5_size); //'md5 + 1' to avoid the ':' character
    
    char * data_s = new char[data_size + 1]; //No need for null termination
    memcpy(data_s, data + 1, data_size); //'data + 1' to avoid the ':' character
                                         //also 'memcpy' to account for null bytes
                                         
    //Build resulting fileparts pointer and return
    FILEPARTS * f = new FILEPARTS;//new FILEPARTS;
    f->chunk_id = atoi(chunk_id);
    f->full_size = length;
    f->data_size = data_size;
    f->data = data_s;
    f->full_header = header;
    f->md5 = md5_s;
    f->super_md5 = super_md5_s;
    f->stats = statlist[super_md5_s];

    if (!statlist[f->super_md5]) {
        printf("Uh-oh. Looks like we're segfaulting.\n");
        printf("BMD5: \"%s\"\n", f->super_md5.c_str());
        printf("BPOINTER: %d\n", statlist[f->super_md5]);
    }

    statlist[f->super_md5]->parts.push_back(f);// = f; //Set the proper index in the super array to equal us
    return f;
}

//Takes a file name, desired chunk number and reference to an int
//Returns the raw bytes for that specific chunk, and the size of
//the data.
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

//Create file
void init_file(std::string file) {
    std::ofstream out(file.c_str(), std::ios_base::binary);
    out.close();
}

int write_chunk_to_file(std::string file, char * buf, int length, int chunk_number, int chunk_size) {
    //Create file if it doesn't exist
    if (!file_exists("received")) init_file("received");

    std::fstream out(file.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    int pos = chunk_number * chunk_size;

    out.seekp(pos, std::ios_base::beg);
    out.write(buf, length);

    out.close();
    return 0;
}

int append_to_file(std::string file, char * buf, int length) {
    std::ofstream out(file.c_str(), std::ofstream::binary | std::ofstream::app);
    out.seekp(std::ios::end);
    out.write(buf, length);
    out.close();
    return 0;
}

std::string md5_file(std::string file) {
    std::string accumulated = "";
    unsigned char buffer[CHUNK_SIZE];
    std::ifstream input_file(file.c_str(), std::ifstream::binary);
    while (input_file.read((char *)buffer, CHUNK_SIZE))
    {
        accumulated.append(md5(buffer, input_file.gcount()));
        bzero(buffer, CHUNK_SIZE);
    }
    input_file.close();

    //printf("ACC: %s\n", accumulated.c_str());

    return md5((unsigned char*)accumulated.c_str(), accumulated.length());
}

std::string md5(unsigned char * buffer, int length) {
    unsigned char result[MD5_DIGEST_LENGTH];

    MD5(buffer, length, result);

    std::ostringstream sout;
    sout<<std::hex<<std::setfill('0');
    for (int i = 0; i < sizeof(result); i++)
    {
        sout<<std::setw(2)<<(long long)result[i];
    }
    
    return sout.str();
}

//Ensures the connection has both tcp and udp counterparts
bool check_connection(Connection * c) {
    return (c->hasTCP() && c->hasUDP());
}