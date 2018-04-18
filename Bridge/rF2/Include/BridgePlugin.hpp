#include "InternalsPlugin.hpp"
#include <string>						// std::string
#include <sstream>						// std::ostringstream

// This is used for the app to use the plugin for its intended purpose
class BridgePlugin : public InternalsPluginV07
{

public:

	// Constructor/destructor
	BridgePlugin() {}
	~BridgePlugin() {}

	// These are the functions derived from base class InternalsPlugin
	// that can be implemented.
	void Startup(long version);  // game startup
	void Shutdown();               // game shutdown

	void EnterRealtime();          // entering realtime
	void ExitRealtime();           // exiting realtime

	// GAME OUTPUT
	long WantsTelemetryUpdates() { return(1); } // CHANGE TO 1 TO ENABLE TELEMETRY EXAMPLE!
	void UpdateTelemetry(const TelemInfoV01 &info);

	// SCORING OUTPUT
	bool WantsScoringUpdates() { return(true); } // CHANGE TO TRUE TO ENABLE SCORING EXAMPLE!
	void UpdateScoring(const ScoringInfoV01 &info);

private:
	float mET;  // event time
	bool mEnabled; // needed for the hardware example

	// data variables
	static const char type_telemetry = 1;
	static const char type_scoring = 2;

	void Send(const std::ostringstream &stream);
	void Log(const char *msg);

	SOCKET s; // socket to send data to
	struct sockaddr_in sad;
	char hostname[256];
	int port;

	// socket variables
	//int senderSocket; // socket to data
	//const char *serverHost;
	//u_short serverPort;
};