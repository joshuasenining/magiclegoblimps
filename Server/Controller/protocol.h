/*
 * protocol.h 
 *
 * Modified on:    $Date$
 * Last Edited By: $Author$
 * Revision:       $Revision$
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <cstring>

#define P_ROBOT_INIT 1
#define P_ROBOT_UPDATE 2
#define P_OBJECT 3

/* the protocol converts arrays of the defined structs into byte arrays 
 * from transmission over the socket the protocol is pretty much as 
 * follows, the first byte if the type of struct in the message,
 * the second and third bytes are short representing the total
 * number of structs transmitted. Each struct is prefixed by a two byte
 * short listing the size of that particular struct instance followed by
 * the struct itself. after that each type of struct is unique.
 */

struct robotInit{
    int RID;
    int x;
    int y;
    std::string* VideoURL;
};
struct robotUpdate{
    int RID;
    int x;
    int y;
};
struct object{
    int OID;
    std::string* name;
    char* color;
    size_t color_size;
};

struct byteArray{
    char* array;
    int size;
};
struct readReturn{
    void* array;
    int size;
    int type;
};

inline void write_data(robotInit* data, short number, byteArray* byte_ptr){
    char* array;
    unsigned short* sizes = new unsigned short[number];
    int overall_size = 0;
    char** structs = new char*[number];
    for(int i = 0; i < number; ++i){
        short str_len = data[i].VideoURL->size() + 1;
        const char* url = data[i].VideoURL->c_str();
        short size = sizeof(int)*3 + str_len + sizeof(short)*2;
        structs[i] = new char[size];

        //put the size on the top of the array
        char* ref = (char*)&size;
        for(int j = 0; j < 2; ++j){
            structs[i][j] = ref[j];
        }

        //push rid on the array
        ref = (char*)&(data[i].RID);
        for(int j = 2; j < 6; ++j){
            structs[i][j] = ref[j-2];
        }

        //push x
        ref = (char*)&(data[i].x);
        for(int j = 6; j <10; ++j){
            structs[i][j] = ref[j-6];
        }

        //push y
        ref = (char*)&(data[i].y);
        for(int j = 10; j < 14; ++j){
            structs[i][j] = ref[j-10];
        }

        //push the string size
        ref = (char*)&str_len;
        for(int j = 14; j < 16; ++j){
            structs[i][j] = ref[j-14];
        }

        //push the string
        for(int j = 16; j < 16+str_len; ++j){
            structs[i][j] = url[j - 16];
        }

        //add the length of this array to the total length and store it
        sizes[i] = size;
        overall_size += size;
    }

    //intiallize the array to return
    array = new char[overall_size + 3];

    //set type
    array[0] = P_ROBOT_INIT;

    //push number of elements;
    char* ref = (char*)&number;
    for(int i = 1; i < 3; ++i){
        array[i] = ref[i -1];
    }
    
    //push each array onto the array
    int position = 3;
    for(int i = 0; i < number; ++i){
             memcpy(&array[position], structs[i], sizes[i]);
             position += sizes[i];
    }
    //clean up
    delete[] sizes;
    for(int i = 0; i < number; ++i){
        delete[] structs[i];
    }
    delete[] structs;
    byte_ptr->array = array;
    byte_ptr->size = overall_size + 3;

}


inline void write_data(robotUpdate* data, short number, byteArray* byte_ptr){
    char* array;
    unsigned short* sizes = new unsigned short[number]; 
    int overall_size = 0;
    char** structs = new char*[number];
    //for each struct
    for(int i = 0; i < number; ++i){
        short size = sizeof(int) * 2;
        structs[i] = new char[size];

        //push size;
        memcpy(structs[i], &size, sizeof(short));

        //push x
        

    }
}

inline int readRobotInit(void* array, robotInit* &robots) {
    char* arr = (char*)array;
    char* current = arr+1;
    char* ref;

    // overall size
    short overall_size;
    ref = (char*)&overall_size;
    ref[0] = current[0]; current++;
    ref[1] = current[0]; current++;

    robots = new robotInit[overall_size];

    for (int i = 0; i < overall_size; ++i) {
        short size;
        int rid;
        int x;
        int y;
        short str_len;
        char* url;

        
        // retreive size
        ref = (char*)&size;
        ref[0] = current[0]; current++;
        ref[1] = current[0]; current++;

        // retrieve RID
        ref = (char*)&rid;
        for (int j = 0; j < sizeof(int); ++j) {
            ref[j] = current[0]; current++;
        }

        // retrieve x
        ref = (char*)&x;
        for (int j = 0; j < sizeof(int); ++j) {
            ref[j] = current[0]; current++;
        }

        // retrieve y
        ref = (char*)&y;
        for (int j = 0; j < sizeof(int); ++j) {
            ref[j] = current[0]; current++;
        }

        // retrieve str_len
        ref = (char*)&str_len;
        for (int j = 0; j < sizeof(short); ++j) {
            ref[j] = current[0]; current++;
        }

        // retrieve url
        url = new char[str_len];
        for (int j = 0; j < str_len; ++j) {
            url[j] = current[0]; current++;
        }

        // build the robotInit struct
        robots[i].RID = rid;
        robots[i].x = x;
        robots[i].y = y;
        robots[i].VideoURL = new std::string(url);

        delete[] url;
    }
    return overall_size;
}

// takes a void pointer to the data as output from write_data and an 
inline int read_data(void* array, readReturn* ret){

    switch( (char) ((char*)array)[0] ) {
        case P_ROBOT_INIT:
            {
                robotInit* arr;
                int count = readRobotInit(array, arr);

                // Create a readReturn if we have a NULL pointer
                if (!ret)
                    ret = new readReturn;
                ret->array = (void*)arr;
                ret->size = count;
                ret->type = P_ROBOT_INIT;

                return count;
            }

        case P_ROBOT_UPDATE:

            break;

        case P_OBJECT:

            break;

        default:

            std::cerr << "Attempt to read unknown type\n";
    }
    return -1;
}

#endif /* PROTOCOL_H_ */

/* vi: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */