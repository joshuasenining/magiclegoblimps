#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

#include <boost/asio.hpp>

#include "../protocol.h"
#include "../DataFile.h"
#include "../Vector_ts.h"
#include "../Object.h"

int main() {
    // let's create some robots to send to the server
    robotInit* robots = new robotInit[3];

    robots[0].RID = 0;
    robots[0].x = 0;
    robots[0].y = 1;
    robots[0].cameraType = 0;
    robots[0].VideoURL = new std::string("http://10.10.1.100:7500");

    robots[1].RID = 1;
    robots[1].x = 1;
    robots[1].y = 4;
    robots[1].cameraType = 0;
    robots[1].VideoURL = new std::string("http://10.10.1.101:7500");

    robots[2].RID = 2;
    robots[2].x = 2;
    robots[2].y = 3;
    robots[2].cameraType = 1;
    robots[2].VideoURL = new std::string("http://10.10.1.102:7500");

    byteArray arr;
    if (write_data(P_ROBOT_INIT, robots, 3, &arr) < 0) {
        std::cerr << "Failed to create write array\n";
        return -1;
    }

    robotUpdate* update = new robotUpdate[3];

    update[0].RID = 0;
    update[0].x = 1;
    update[0].y = 2;
    update[0].dir = 0;
    update[0].listSize = 0;
    /*
    update[0].listSize = 1;
    update[0].objects = new int[1];
    update[0].objects[0] = 1;
    update[0].qualities = new float[1];
    update[0].qualities[0] = 0.5;
    update[0].xs= new int[1];
    update[0].xs[0] = 1;
    update[0].ys= new int[1];
    update[0].ys[0] = 1;
    */

    update[1].RID = 1;
    update[1].x = 2;
    update[1].y = 4;
    update[1].dir = 2;
    update[1].listSize = 2;
    update[1].objects = new int[2];
    update[1].objects[0] = 2;
    update[1].objects[1] = 3;
    update[1].qualities = new float[2];
    update[1].qualities[0] = 0.5;
    update[1].qualities[1] = 0.25;
    update[1].xs= new int[2];
    update[1].xs[0] = 2;
    update[1].xs[1] = 3;
    update[1].ys= new int[2];
    update[1].ys[0] = 2;
    update[1].ys[1] = 3;

    update[2].RID = 2;
    update[2].x = 3;
    update[2].y = 3;
    update[2].dir = 1;
    update[2].listSize = 3;
    update[2].objects = new int[3];
    update[2].objects[0] = 1;
    update[2].objects[1] = 3;
    update[2].objects[2] = 4;
    update[2].qualities = new float[3];
    update[2].qualities[0] = 0.1;
    update[2].qualities[1] = 0.2;
    update[2].qualities[2] = 0.5;
    update[2].xs= new int[3];
    update[2].xs[0] = 1;
    update[2].xs[1] = 2;
    update[2].xs[2] = 3;
    update[2].ys= new int[3];
    update[2].ys[0] = 1;
    update[2].ys[1] = 2;
    update[2].ys[2] = 3;

    DataFile *objfile = new DataFile("object.dat", DataFile::OBJECT);
    Vector_ts<Object*> *objects = (Vector_ts<Object*>*) objfile->read();

    byteArray updArr;
    if (write_data(P_ROBOT_UPDATE, update, 3, &updArr)) {
        std::cerr << "Failed to create update array\n";
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    struct hostent *hostPtr = NULL;
    struct sockaddr_in serverName = { 0 };

    //in_addr_t ip = inet_addr("10.176.14.94");
    //hostPtr = gethostbyaddr(&ip, 4, AF_INET);
    hostPtr = gethostbyname("localhost");


    serverName.sin_family = AF_INET;
    serverName.sin_port = htons(9999);
    memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

    int conn = connect(sock, (struct sockaddr*)&serverName, sizeof(serverName));

    if (conn == -1) {
        std::cerr << "Failed to connect()\n";
        return -1;
    }

    conn = write(sock, arr.array, arr.size);
    std::cout << "Wrote robot data to socket:\n";
    for (int i = 0; i < arr.size; ++i) {
        printf("%02X ", arr.array[i]);
    }
    std::cout << std::endl;

    sleep(1);

    conn = write(sock, updArr.array, updArr.size);
    std::cout << "Wrote update data to socket:\n";
    for (int i = 0; i < updArr.size; ++i) {
        printf("%02X ", updArr.array[i]);
    }
    std::cout << std::endl;
    char buffer[1];

    sleep(1);

    object *objs = new object[objects->size()];
    /*objs[0].OID = 6;
    objs[0].name = new std::string("Obj 6");
    objs[0].color_size = 10;
    objs[0].color = "1234567890";
    objs[0].box_size = 1;
    objs[0].box = new char[1];
    objs[0].box[0] = 'x';*/

    Vector_ts<Object*>::iterator it;
    int i = 0;
    for(it = objects->begin(); it != objects->end(); ++it){
	objs[i].name = new std::string((*it)->getName());
	objs[i].OID = (*it)->getOID();
	objs[i].color_size = (*it)->getColorsize(); 
	objs[i].color = (*it)->getColor();
	objs[i].box_size = (*it)->getBoxsize();
	objs[i].box = (*it)->getBox();
	++i;
    }

    
    byteArray objArr;
    if (write_data(P_OBJECT, objs, objects->size(), &objArr)) {
        std::cerr << "Failed to create object array\n";
        return -1;
    }

    conn = write(sock, objArr.array, objArr.size);
    std::cout << "Wrote object data to socket:\n";
    for (int i = 0; i < objArr.size; ++i) {
        printf("%02X ", objArr.array[i]);
    }
    std::cout << std::endl;
    

    
/*
    std::cout << "Read object data from socket:\n";
    while (0 < (conn = read(sock, buffer, 1))) {
        printf("%02X ", (unsigned)*buffer);
        fflush(stdout);
    }
    std::cout << std::endl;

    if (-1 == conn)
            perror("read()");
*/

    bool loop = true;
    while(loop) {
        int command;
        std::cout << "Enter the command:\n1. RobotUpdate\n2. Quit\n";
        std::cin >> command;

        switch (command) {
            case 2:
                loop = false;
                break;

            case 1:
                {
                    int RID, x, y, dir;
                    int listSize = 0;
                    int *objects, *xs, *ys;
                    float* qual;
                    std::cout << "RID: ";
                    std::cin >> RID;

                    std::cout << "x: ";
                    std::cin >> x;

                    std::cout << "y: ";
                    std::cin >> y;

                    std::cout << "dir: ";
                    std::cin >> dir;

                    std:: cout << "listSize: ";
                    std::cin >> listSize;

                    if (listSize == 0){
                        objects = xs = ys = NULL;
                        qual= NULL;
                    } else {
                        objects = new int[listSize];
                        qual = new float[listSize];
                        xs = new int[listSize];
                        ys = new int[listSize];
                    }

                    for (int i = 0; i < listSize; ++i) {
                        std::cout << "objID: ";
                        std::cin >> objects[i];

                        std::cout << "qual: ";
                        std::cin >> qual[i];

                        std::cout << "x: ";
                        std::cin >> xs[i];

                        std::cout << "y: ";
                        std::cin >> ys[i];
                    }

                    robotUpdate* u = new robotUpdate;
                    u->RID = RID;
                    u->x = x;
                    u->y = y;
                    u->dir = dir;
                    u->listSize = listSize;
                    u->objects = objects;
                    u->qualities = qual;
                    u->xs = xs;
                    u->ys = ys;

                    byteArray uar;
                    if (write_data(P_ROBOT_UPDATE, u, 1, &uar)) {
                        std::cerr << "Failed to create update array\n";
                        return -1;
                    }

                    conn = write(sock, uar.array, uar.size);
                    std::cout << "Wrote update data to socket:\n";
                    for (int i = 0; i < uar.size; ++i) {
                        printf("%02X ", uar.array[i]);
                    }
                    std::cout << std::endl;
                }
                break;

            default:

                break;
        }
    }

    close(sock);
    return 0;
}


/* vi: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */
