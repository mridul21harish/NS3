#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-phy-state.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThroughputExperiment");

const int PHY_STATES = 7;
double simTime = 15.0;
double logStartTime = 5.0;
double logStopTime = 10.0;
const std::string CSV_DELIMITER = ",";

void ApStateLogger(Time start, Time duration, WifiPhyState state)
{
	/*static bool firstCall = true;
    	if (firstCall)
    	{
        	std::cout << "First call to ApStateLogger at simulation time: " << Simulator::Now().GetSeconds() << "s" << std::endl;
        	std::cout << "Start time: " << start.GetSeconds() << "s, Duration: " << duration.GetSeconds() << "s, State: " << state << std::endl;
        	firstCall = false;

        	Simulator::Stop();
        	return;
    	}*/

	double currentTime = Simulator::Now().GetSeconds();
    	if (currentTime < logStartTime || currentTime > logStopTime) return;

	std::string stateName;
   	switch(state)
    	{
        	case WifiPhyState::IDLE: stateName = "IDLE"; break;
        	case WifiPhyState::CCA_BUSY: stateName = "CCA_BUSY"; break;
        	case WifiPhyState::TX: stateName = "TX"; break;
        	case WifiPhyState::RX: stateName = "RX"; break;
        	case WifiPhyState::SWITCHING: stateName = "SWITCHING"; break;
        	case WifiPhyState::SLEEP: stateName = "SLEEP"; break;
		case WifiPhyState::OFF: stateName = "OFF"; break;
        	default: stateName = "UNKNOWN"; break;
    	}

	std::cout << "At " << start.GetSeconds() << "s: AP State change to " << stateName 
                  << " (duration " << duration.GetSeconds() << "s)" << std::endl;

	std::ofstream csvFile("ApStateLog.csv", std::ios::app);
    	if(csvFile.is_open())
    	{
        	csvFile << start.GetSeconds() << CSV_DELIMITER 
                   	   << duration.GetSeconds() << CSV_DELIMITER 
                   	   << stateName << std::endl;
        	csvFile.close();
    	}
    	else
    	{
        	std::cerr << "Unable to open file ApStateLog.csv" << std::endl;
    	}

    	std::ofstream logFile("ApStateLog.txt", std::ios::app);
    	if(logFile.is_open())
    	{
        	logFile << "At " << start.GetSeconds() << "s: AP State change to " << stateName 
                           << " (duration " << duration.GetSeconds() << "s)" << std::endl;
        	logFile.close();
    	}
    	else
    	{
        	std::cerr << "Unable to open file ApStateLog.txt" << std::endl;
    	}
}

void ClientStateLogger(Time start, Time duration, WifiPhyState state)
{
	/*static bool firstCall = true;
        if (firstCall)
        {
                std::cout << "First call to ClientStateLogger at simulation time: " << Simulator::Now().GetSeconds() << "s" << std::endl;
                std::cout << "Start time: " << start.GetSeconds() << "s, Duration: " << duration.GetSeconds() << "s, State: " << state << std::endl;
                firstCall = false;

                Simulator::Stop();
                return;
        }*/

	double currentTime = Simulator::Now().GetSeconds();
    	if (currentTime < logStartTime || currentTime > logStopTime) return;

    	std::string stateName;
    	switch(state)
    	{
        	case WifiPhyState::IDLE: stateName = "IDLE"; break;
        	case WifiPhyState::CCA_BUSY: stateName = "CCA_BUSY"; break;
        	case WifiPhyState::TX: stateName = "TX"; break;
        	case WifiPhyState::RX: stateName = "RX"; break;
        	case WifiPhyState::SWITCHING: stateName = "SWITCHING"; break;
        	case WifiPhyState::SLEEP: stateName = "SLEEP"; break;
		case WifiPhyState::OFF: stateName = "OFF"; break;
        	default: stateName = "UNKNOWN"; break;
    	}

    	std::cout << "At " << start.GetSeconds() << "s: Client State change to " << stateName 
                  << " (duration " << duration.GetSeconds() << "s)" << std::endl;

	std::ofstream csvFile("ClientStateLog.csv", std::ios::app);
        if(csvFile.is_open())
        {
                csvFile << start.GetSeconds() << CSV_DELIMITER 
                           << duration.GetSeconds() << CSV_DELIMITER 
                           << stateName << std::endl;
                csvFile.close();
        }
        else
        {
                std::cerr << "Unable to open file ClientStateLog.csv" << std::endl;
        }


	std::ofstream logFile("ClientStateLog.txt", std::ios::app);
    	if(logFile.is_open())
    	{
        	logFile << "At " << start.GetSeconds() << "s: Client State change to " << stateName 
                   	   << " (duration " << duration.GetSeconds() << "s)" << std::endl;
        	logFile.close();
    	}
    	else
    	{
        	std::cerr << "Unable to open file ClientStateLog.txt" << std::endl;
    	}
}

