/*
 * AdminHandler.cpp
 *
 * Modified on:    $Date$
 * Last Edited By: $Author$
 * Revision:       $Revision$
 */


#include "AdminHandler.h"
#include "controller.h"

//AdminHandler::conn_map AdminHandler::connections;

AdminHandler::AdminHandler(){
}

AdminHandler::AdminHandler(RobotHandler* robotControl_, Vector_ts<Robot*>* robots_, Vector_ts<Robot*>* inUse_, Vector_ts<Object*>* objects_, controller* cont_)
    : robotControl(robotControl_), robots(robots_), inUse(inUse_), objects(objects_), cont(cont_)
{
}

AdminHandler::~AdminHandler(){
}

void AdminHandler::onConnect(TcpServer::TcpConnection::pointer tcp_connection) {
    boost::asio::ip::tcp::endpoint endpoint = tcp_connection->socket().remote_endpoint();
    
    tcp_connection->releaseSocket();

    //connections[endpoint] = tcp_connection;
    boost::thread connThread(&AdminHandler::threaded_on_connect, this, tcp_connection);
}

void AdminHandler::threaded_on_connect(TcpServer::TcpConnection::pointer tcp_connection){

    boost::shared_ptr<session> sess(new session(tcp_connection, robots, inUse,  robotControl, objects, cont));
    sessions.push_back(sess);
    sess->start();

    std::cout << "[AH] thread exiting\n";
}

AdminHandler::session::session(TcpServer::TcpConnection::pointer tcp_, Vector_ts<Robot*>* robots_, Vector_ts<Robot*>* inUse_,RobotHandler* robotHandler, Vector_ts<Object*>* objects_, controller* cont_) 
    : conn_(tcp_), robots(robots_), inUse(inUse_), objects(objects_), robotControl(robotHandler), cont(cont_), closing(false)
{
}

AdminHandler::session::~session()
{
}

