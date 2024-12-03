#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-phy-state.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwinThroughputExperiment");

const int PHY_STATES = 7;
double simTime = 15.0;
double logStartTime = 5.0;
double logStopTime = 10.0;
const std::string CSV_DELIMITER = ",";

void Ap1StateLogger(Time start, Time duration, WifiPhyState state)
{
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

        std::cout << "At " << start.GetSeconds() << "s: AP1 State change to " << stateName
                  << " (duration " << duration.GetSeconds() << "s)" << std::endl;

        std::ofstream csvFile("Ap1StateLog.csv", std::ios::app);
        if(csvFile.is_open())
        {
                csvFile << start.GetSeconds() << CSV_DELIMITER
                           << duration.GetSeconds() << CSV_DELIMITER
                           << stateName << std::endl;
                csvFile.close();
        }
        else
        {
                std::cerr << "Unable to open file Ap1StateLog.csv" << std::endl;
        }

	std::ofstream logFile("Ap1StateLog.txt", std::ios::app);
        if(logFile.is_open())
        {
                logFile << "At " << start.GetSeconds() << "s: AP State change to " << stateName
                           << " (duration " << duration.GetSeconds() << "s)" << std::endl;
                logFile.close();
        }
        else
        {
                std::cerr << "Unable to open file Ap1StateLog.txt" << std::endl;
        }
}

void Ap2StateLogger(Time start, Time duration, WifiPhyState state)
{
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

        std::cout << "At " << start.GetSeconds() << "s: AP2 State change to " << stateName
                  << " (duration " << duration.GetSeconds() << "s)" << std::endl;

        std::ofstream csvFile("Ap2StateLog.csv", std::ios::app);
        if(csvFile.is_open())
        {
                csvFile << start.GetSeconds() << CSV_DELIMITER
                           << duration.GetSeconds() << CSV_DELIMITER
                           << stateName << std::endl;
                csvFile.close();
        }
        else
        {
                std::cerr << "Unable to open file Ap2StateLog.csv" << std::endl;
        }

        std::ofstream logFile("Ap2StateLog.txt", std::ios::app);
        if(logFile.is_open())
        {
                logFile << "At " << start.GetSeconds() << "s: AP State change to " << stateName
                           << " (duration " << duration.GetSeconds() << "s)" << std::endl;
                logFile.close();
        }
        else
        {
                std::cerr << "Unable to open file Ap2StateLog.txt" << std::endl;
        }
}

void AssocTrace(Mac48Address addr)
{
    	std::cout << "Associated with AP: " << addr << std::endl;
}

int main(int argc, char *argv[])
{
	std::string logFiles[] = {"Ap1StateLog.txt", "Ap2StateLog.txt", "Ap1StateLog.csv", "Ap2StateLog.csv"};
        for (const auto& logFile : logFiles)
        {
                if(access(logFile.c_str(), F_OK) == 0)
                {
                        std::string command = "rm -f " + logFile;
                        system(command.c_str());
                }
        }

        bool pcapTracing = true;

        uint32_t packetSize = 1400;
	double power = 20;
	double gi = 800;

        //LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
        //LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);

	NodeContainer wifiApNodes;
	wifiApNodes.Create(2);

	NodeContainer wifiClientNodes;
	wifiClientNodes.Create(2);

	NodeContainer serverNodes;
	serverNodes.Create(2);

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
        NetDeviceContainer apDevices;
        apDevices.Add(wifi.Install(phy, mac, wifiApNodes.Get(0)));
	apDevices.Add(wifi.Install(phy, mac, wifiApNodes.Get(1)));

        mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
        NetDeviceContainer clientDevices;
        clientDevices.Add(wifi.Install(phy, mac, wifiClientNodes.Get(0)));
	clientDevices.Add(wifi.Install(phy, mac, wifiClientNodes.Get(1)));

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("1us"));

	NetDeviceContainer p2pDevices;
	p2pDevices.Add(pointToPoint.Install(serverNodes.Get(0), wifiApNodes.Get(0)));
	p2pDevices.Add(pointToPoint.Install(serverNodes.Get(1), wifiApNodes.Get(1)));

	MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        positionAlloc->Add(Vector(0.0, 0.0, 0.0)); //Position for AP1
        positionAlloc->Add(Vector(5.0, 0.0, 0.0)); //Position for Client1
	positionAlloc->Add(Vector(0.0, 5.0, 0.0)); //Position for AP2
	positionAlloc->Add(Vector(5.0, 5.0, 0.0)); //Position for Client2
	positionAlloc->Add(Vector(-5.0, 0.0, 0.0)); //Positon for Server1
	positionAlloc->Add(Vector(-5.0, 5.0, 0.0)); //Position for Server2
        mobility.SetPositionAllocator(positionAlloc);
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

        mobility.Install(wifiApNodes.Get(0));
        mobility.Install(wifiClientNodes.Get(0));
	mobility.Install(wifiApNodes.Get(1));
	mobility.Install(wifiClientNodes.Get(1));
	mobility.Install(serverNodes.Get(0));
	mobility.Install(serverNodes.Get(1));

	/*Ptr<MobilityModel> apMobility1 = wifiApNodes.Get(0)->GetObject<MobilityModel>();
        Ptr<MobilityModel> clientMobility1 = wifiClientNodes.Get(0)->GetObject<MobilityModel>();
	Ptr<MobilityModel> apMobility2 = wifiApNodes.Get(1)->GetObject<MobilityModel>();
	Ptr<MobilityModel> clientMobility2 = wifiClientNodes.Get(1)->GetObject<MobilityModel>();
	Ptr<MobilityModel> serverMobility1 = serverNodes.Get(0)->GetObject<MobilityModel>();
	Ptr<MobilityModel> serverMobility2 = serverNodes.Get(1)->GetObject<MobilityModel>();
        Vector apPosition1 = apMobility1->GetPosition();
        Vector clientPosition1 = clientMobility1->GetPosition();
	Vector apPosition2 = apMobility2->GetPosition();
	Vector clientPosition2 = clientMobility2->GetPosition();
	Vector serverPosition1 = serverMobility1->GetPosition();
	Vector serverPosition2 = serverMobility2->GetPosition();

        std::cout << "AP1 Position: (" << apPosition1.x << ", " << apPosition1.y << ")" << std::endl;
        std::cout << "Client1 Position: (" << clientPosition1.x << ", " << clientPosition1.y << ")" << std::endl;
	std::cout << "AP2 Position: (" << apPosition2.x << ", " << apPosition2.y << ")" <<std::endl;
	std::cout << "Client2 Position: (" << clientPosition2.x << ", " << clientPosition2.y << ")" << std::endl;
	std::cout << "Server1 Position: (" << serverPosition1.x << ", " << serverPosition1.y << ")" << std::endl;
	std::cout << "Server2 Position: (" << serverPosition2.x << ", " << serverPosition2.y << ")" << std::endl;*/

        InternetStackHelper stack;
        stack.Install(wifiApNodes);
        stack.Install(wifiClientNodes);
	stack.Install(serverNodes);

	/*Ptr<WifiNetDevice> apNetDevice1 = DynamicCast<WifiNetDevice>(apDevices.Get(0));
    	Ptr<WifiNetDevice> apNetDevice2 = DynamicCast<WifiNetDevice>(apDevices.Get(1));
    	std::cout << "AP1 MAC Address: " << apNetDevice1->GetMac()->GetAddress() << std::endl;
    	std::cout << "AP2 MAC Address: " << apNetDevice2->GetMac()->GetAddress() << std::endl;
	Ptr<WifiNetDevice> clientNetDevice1 = DynamicCast<WifiNetDevice>(clientDevices.Get(0));
   	Ptr<WifiNetDevice> clientNetDevice2 = DynamicCast<WifiNetDevice>(clientDevices.Get(1));
   	std::cout << "Client1 MAC Address: " << clientNetDevice1->GetMac()->GetAddress() << std::endl;
    	std::cout << "Client2 MAC Address: " << clientNetDevice2->GetMac()->GetAddress() << std::endl;*/

        Ipv4AddressHelper address;
	address.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer apInterface1 = address.Assign(apDevices.Get(0));
        Ipv4InterfaceContainer clientInterface1 = address.Assign(clientDevices.Get(0));
	Ipv4InterfaceContainer apInterface2 = address.Assign(apDevices.Get(1));
	Ipv4InterfaceContainer clientInterface2 = address.Assign(clientDevices.Get(1));

	address.SetBase("10.1.2.0", "255.255.255.0");
	address.Assign(p2pDevices);

        /*std::cout << "AP1 IP Address: " << apInterface1.GetAddress(0) << std::endl;
        std::cout << "Client1 IP Address: " << clientInterface1.GetAddress(0) << std::endl;
	std::cout << "AP2 IP Address: " << apInterface2.GetAddress(0) << std::endl;
	std::cout << "Client2 IP Address: " << clientInterface2.GetAddress(0) << std::endl;*/

	UdpServerHelper udpServer1(9);
        ApplicationContainer serverApp1 = udpServer1.Install(wifiClientNodes.Get(0));
        serverApp1.Start(Seconds(1.0));
        serverApp1.Stop(Seconds(simTime));

        OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address(InetSocketAddress (clientInterface1.GetAddress(0), 9)));
        onoff1.SetAttribute("DataRate", StringValue("200Mbps"));
        onoff1.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        ApplicationContainer app1 = onoff1.Install(wifiApNodes.Get(0));
        app1.Start(Seconds (1.0));
        app1.Stop(Seconds (simTime));

	UdpServerHelper udpServer2(10);
        ApplicationContainer serverApp2 = udpServer2.Install(wifiClientNodes.Get(1));
        serverApp2.Start(Seconds(1.0));
        serverApp2.Stop(Seconds(simTime));

        OnOffHelper onoff2 ("ns3::UdpSocketFactory", Address(InetSocketAddress (clientInterface2.GetAddress(0), 10)));
        onoff2.SetAttribute("DataRate", StringValue("200Mbps"));
        onoff2.SetAttribute("PacketSize", UintegerValue(packetSize));
        onoff2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        ApplicationContainer app2 = onoff2.Install(wifiApNodes.Get(1));
        app2.Start(Seconds (1.0));
        app2.Stop(Seconds (simTime));

	//Config::ConnectWithoutContext("/NodeList/3/DeviceList/0/$ns3::WifiNetDevice/Mac/Assoc", MakeCallback(&AssocTrace));

	Config::ConnectWithoutContext("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback(&Ap1StateLogger));
        Config::ConnectWithoutContext("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback(&Ap2StateLogger));

        if(pcapTracing)
        {
                phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
                phy.EnablePcap("Ap1", apDevices.Get(0));
                phy.EnablePcap("Client1", clientDevices.Get(0));
		phy.EnablePcap("Ap2", apDevices.Get(1));
                phy.EnablePcap("Client2", clientDevices.Get(1));
        }

        Simulator::Stop(Seconds(simTime));
        Simulator::Run();

        uint64_t rxBytes1 = 0;
        for (uint32_t i = 0; i < serverApp1.GetN(); i++)
        {
                rxBytes1 += packetSize * DynamicCast<UdpServer>(serverApp1.Get(i))->GetReceived();
                std::cout << "\nReceived packets from AP1 = " << rxBytes1 << std::endl;
        }

        double throughput_1 = (rxBytes1 * 8)/(simTime * 1000000.0); // Convert to Mbps
        std::cout << "Throughput_1 Achieved = " << throughput_1 << "Mbps" << std::endl;

	uint64_t rxBytes2 = 0;
        for (uint32_t i = 0; i < serverApp2.GetN(); i++)
        {
                rxBytes2 += packetSize * DynamicCast<UdpServer>(serverApp2.Get(i))->GetReceived();
                std::cout << "\nReceived packets from AP2 = " << rxBytes2 << std::endl;
        }

        double throughput_2 = (rxBytes2 * 8)/(simTime * 1000000.0); // Convert to Mbps
        std::cout << "Throughput_2 Achieved = " << throughput_2 << "Mbps" << std::endl;

	Simulator::Destroy();

        return 0;
}
