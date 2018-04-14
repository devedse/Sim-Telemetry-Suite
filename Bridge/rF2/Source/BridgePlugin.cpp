#include <WinSock2.h>					// socket WinSock2.h must be included before <windows.h>
#include "BridgePlugin.hpp"				// corresponding header file
#include <math.h>						// for atan2, sqrt
#include <stdio.h>						// for sample output
#include <limits>
#include <string>						// std::string
#include <sstream>						// std::ostringstream
// socket
#include <ws2tcpip.h>
// thread
#include <process.h>
#pragma comment(lib, "Ws2_32.lib")
#include <time.h>


// plugin information

extern "C" __declspec(dllexport)
const char * __cdecl GetPluginName() { return("Bridge.rF2"); }

extern "C" __declspec(dllexport)
PluginObjectType __cdecl GetPluginType() { return(PO_INTERNALS); }

extern "C" __declspec(dllexport)
int __cdecl GetPluginVersion() { return(7); }

extern "C" __declspec(dllexport)
PluginObject * __cdecl CreatePluginObject() { return((PluginObject *) new BridgePlugin); }

extern "C" __declspec(dllexport)
void __cdecl DestroyPluginObject(PluginObject *obj) { delete((BridgePlugin *)obj); }


// BridgePlugin class


void BridgePlugin::Startup(long version)
{
	FILE *settings;
	char portstring[10];

	ADDRINFO hints = { sizeof(addrinfo) };
	hints.ai_flags = AI_ALL;
	hints.ai_family = PF_INET;
	hints.ai_protocol = IPPROTO_IPV4;
	ADDRINFO *pResult = NULL;

	Log("starting plugin");

	// open socket
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		Log("could not create datagram socket");
		return;
	}

	int err = fopen_s(&settings, "Bridge.rF2.ini", "r");
	if (err == 0) {
		Log("reading settings");
		if (fscanf_s(settings, "%[^:]:%i", hostname, _countof(hostname), &port) != 2) {
			Log("could not read host and port");
		}

		Log("settings read from file");
		int errcode = getaddrinfo(hostname, NULL, &hints, &pResult);

		fclose(settings);

		Log("hostname is:");
		Log(hostname);
		Log("port is:");
		sprintf_s(portstring, "%i", port);
		Log(portstring);
	}
	else {
		Log("could not read settings, using defaults: localhost:666");
		int errcode = getaddrinfo("localhost", NULL, &hints, &pResult);
		port = 666;
	}

	memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET;           /* set family to Internet     */
	sad.sin_port = htons((u_short)port);
	sad.sin_addr.S_un.S_addr = *((ULONG*)&(((sockaddr_in*)pResult->ai_addr)->sin_addr));
}

void BridgePlugin::Shutdown()
{
	if (s > 0) {
		closesocket(s);
		s = 0;
	}
}

void BridgePlugin::EnterRealtime()
{
	mET = 0.0f;
}

void BridgePlugin::ExitRealtime()
{
	mET = -1.0f;
}

void BridgePlugin::UpdateScoring(const ScoringInfoV01 &info)
{
	std::ostringstream scoringStream;
	scoringStream
		// Opening json tag
		<< "{"

		// Set the message info
		<< "\"application\":\"rfactor2\""
		<< ",\"type\":\"scoring\""

		// General scoring info
		<< ",\"trackName\":\"%s\"" << info.mTrackName
		<< ",\"session\":%d" << info.mSession
		<< ",\"numVehicles\":%d" << info.mNumVehicles
		<< ",\"currentET\":%.3f" << info.mCurrentET
		<< ",\"endET\":%.3f" << info.mEndET
		<< ",\"maxLaps\":%d" << info.mMaxLaps
		<< ",\"lapDist\":%.1f" << info.mLapDist

		// Session info
		<< ",\"gamePhase\":%d" << info.mGamePhase
		<< ",\"yellowFlagState\":%d" << info.mYellowFlagState
		<< ",\"sectorFlags\":[%d,%d,%d]" << info.mSectorFlag[0] << info.mSectorFlag[1] << info.mSectorFlag[2]
		<< ",\"inRealTime\":%d" << info.mInRealtime
		<< ",\"startLight\":%d" << info.mStartLight
		<< ",\"numRedLights\":%d" << info.mNumRedLights
		<< ",\"playerName\":\"%s\"" << info.mPlayerName
		<< ",\"plrFileName\":\"%s\"" << info.mPlrFileName
		<< ",\"darkCloud\":%.2f" << info.mDarkCloud
		<< ",\"raining\":%.2f" << info.mRaining
		<< ",\"ambientTemp\":%.1f" << info.mAmbientTemp
		<< ",\"trackTemp\":%.1f" << info.mTrackTemp
		<< ",\"wind\":[%.1f,%.1f,%.1f]" << info.mWind.x << info.mWind.y << info.mWind.z
		<< ",\"minPathWetness\":%.1f" << info.mMinPathWetness
		<< ",\"maxPathWetness\":%.1f" << info.mMaxPathWetness

		// Create a vehicle array
		<< ",\"vehicles\":[";

	// Print vehicle info
	for (long i = 0; i < info.mNumVehicles; ++i)
	{
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];

		const double metersPerSec = sqrt((vinfo.mLocalVel.x * vinfo.mLocalVel.x) +
			(vinfo.mLocalVel.y * vinfo.mLocalVel.y) +
			(vinfo.mLocalVel.z * vinfo.mLocalVel.z));

		if (i > 0) {
			scoringStream << ",{";
		}
		else {
			scoringStream << "{";
		}

		// Start writing to scoringStream again
		scoringStream

			<< "\"id\":%d" << vinfo.mID
			<< ",\"driverName\":\"%s\"" << vinfo.mDriverName
			<< ",\"vehicleName\":\"%s\"" << vinfo.mVehicleName
			<< ",\"totalLaps\":%d" << vinfo.mTotalLaps
			<< ",\"sector\":%d" << vinfo.mSector
			<< ",\"finishStatus\":%d" << vinfo.mFinishStatus
			<< ",\"lapDist\":%.1f" << vinfo.mLapDist
			<< ",\"pathLateral\":%.2f" << vinfo.mPathLateral
			<< ",\"relevantTrackEdge\":%.2f" << vinfo.mTrackEdge
			<< ",\"best\":[%.3f << %.3f << %.3f]" << vinfo.mBestSector1 << vinfo.mBestSector2 << vinfo.mBestLapTime
			<< ",\"last\":[%.3f << %.3f << %.3f]" << vinfo.mLastSector1 << vinfo.mLastSector2 << vinfo.mLastLapTime
			<< ",\"currentSector1\":%.3f" << vinfo.mCurSector1
			<< ",\"currentSector2\":%.3f" << vinfo.mCurSector2
			<< ",\"numPitstops\":%d" << vinfo.mNumPitstops
			<< ",\"numPenalties\":%d" << vinfo.mNumPenalties
			<< ",\"isPlayer\":%d" << vinfo.mIsPlayer
			<< ",\"control\":%d" << vinfo.mControl
			<< ",\"inPits\":%d" << vinfo.mInPits
			<< ",\"lapStartET\":%.3f" << vinfo.mLapStartET
			<< ",\"place\":%d" << vinfo.mPlace
			<< ",\"vehicleClass\":\"%s\"" << vinfo.mVehicleClass
			<< ",\"timeBehindNext\":%.3f" << vinfo.mTimeBehindNext
			<< ",\"lapsBehindNext\":%d" << vinfo.mLapsBehindNext
			<< ",\"timeBehindLeader\":%.3f" << vinfo.mTimeBehindLeader
			<< ",\"lapsBehindLeader\":%d" << vinfo.mLapsBehindLeader
			<< ",\"Pos\":[%.3f,%.3f,%.3f]" << vinfo.mPos.x << vinfo.mPos.y << vinfo.mPos.z
			<< ",\"metersPerSecond\":" << &metersPerSec

			// Forward is roughly in the -z direction (although current pitch of car may cause some y-direction velocity)
			<< ",\"localVel\":[%.2f,%.2f,%.2f]" << vinfo.mLocalVel.x << vinfo.mLocalVel.y << vinfo.mLocalVel.z
			<< ",\"localAccel\":[%.1f,%.1f,%.1f]" << vinfo.mLocalAccel.x << vinfo.mLocalAccel.y << vinfo.mLocalAccel.z

			// Orientation matrix is left-handed
			<< ",\"orientationMatrix\":[[%6.3f,%6.3f,%6.3f],[%6.3f,%6.3f,%6.3f],[%6.3f,%6.3f,%6.3f]]" << vinfo.mOri[0].x << vinfo.mOri[0].y << vinfo.mOri[0].z << vinfo.mOri[1].x << vinfo.mOri[1].y << vinfo.mOri[1].z << vinfo.mOri[2].x << vinfo.mOri[2].y << vinfo.mOri[2].z

			<< ",\"localRot\":[%.3f,%.3f,%.3f]" << vinfo.mLocalRot.x << vinfo.mLocalRot.y << vinfo.mLocalRot.z
			<< ",\"localRotAccel\":[%.2f,%.2f,%.2f]" << vinfo.mLocalRotAccel.x << vinfo.mLocalRotAccel.y << vinfo.mLocalRotAccel.z

			<< "}";
	}

	// Start writing to scoringStream again
	scoringStream

		// Close the vehicle array
		<< "]"

		// Closing json character
		<< "}";

	// Send the buffer
	Send(scoringStream);
}

void BridgePlugin::UpdateTelemetry(const TelemInfoV01 &info)
{
	/*
	StreamData((char *)&type_telemetry, sizeof(char));
	StreamData((char *)&info.mGear, sizeof(long));
	StreamData((char *)&info.mEngineRPM, sizeof(double));
	StreamData((char *)&info.mEngineMaxRPM, sizeof(double));
	StreamData((char *)&info.mEngineWaterTemp, sizeof(double));
	StreamData((char *)&info.mEngineOilTemp, sizeof(double));
	StreamData((char *)&info.mClutchRPM, sizeof(double));
	StreamData((char *)&info.mOverheating, sizeof(bool));
	StreamData((char *)&info.mFuel, sizeof(double));

	StreamData((char *)&info.mPos.x, sizeof(double));
	StreamData((char *)&info.mPos.y, sizeof(double));
	StreamData((char *)&info.mPos.z, sizeof(double));

	const double metersPerSec = sqrt((info.mLocalVel.x * info.mLocalVel.x) +
		(info.mLocalVel.y * info.mLocalVel.y) +
		(info.mLocalVel.z * info.mLocalVel.z));
	StreamData((char *)&metersPerSec, sizeof(double));

	StreamData((char *)&info.mLapStartET, sizeof(double));
	StreamData((char *)&info.mLapNumber, sizeof(long));

	StreamData((char *)&info.mUnfilteredThrottle, sizeof(double));
	StreamData((char *)&info.mUnfilteredBrake, sizeof(double));
	StreamData((char *)&info.mUnfilteredSteering, sizeof(double));
	StreamData((char *)&info.mUnfilteredClutch, sizeof(double));

	StreamData((char *)&info.mLastImpactET, sizeof(double));
	StreamData((char *)&info.mLastImpactMagnitude, sizeof(double));
	StreamData((char *)&info.mLastImpactPos.x, sizeof(double));
	StreamData((char *)&info.mLastImpactPos.y, sizeof(double));
	StreamData((char *)&info.mLastImpactPos.z, sizeof(double));
	for (long i = 0; i < 8; i++) {
		StreamData((char *)&info.mDentSeverity[i], sizeof(byte));
	}

	for (long i = 0; i < 4; i++) {
		const TelemWheelV01 &wheel = info.mWheel[i];
		StreamData((char *)&wheel.mDetached, sizeof(bool));
		StreamData((char *)&wheel.mFlat, sizeof(bool));
		StreamData((char *)&wheel.mBrakeTemp, sizeof(double));
		StreamData((char *)&wheel.mPressure, sizeof(double));
		StreamData((char *)&wheel.mRideHeight, sizeof(double));
		StreamData((char *)&wheel.mTemperature[0], sizeof(double));
		StreamData((char *)&wheel.mTemperature[1], sizeof(double));
		StreamData((char *)&wheel.mTemperature[2], sizeof(double));
		StreamData((char *)&wheel.mWear, sizeof(double));
	}
	*/
}

void BridgePlugin::Send(const std::ostringstream &stream)
{
	std::string completedStream = stream.str();
	const char *scoring = const_cast<char*>(completedStream.c_str());
	// const char * var = str.c_str();

	sendto(senderSocket, scoring, sizeof(completedStream), 0, (sockaddr*)&sadSender, sizeof(struct sockaddr));
	// sendto(s, msg, strlen(msg), 0, (struct sockaddr *) &sad, sizeof(struct sockaddr));
}

void BridgePlugin::Log(const char *msg) {
	FILE *logFile;
	time_t curtime;
	struct tm loctime;
	const int timelength = 26;
	char thetime[timelength];

	int err = fopen_s(&logFile, "Bridge.rF2.log", "a");
	if (err == 0) {
		curtime = time(NULL);
		int err2 = localtime_s(&loctime, &curtime);
		int err3 = asctime_s(thetime, timelength, &loctime);
		thetime[timelength - 2] = 0;
		fprintf(logFile, "[%s] %s\n", thetime, msg);
		fclose(logFile);
	}
}