int main(int argc, char *argv[])
{
	std::string logFiles[] = {"ApStateLog.txt", "ClientStateLog.txt", "ApStateLog.csv", "ClientStateLog.csv"};
    	for (const auto& logFile : logFiles)
	{
        	if(access(logFile.c_str(), F_OK) == 0)
		{
            		std::string command = "rm -f " + logFile;
            		system(command.c_str());
        	}
    	}

	bool pcapTracing = false;

	uint32_t packetSize = 1400;
	double power = 20;
	double gi = 800;

        //LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
  	//LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);

  	NodeContainer wifiNodes;
  	wifiNodes.Create(2);
  	NodeContainer apNode = wifiNodes.Get(0);
  	NodeContainer clientNode = wifiNodes.Get(1);

	NodeContainer serverNode;
	serverNode.Create(1);

  	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  	YansWifiPhyHelper phy;
  	phy.SetChannel(channel.Create());

	WifiHelper wifi;
        wifi.SetStandard(WIFI_STANDARD_80211ax);
        wifi.ConfigHeOptions("GuardInterval", TimeValue(NanoSeconds(gi)));

	//Setting the Channel Settings
	Config::SetDefault("ns3::WifiPhy::ChannelSettings", StringValue("{36, 20, BAND_5GHZ, 0}"));

	//Setting the Power value
	Config::SetDefault("ns3::WifiPhy::TxPowerStart", DoubleValue(power));
	Config::SetDefault("ns3::WifiPhy::TxPowerEnd", DoubleValue(power));

	//Enable RTS/CTS
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
	Config::SetDefault("ns3::WifiDefaultProtectionManager::EnableMuRts", BooleanValue("true"));

	//Enable Aggregation
	Config::SetDefault("ns3::WifiMac::BE_MaxAmpduSize", UintegerValue(65535));

	//Set spatial streams
	Config::SetDefault("ns3::WifiPhy::Antennas", UintegerValue(1));
	Config::SetDefault("ns3::WifiPhy::MaxSupportedTxSpatialStreams", UintegerValue(1));
	Config::SetDefault("ns3::WifiPhy::MaxSupportedRxSpatialStreams", UintegerValue(1));

	WifiMacHelper mac;
  	Ssid ssid = Ssid("ns3-wifi-he");
  	mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  	NetDeviceContainer apDevice = wifi.Install(phy, mac, apNode);

  	mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  	NetDeviceContainer clientDevice = wifi.Install(phy, mac, clientNode);

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("1us"));

	NetDeviceContainer p2pDevices = pointToPoint.Install(serverNode.Get(0), apNode.Get(0));

  	MobilityHelper mobility;
  	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  	mobility.Install(wifiNodes);

  	InternetStackHelper stack;
  	stack.Install(wifiNodes);
	stack.Install(serverNode);

  	Ipv4AddressHelper address;

	address.SetBase("10.1.1.0", "255.255.255.0");
  	Ipv4InterfaceContainer apInterface = address.Assign(apDevice);
  	Ipv4InterfaceContainer clientInterface = address.Assign(clientDevice);

	//std::cout << "AP IP Address: " << apInterface.GetAddress(0) << std::endl;
	//std::cout << "Client IP Address: " << clientInterface.GetAddress(0) << std::endl;

	address.SetBase("10.1.2.0", "255.255.255.0");
	address.Assign(p2pDevices);

  	UdpServerHelper udpServer(9);
  	ApplicationContainer serverApp = udpServer.Install(clientNode);
  	serverApp.Start(Seconds(1.0));
  	serverApp.Stop(Seconds(simTime));

	OnOffHelper onoff ("ns3::UdpSocketFactory", Address(InetSocketAddress (clientInterface.GetAddress(0), 9)));
        onoff.SetAttribute("DataRate", StringValue("200Mbps"));
	onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
	ApplicationContainer app = onoff.Install(apNode);
        app.Start(Seconds (1.0));
        app.Stop(Seconds (simTime));

	if(pcapTracing)
	{
		phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
		phy.EnablePcap("Ap", apDevice.Get(0));
		phy.EnablePcap("Client", clientDevice.Get(0));
    	}

	//Make the connection of the trace sink with the trace source
	Config::ConnectWithoutContext("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback(&ApStateLogger));
	Config::ConnectWithoutContext("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback(&ClientStateLogger));

    	Simulator::Stop(Seconds(simTime));
  	Simulator::Run();

	uint64_t rxBytes = 0;
        for (uint32_t i = 0; i < serverApp.GetN(); i++)
        {
        	rxBytes += packetSize * DynamicCast<UdpServer>(serverApp.Get(i))->GetReceived();
        }

	double throughput = (rxBytes * 8)/(simTime * 1000000.0); // Convert to Mbps
        std::cout << "\nThroughput Achieved = " << throughput << "Mbps" << std::endl;

  	Simulator::Destroy();

  	return 0;
}
