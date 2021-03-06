/*
 * DataFile.cpp
 *
 * Modified on:    $Date$
 * Last Edited By: $Author$
 * Revision:       $Revision$
 */

#include "DataFile.h"

DataFile::DataFile(std::string filename, type t)
    : filename_(filename), t_(t)
{
}

DataFile::~DataFile()
{
}

// Returns -1 on failure and 1 on success
int DataFile::write(void* data)
{
    struct stat status; 
    if (stat("data/", &status)) {
        // data does not exist or is unreadable
        if (mkdir("data/", O_CREAT | 0755))
            return -1;
    }
    std::string fn("data/");
    fn.append(filename_);
    std::fstream file(fn.c_str(), std::fstream::out | std::fstream::trunc | std::fstream::binary);
    int retVal = -1;
    switch (t_) {
        case ROBOT:
            {
                std::cout << "Writing ROBOT data\n";
                Vector_ts<Robot*>* robots = (Vector_ts<Robot*>*)data;
                
                robots->readLock();

                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator it_end = robots->end();
                
                // build array of robotInit structs
                robotInit* r = new robotInit[robots->size()];
                int i = 0;
                for (it = robots->begin(); it < it_end; ++it) {
                    (*it)->lock();

                    r[i].RID = (*it)->getRID();
                    r[i].x =(*it)->getXCord();
                    r[i].y =(*it)->getYCord();
                    r[i].VideoURL = new std::string((*it)->getVideoURL());

                    (*it)->unlock();
                    ++i; 
                }

                robots->readUnlock();

                byteArray byteArr;
                write_data(P_ROBOT_INIT, r, i, &byteArr);

                // delete robotInit array
                delete[] r;

                std::cout << "Writing data to ROBOT file\n";
                file.write(byteArr.array, byteArr.size);

                //delete[] byteArr.array;

                retVal = 1;
            }
            break;

        case OBJECT:
            {
                std::cout << "Writing OBJECT data\n";

                Vector_ts<Object*>* objects = (Vector_ts<Object*>*)data;

                objects->readLock();

                Vector_ts<Object*>::iterator it;
                Vector_ts<Object*>::iterator it_end = objects->end();

                // build array of object structs
                object* o = new object[objects->size()];
                int i = 0;
                for (it = objects->begin(); it < it_end; ++it) {
                    (*it)->lock();
                    o[i].OID = (*it)->getOID();
                    o[i].name = new std::string((*it)->getName());
                    o[i].color_size = (*it)->getColorsize();
                    o[i].color = (*it)->getColor();
                    o[i].box_size = (*it)->getBoxsize();
                    o[i].box = (*it)->getBox();
                    (*it)->unlock();

                    ++i;
                }

                objects->readUnlock();

                byteArray byteArr;
                write_data(P_OBJECT, o, i, &byteArr);

                std::cout << "Writing data to OBJECT file\n";
                file.write(byteArr.array, byteArr.size);

                //delete the object array
                delete[] o;

                //delete[] byteArr.array;

                retVal = 1;
            }
            break;
    }
    file.close();
    return retVal;
}

void* DataFile::read()
{
    struct stat status; 
    if (stat("data/", &status)) {
        // data does not exist or is unreadable
        if (mkdir("data/", O_CREAT | 0755))
            return NULL;
    }
    std::string fn("data/");
    fn.append(filename_);
    std::fstream file(fn.c_str(), std::fstream::in | std::fstream::binary);
    void* retVal = NULL;
    switch (t_) {
        case ROBOT:
            std::cout << "Reading ROBOT data\n";
            if (file.good()){
                // read the data here.
                std::cout << "ROBOT Datafile good.\n";

                // get length of file:
                file.seekg (0, std::fstream::end);
                int length = file.tellg();
                file.seekg (0, std::fstream::beg);

                if (length > 1024*1024*1024) {
                    std::cerr << "ROBOT datafile too large\n";
                    break;
                }
                char* buffer = new char[length];

                file.read(buffer, length);

                readReturn* readRet = new readReturn;
                read_data(buffer, readRet);

                delete[] buffer;

                // take array and construct Vector_ts from it
                Vector_ts<Robot*>* robots = new Vector_ts<Robot*>;

                robotInit* array = (robotInit*)readRet->array;

                for (int i = 0; i < readRet->size; ++i) {
                    boost::asio::ip::tcp::endpoint ep;
                    Robot* r = new Robot(ep, array[i].RID);
                    r->setXCord(array[i].x);
                    r->setYCord(array[i].x);
                    r->setVideoURL(*array[i].VideoURL);

                    robots->push_back(r);
                }
                delete[] array;

                delete readRet;

                retVal = robots;
            }
            break;

        case OBJECT:
            std::cout << "Reading OBJECT data\n";
            if (file.good()){
                // read the data here.
                std::cout << "OBJECT Datafile good.\n";

                // get the length of the file:
                file.seekg (0, std::fstream::end);
                int length = file.tellg();
                file.seekg (0, std::fstream::beg);

                if (length > 1024*1024*1024) {
                    std::cerr << "OBJECT datafile too large\n";
                    break;
                }
                char* buffer = new char[length];

                file.read(buffer, length);

                readReturn* readRet = new readReturn;
                read_data(buffer, readRet);

                delete[] buffer;

                // take array and construct Vector_ts from it
                Vector_ts<Object*>* objects = new Vector_ts<Object*>;

                object* array = (object*)readRet->array;

                for (int i = 0; i < readRet->size; ++i) {
                    Object* o = new Object(array[i].OID, *array[i].name, array[i].color, array[i].color_size, array[i].box, array[i].box_size);
                    delete[] array[i].color;
                    delete[] array[i].box;

                    objects->push_back(o);

                }

                delete[] array;

                delete readRet;

                retVal = objects;
            }
            break;
    }

    return retVal;
}

/* vi: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */
