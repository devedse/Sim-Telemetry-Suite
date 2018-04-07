#include <WinSock2.h>			// socket WinSock2.h must be included before <windows.h>
#include "BridgePlugin.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <limits>
#include <string>         // std::string
// socket
#include <WS2tcpip.h>
// thread
#include <process.h>
#pragma comment(lib, "Ws2_32.lib")
#include <time.h>

#define TIME_LENGTH 26

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
	data_version = 1;
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
	int err = fopen_s(&settings, "DataPlugin.ini", "r");
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
		Log("could not read settings, using defaults: localhost:6789");
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
	StartStream();
	StreamData((char *)&type_scoring, sizeof(char));

	// session data (changes mostly with changing sessions)
	StreamString((char *)&info.mTrackName, 64);
	StreamData((char *)&info.mSession, sizeof(long));

	// event data (changes continuously)
	StreamData((char *)&info.mCurrentET, sizeof(double));
	StreamData((char *)&info.mEndET, sizeof(double));
	StreamData((char *)&info.mLapDist, sizeof(double));
	StreamData((char *)&info.mNumVehicles, sizeof(long));

	StreamData((char *)&info.mGamePhase, sizeof(byte));
	StreamData((char *)&info.mYellowFlagState, sizeof(byte));
	StreamData((char *)&info.mSectorFlag[0], sizeof(byte));
	StreamData((char *)&info.mSectorFlag[1], sizeof(byte));
	StreamData((char *)&info.mSectorFlag[2], sizeof(byte));
	StreamData((char *)&info.mStartLight, sizeof(byte));
	StreamData((char *)&info.mNumRedLights, sizeof(byte));

	// scoring data (changes with new sector times)
	for (long i = 0; i < info.mNumVehicles; i++) {
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];
		StreamData((char *)&vinfo.mPos.x, sizeof(double));
		StreamData((char *)&vinfo.mPos.z, sizeof(double));
		StreamData((char *)&vinfo.mPlace, sizeof(char));
		StreamData((char *)&vinfo.mLapDist, sizeof(double));
		StreamData((char *)&vinfo.mPathLateral, sizeof(double));
		const double metersPerSec = sqrt((vinfo.mLocalVel.x * vinfo.mLocalVel.x) +
			(vinfo.mLocalVel.y * vinfo.mLocalVel.y) +
			(vinfo.mLocalVel.z * vinfo.mLocalVel.z));
		StreamData((char *)&metersPerSec, sizeof(double));
		StreamString((char *)&vinfo.mVehicleName, 64);
		StreamString((char *)&vinfo.mDriverName, 32);
		StreamString((char *)&vinfo.mVehicleClass, 32);
		StreamData((char *)&vinfo.mTotalLaps, sizeof(short));
		StreamData((char *)&vinfo.mBestSector1, sizeof(double));
		StreamData((char *)&vinfo.mBestSector2, sizeof(double));
		StreamData((char *)&vinfo.mBestLapTime, sizeof(double));
		StreamData((char *)&vinfo.mLastSector1, sizeof(double));
		StreamData((char *)&vinfo.mLastSector2, sizeof(double));
		StreamData((char *)&vinfo.mLastLapTime, sizeof(double));
		StreamData((char *)&vinfo.mCurSector1, sizeof(double));
		StreamData((char *)&vinfo.mCurSector2, sizeof(double));
		StreamData((char *)&vinfo.mTimeBehindLeader, sizeof(double));
		StreamData((char *)&vinfo.mLapsBehindLeader, sizeof(long));
		StreamData((char *)&vinfo.mTimeBehindNext, sizeof(double));
		StreamData((char *)&vinfo.mLapsBehindNext, sizeof(long));
		StreamData((char *)&vinfo.mNumPitstops, sizeof(short));
		StreamData((char *)&vinfo.mNumPenalties, sizeof(short));
		StreamData((char *)&vinfo.mInPits, sizeof(bool));
		StreamData((char *)&vinfo.mSector, sizeof(char));
		StreamData((char *)&vinfo.mFinishStatus, sizeof(char));
	}
	StreamVarString((char *)info.mResultsStream);
	EndStream();
}

void BridgePlugin::UpdateTelemetry(const TelemInfoV01 &info)
{
	StartStream();
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
	EndStream();
}

void BridgePlugin::StartStream() {
	data_packet = 0;
	data_sequence++;
	data[0] = data_version;
	data[1] = data_packet;
	memcpy(&data[2], &data_sequence, sizeof(short));
	data_offset = 4;
}

void BridgePlugin::StreamData(char *data_ptr, int length) {
	int i;

	for (i = 0; i < length; i++) {
		if (data_offset + i == 512) {
			sendto(s, data, 512, 0, (struct sockaddr *) &sad, sizeof(struct sockaddr));
			data_packet++;
			data[0] = data_version;
			data[1] = data_packet;
			memcpy(&data[2], &data_sequence, sizeof(short));
			data_offset = 4;
			length = length - i;
			data_ptr += i;
			i = 0;
		}
		data[data_offset + i] = data_ptr[i];
	}
	data_offset = data_offset + length;
}

void BridgePlugin::StreamVarString(char *data_ptr) {
	int i = 0;
	while (data_ptr[i] != 0) {
		i++;
	}
	StreamData((char *)&i, sizeof(int));
	StreamString(data_ptr, i);
}

void BridgePlugin::StreamString(char *data_ptr, int length) {
	int i;

	for (i = 0; i < length; i++) {
		if (data_offset + i == 512) {
			sendto(s, data, 512, 0, (struct sockaddr *) &sad, sizeof(struct sockaddr));
			data_packet++;
			data[0] = data_version;
			data[1] = data_packet;
			memcpy(&data[2], &data_sequence, sizeof(short));
			data_offset = 4;
			length = length - i;
			data_ptr += i;
			i = 0;
		}
		data[data_offset + i] = data_ptr[i];
		if (data_ptr[i] == 0) {
			// found end of string, so this is where we stop
			data_offset = data_offset + i + 1;
			return;
		}
	}
	data_offset = data_offset + length;
}

void BridgePlugin::EndStream() {
	if (data_offset > 4) {
		sendto(s, data, data_offset, 0, (struct sockaddr *) &sad, sizeof(struct sockaddr));
	}
}

/*
void BridgePlugin::UpdateScoring(const ScoringInfoV01 &info)
{
	// variables
	char buffer[8000];
	ZeroMemory(buffer, sizeof(buffer));		// Fill my block of memory with zeroes

	// Opening json tag
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "{");

	// Set the message info
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "\"application\":\"rfactor2\"");
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"type\":\"scoring\"");

	// General scoring info
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"trackName\":\"%s\"", info.mTrackName);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"session\":%d", info.mSession);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"numVehicles\":%d", info.mNumVehicles);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"currentET\":%.3f", info.mCurrentET);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"endET\":%.3f", info.mEndET);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"maxLaps\":%d", info.mMaxLaps);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"lapDist\":%.1f", info.mLapDist);

	// Session info
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"gamePhase\":%d", info.mGamePhase);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"yellowFlagState\":%d", info.mYellowFlagState);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"sectorFlags\":[%d,%d,%d]", info.mSectorFlag[0], info.mSectorFlag[1], info.mSectorFlag[2]);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"inRealTime\":%d", info.mInRealtime);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"startLight\":%d", info.mStartLight);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"numRedLights\":%d", info.mNumRedLights);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"playerName\":\"%s\"", info.mPlayerName);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"plrFileName\":\"%s\"", info.mPlrFileName);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"darkCloud\":%.2f", info.mDarkCloud);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"raining\":%.2f", info.mRaining);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"ambientTemp\":%.1f", info.mAmbientTemp);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"trackTemp\":%.1f", info.mTrackTemp);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"wind\":[%.1f,%.1f,%.1f]", info.mWind.x, info.mWind.y, info.mWind.z);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"minPathWetness\":%.1f", info.mMinPathWetness);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"maxPathWetness\":%.1f", info.mMaxPathWetness);

	// Create a vehicle array
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"vehicles\":[");

	// Print vehicle info
	for (long i = 0; i < info.mNumVehicles; ++i)
	{
		VehicleScoringInfoV01 &vinfo = info.mVehicle[i];

		if (i > 0) {
			snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",{");
		}
		else {
			snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "{");
		}

		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "\"id\":%d", vinfo.mID);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"driverName\":\"%s\"", vinfo.mDriverName);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"vehicleName\":\"%s\"", vinfo.mVehicleName);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"totalLaps\":%d", vinfo.mTotalLaps);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"sector\":%d", vinfo.mSector);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"finishStatus\":%d", vinfo.mFinishStatus);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"lapDist\":%.1f", vinfo.mLapDist);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"pathLateral\":%.2f", vinfo.mPathLateral);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"relevantTrackEdge\":%.2f", vinfo.mTrackEdge);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"best\":[%.3f, %.3f, %.3f]", vinfo.mBestSector1, vinfo.mBestSector2, vinfo.mBestLapTime);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"last\":[%.3f, %.3f, %.3f]", vinfo.mLastSector1, vinfo.mLastSector2, vinfo.mLastLapTime);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"currentSector1\":%.3f", vinfo.mCurSector1);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"currentSector2\":%.3f", vinfo.mCurSector2);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"numPitstops\":%d", vinfo.mNumPitstops);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"numPenalties\":%d", vinfo.mNumPenalties);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"isPlayer\":%d", vinfo.mIsPlayer);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"control\":%d", vinfo.mControl);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"inPits\":%d", vinfo.mInPits);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"lapStartET\":%.3f", vinfo.mLapStartET);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"place\":%d", vinfo.mPlace);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"vehicleClass\":\"%s\"", vinfo.mVehicleClass);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"timeBehindNext\":%.3f", vinfo.mTimeBehindNext);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"lapsBehindNext\":%d", vinfo.mLapsBehindNext);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"timeBehindLeader\":%.3f", vinfo.mTimeBehindLeader);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"lapsBehindLeader\":%d", vinfo.mLapsBehindLeader);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"Pos\":[%.3f,%.3f,%.3f]", vinfo.mPos.x, vinfo.mPos.y, vinfo.mPos.z);

		// Forward is roughly in the -z direction (although current pitch of car may cause some y-direction velocity)
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"localVel\":[%.2f,%.2f,%.2f]", vinfo.mLocalVel.x, vinfo.mLocalVel.y, vinfo.mLocalVel.z);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"localAccel\":[%.1f,%.1f,%.1f]", vinfo.mLocalAccel.x, vinfo.mLocalAccel.y, vinfo.mLocalAccel.z);

		// Orientation matrix is left-handed
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"orientationMatrix\":[[%6.3f,%6.3f,%6.3f],[%6.3f,%6.3f,%6.3f],[%6.3f,%6.3f,%6.3f]]", vinfo.mOri[0].x, vinfo.mOri[0].y, vinfo.mOri[0].z, vinfo.mOri[1].x, vinfo.mOri[1].y, vinfo.mOri[1].z, vinfo.mOri[2].x, vinfo.mOri[2].y, vinfo.mOri[2].z);

		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"localRot\":[%.3f,%.3f,%.3f]", vinfo.mLocalRot.x, vinfo.mLocalRot.y, vinfo.mLocalRot.z);
		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ",\"localRotAccel\":[%.2f,%.2f,%.2f]", vinfo.mLocalRotAccel.x, vinfo.mLocalRotAccel.y, vinfo.mLocalRotAccel.z);

		snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "}");
	}

	// Close the vehicle array
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "]");

	// Closing json character
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "}");

	// Send the buffer out
	sendto(senderSocket, buffer, sizeof(buffer), 0, (sockaddr*)&sadSender, sizeof(struct sockaddr));
}
*/

void BridgePlugin::Log(const char *msg) {
	FILE *logFile;
	time_t curtime;
	struct tm loctime;
	char thetime[TIME_LENGTH];


	int err = fopen_s(&logFile, "Bridge.rF2.log", "a");
	if (err == 0) {
		curtime = time(NULL);
		int err2 = localtime_s(&loctime, &curtime);
		int err3 = asctime_s(thetime, TIME_LENGTH, &loctime);
		thetime[TIME_LENGTH - 2] = 0;
		fprintf(logFile, "[%s] %s\n", thetime, msg);
		fclose(logFile);
	}
}

void BridgePlugin::Error(const char * const msg)
{
	Log(msg);
}
