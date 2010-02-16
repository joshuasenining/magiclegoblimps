#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include "Camera.h"
#include "NXT.h"

using namespace std;

class Robot
{
public:
	Robot(int port, string ip, bool dLinkCam);

	int GetID() { return id_; }
	
	Camera* GetCamera() { return camera_; }
	NXT* GetNXT() { return nxt_; }

	void Connect();
	void Disconnect();

	bool GetNXTConnected() { return nxtConnected_; }
	bool GetCamConnected() { return camConnected_; }
	bool GetRobotOnline() { return robotOnline_; }

	void ExecuteCommand(string command);

private:
	int id_;

	Camera* camera_;
	NXT* nxt_;

	bool nxtConnected_;
	bool camConnected_;

	bool robotOnline_;
	bool robotActive_;
};

#endif