void AdminHandler::session::start(){

    tcp::socket &sock = conn_->socket();
    boost::asio::async_read_until(sock, read_message_.buffer(), '\n', 
        boost::bind(&AdminHandler::session::read_handler, this, 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    conn_->releaseSocket();
    
}

void AdminHandler::session::write_handler(const boost::system::error_code& error,  std::size_t bytes_transferred) {
}

void AdminHandler::session::read_handler(const boost::system::error_code& error,  std::size_t bytes_transferred) {
    if (error == boost::asio::error::eof){
        close();
        return;
    } else if (error == boost::asio::error::operation_aborted) {
        close();
        return;
    } else if (error) {
        std::cerr << "[AH] unknown error in read handler\n";
        close();
        return;
    }

    tcp::socket &sock = conn_->socket();;

    std::string s(read_message_.data(bytes_transferred));
    read_message_.consume(bytes_transferred);

    std::cout << "[AH] Got string \"" << s.substr(0, s.size() - 1) << "\"\n";
    std::cout.flush();

    command cmd;
    cmd.arg = 0;
    Robot* subject = NULL;

    if(!s.compare(std::string("shutdown\n"))){
        // we need to shutdown the system.

        cmd.RID = -1;
        cmd.cmd = P_CMD_SHUTDOWN;
        cmd.arg = 0;
        robotControl->sendCommand(&cmd);
        conn_->releaseSocket();
        cont->shutdown();
        
        return;
    }


    //check to make sure that the input is what we want
    if(s.find('$', 0) != std::string::npos){
        
        //parse the command id
        std::string token = s.substr(0, s.find('$', 0));
        s =  s.substr( token.size() + 1, std::string::npos);
        //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

        robots->readLock();
        int switchvar = atoi(token.c_str());
        bool isUsed = false;
        //switch on the command
        switch(switchvar){
            case P_CMD_FWD:
            {
                
                cmd.cmd = P_CMD_FWD;
                
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int robotid = atoi(token.c_str());
                //std::cout << "[AH] got RID:" << robotid << "\n";

                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator used_it;

                for(it = robots->begin(); it != robots->end(); ++it){
                    (*it)->lock();
                    if((*it)->getGlobalID() == robotid){
                        subject = (*it);
                        break;
                    }
                    (*it)->unlock();
                }
                if (subject == NULL) {
                    std::cout << "[AH] couldn't find rid:" << robotid << "\n";
                    break;
                }


                inUse->lock();
                    
                for(used_it = inUse->begin(); used_it != inUse->end(); ++used_it){
                    if((*used_it) == subject)
                         isUsed = true;
                }


                if(!isUsed)
                    inUse->push_back(subject);
                inUse->unlock();

                cmd.RID = subject->getRID();
                robotControl->sendCommand(&cmd, subject->getEndpoint());
                (*it)->unlock();
            }
            break;

            case P_CMD_LFT:
            {
                cmd.cmd = P_CMD_LFT;
                
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int robotid = atoi(token.c_str());

                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator used_it;

                for(it = robots->begin(); it != robots->end(); ++it){
                    (*it)->lock();
                    if((*it)->getGlobalID() == robotid){
                        subject = (*it);
                        break;
                    }
                    (*it)->unlock();
                }
                if (subject == NULL) {
                    std::cout << "[AH] couldn't find rid:" << robotid << "\n";
                    break;
                }


                inUse->lock();
                    
                for(used_it = inUse->begin(); used_it != inUse->end(); ++used_it){
                    if((*used_it) == subject)
                         isUsed = true;
                }


                if(!isUsed)
                    inUse->push_back(subject);
                inUse->unlock();
                
                cmd.RID = subject->getRID();
                robotControl->sendCommand(&cmd, subject->getEndpoint());
                (*it)->unlock();
            }
            break;

            case P_CMD_RGHT:
            {
                cmd.cmd = P_CMD_RGHT;
                
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int robotid = atoi(token.c_str());

                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator used_it;


                for(it = robots->begin(); it != robots->end(); ++it){
                    (*it)->lock();
                    if((*it)->getGlobalID() == robotid){
                        subject = (*it);
                        break;
                    }
                    (*it)->unlock();
                }
                if (subject == NULL) {
                    std::cout << "[AH] couldn't find rid:" << robotid << "\n";
                    break;
                }


                inUse->lock();
                    
                for(used_it = inUse->begin(); used_it != inUse->end(); ++used_it){
                    if((*used_it) == subject)
                         isUsed = true;
                }


                if(!isUsed)
                    inUse->push_back(subject);
                inUse->unlock();
                
                cmd.RID = subject->getRID();
                robotControl->sendCommand(&cmd, subject->getEndpoint());
                (*it)->unlock();
            }
            break;

            case P_CMD_WEST:

            break;
            case P_CMD_MVTO:
            {   
                cmd.cmd = P_CMD_MVTO;
                token = s.substr(0, s.find('$', 0));
                if(token.size() == s.size()){
                    break;
                }

                s = s.substr(token.size() +1, std::string::npos);
                int robotid = atoi(token.c_str());

                
                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator used_it;

                for(it = robots->begin(); it != robots->end(); ++it){
                    (*it)->lock();
                    if((*it)->getGlobalID() == robotid){
                        subject = (*it);
                        break;
                    }
                    (*it)->unlock();
                }
                if (subject == NULL) {
                    std::cout << "[AH] couldn't find rid:" << robotid << "\n";
                    break;
                }

                inUse->lock();
                    
                for(used_it = inUse->begin(); used_it != inUse->end(); ++used_it){
                    if((*used_it) == subject)
                         isUsed = true;
                }


                if(!isUsed)
                    inUse->push_back(subject);
                inUse->unlock();


                cmd.RID = subject->getRID();

 
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);

                int arg1, arg2;

                arg1 = atoi(token.c_str());

                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);

                arg2 = atoi(token.c_str());

                cmd.arg = P_COORD(arg1, arg2);


                robotControl->sendCommand(&cmd, subject->getEndpoint());
                (*it)->unlock();
            }
            break;
            case P_CMD_CAMROT:
            {
                cmd.cmd = P_CMD_CAMROT;
                
                token = s.substr(0, s.find('$', 0));
                if (token.size() == s.size()) {
                    //std::cout << "[AH] token:\"" << token << "\" *is* s\n";
                    break;
                }
                s = s.substr(token.size() + 1, std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int robotid = atoi(token.c_str());
                //std::cout << "[AH] camrot: got RID: " << robotid << "\n";

                Vector_ts<Robot*>::iterator it;
                Vector_ts<Robot*>::iterator used_it;

                for(it = robots->begin(); it != robots->end(); ++it){
                    (*it)->lock();
                    if((*it)->getGlobalID() == robotid){
                        subject = (*it);
                        break;
                    }
                    (*it)->unlock();
                }
                if (subject == NULL) {
                    std::cout << "[AH] couldn't find rid:" << robotid << "\n";
                    break;
                }
                

                inUse->lock();
                    
                for(used_it = inUse->begin(); used_it != inUse->end(); ++used_it){
                    if((*used_it) == subject)
                         isUsed = true;
                }


                if(!isUsed)
                    inUse->push_back(subject);
                inUse->unlock();


                cmd.RID = subject->getRID();

                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                cmd.arg = atoi(token.c_str());


                robotControl->sendCommand(&cmd, subject->getEndpoint());
                (*it)->unlock();
            }
            break;
            case P_CMD_RLS_RBT:
            {
            
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int robotid = atoi(token.c_str());

                std::cout << "[AH] Marking robot " << robotid << " as not in use\n";
                Vector_ts<Robot*>::iterator it;

                inUse->lock();
                for(it = inUse->begin(); it != inUse->end(); ++it){
                    if((*it)->getGlobalID() == robotid){    
                        inUse->erase(it);
                        break;
                    }
                }
                inUse->unlock();

            }
            break;

            case P_CMD_DEL_OBJ:
            {
                token = s.substr(0, s.find('$', 0));
                s = s.substr(token.size(), std::string::npos);
                //std::cout << "[AH] token:\"" << token << "\" s:\"" << s << "\"\n";

                int objID = atoi(token.c_str());
                std::cout << "[AH] deleting object " << objID << "\n";

                Vector_ts<Object*>::iterator it;

                objects->lock();

                for(it = objects->begin(); it != objects->end(); ++it){
                    if((*it)->getOID() == objID){
                        objects->erase(it);
                        break;
                    }
                }

                objects->unlock();

                // also let the robot controllers know to delete it
                cmd.cmd = P_CMD_DEL_OBJ;
                cmd.RID = -1;
                cmd.arg = objID;
                robotControl->sendCommand(&cmd);

            }
            break;

            case P_CMD_SHUTDOWN:
            {
                cmd.cmd = P_CMD_SHUTDOWN;
                cmd.RID = -1;
                cmd.arg = 0;
                robotControl->sendCommand(&cmd);

            }
            break;
            
            default:
            break;
        }

        robots->readUnlock();
    }
    
    boost::asio::async_read_until(sock, read_message_.buffer(), '\n', 
        boost::bind(&AdminHandler::session::read_handler, this, 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    
    conn_->releaseSocket();

}

void AdminHandler::session::close() {
    if (closing || !conn_)
        return;

    closing = true;
    try {
        tcp::socket &sock = conn_->socket();;

        sock.get_io_service().post(boost::bind(&AdminHandler::session::do_close, this));
    } catch (...) {
        std::cerr << "[AH] error occurred in close()\n";
    }
    conn_->releaseSocket();
}

void AdminHandler::session::do_close() {
    try {
        tcp::socket &sock = conn_->socket();;

        std::cout << "[AH] Closing socket to " << sock.remote_endpoint().address().to_string() << ":" << sock.remote_endpoint().port() << "\n";
        conn_->stop();
    } catch (...) {
        std::cerr << "[AH] error occurred in do_close()\n";
    }

    conn_->releaseSocket();
}

void AdminHandler::shutdown() {
    std::vector<boost::shared_ptr<session> >::iterator it;
    for (it = sessions.begin(); it < sessions.end(); ++it) {
        (*it)->close();
    }
}

/* vi: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */
