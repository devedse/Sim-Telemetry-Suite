#include <WinSock2.h>					// socket WinSock2.h must be included before <windows.h>
#include "BridgePlugin.hpp"				// corresponding header file
#include <math.h>						// for atan2, sqrt
#include <stdio.h>						// for sample output
#include <limits>
#include <string>						// std::string
#include <sstream>						// std::ostringstream
#include <iomanip>
// socket
#include <ws2tcpip.h>
// thread
#include <process.h>
#pragma comment(lib, "Ws2_32.lib")
#include <time.h>

using std::ostringstream;
using std::string;
using std::boolalpha;
using std::noboolalpha;
using std::dec;
using std::hex;
using std::oct;
using std::endl;
using std::scientific;
using std::fixed;
using std::setprecision;
using std::setw;
using std::setfill;

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
		<< ",\"trackName\":\"" << info.mTrackName << "\""
		<< ",\"session\":" << info.mSession
		<< ",\"numVehicles\":" << info.mNumVehicles
		<< ",\"maxLaps\":" << info.mMaxLaps
		<< ",\"currentET\":" << info.mCurrentET
		<< ",\"endET\":" << info.mEndET
		<< ",\"lapDist\":" << info.mLapDist

		// Session info
		//<< ",\"gamePhase\":" << info.mGamePhase
		//<< ",\"yellowFlagState\":" << info.mYellowFlagState
		//<< ",\"sectorFlags\":[" << info.mSectorFlag[0] << "," << info.mSectorFlag[1] << "," << info.mSectorFlag[2] << "]"
		<< ",\"inRealTime\":" << info.mInRealtime
		//<< ",\"startLight\":" << info.mStartLight
		//<< ",\"numRedLights\":" << info.mNumRedLights
		<< ",\"playerName\":\"" << info.mPlayerName << "\""
		<< ",\"plrFileName\":\"" << info.mPlrFileName << "\""
		<< ",\"darkCloud\":" << info.mDarkCloud
		<< ",\"raining\":" << info.mRaining
		<< ",\"ambientTemp\":" << fixed << info.mAmbientTemp
		<< ",\"trackTemp\":" << fixed << info.mTrackTemp
		<< ",\"wind\":[" << info.mWind.x << "," << info.mWind.y << "," << info.mWind.z << "]"
		<< ",\"minPathWetness\":" << info.mMinPathWetness
		<< ",\"maxPathWetness\":" << info.mMaxPathWetness

		// Create a vehicle array
		<< ",\"vehicles\":[";

	// Print vehicle info
	for (long i = 0; i < info.mNumVehicles; ++i)
	{
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];

		double metersPerSec = sqrt((vinfo.mLocalVel.x * vinfo.mLocalVel.x) +
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

			<< "\"id\":" << vinfo.mID
			<< ",\"driverName\":\"" << vinfo.mDriverName << "\""
			<< ",\"vehicleName\":\"" << vinfo.mVehicleName << "\""
			<< ",\"totalLaps\":" << vinfo.mTotalLaps
			//<< ",\"sector\":" << vinfo.mSector
			//<< ",\"finishStatus\":" << vinfo.mFinishStatus
			<< ",\"lapDist\":" << vinfo.mLapDist
			<< ",\"pathLateral\":" << vinfo.mPathLateral
			<< ",\"relevantTrackEdge\":" << vinfo.mTrackEdge
			<< ",\"best\":[" << vinfo.mBestSector1 << "," << vinfo.mBestSector2 << "," << vinfo.mBestLapTime << "]"
			<< ",\"last\":[" << vinfo.mLastSector1 << "," << vinfo.mLastSector2 << "," << vinfo.mLastLapTime << "]"
			<< ",\"currentSector1\":" << vinfo.mCurSector1
			<< ",\"currentSector2\":" << vinfo.mCurSector2
			<< ",\"numPitstops\":" << vinfo.mNumPitstops
			<< ",\"numPenalties\":" << vinfo.mNumPenalties
			<< ",\"isPlayer\":" << vinfo.mIsPlayer
			//<< ",\"control\":" << vinfo.mControl
			<< ",\"inPits\":" << vinfo.mInPits
			<< ",\"lapStartET\":" << vinfo.mLapStartET
			//<< ",\"place\":" << dec << vinfo.mPlace
			<< ",\"vehicleClass\":\"" << vinfo.mVehicleClass << "\""
			<< ",\"timeBehindNext\":" << vinfo.mTimeBehindNext
			<< ",\"lapsBehindNext\":" << vinfo.mLapsBehindNext
			<< ",\"timeBehindLeader\":" << vinfo.mTimeBehindLeader
			<< ",\"lapsBehindLeader\":" << vinfo.mLapsBehindLeader
			<< ",\"Pos\":[" << vinfo.mPos.x << "," << vinfo.mPos.y << "," << vinfo.mPos.z << "]"
			<< ",\"metersPerSecond\":" << metersPerSec

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

	sendto(s, scoring, strlen(scoring), 0, (sockaddr*)&sad, sizeof(struct sockaddr));
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