// Network topology
//
//       n0    n1   n2   n3       
//       |     |    |    |        
//       ================= 
//              LAN
//

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/v4ping-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpClient2Client");

int 
main (int argc, char *argv[]){
    uint16_t numNodes = 2;
    Address serverAddress;

    // --- LOGGING --- //
    LogComponentEnable ("UdpClient2Client", LOG_LEVEL_ALL);

    // --- CRIACAO DOS NODES --- //    
    NS_LOG_INFO ("Create nodes.");

    NodeContainer nodes;
    nodes.Create(numNodes);

    // --- INTERNET --- // 
    NS_LOG_INFO("Install internet protocols stack.");
    InternetStackHelper internet;
    internet.Install (nodes);

    // --- SET E INSTALL CANAL --- //
    NS_LOG_INFO("Set channel attributes.");
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));

    NS_LOG_INFO("Install channel.");
    NetDeviceContainer csmaDevices = csma.Install (nodes);

    // --- SET E INSTALL IP ADRESSES --- //
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface = address.Assign (csmaDevices);

    // --- APLICACOES --- //
    // Coloca um ping e um sink em cada cliente
    NS_LOG_INFO ("Create applications on node 0.");

    //Ping
    V4PingHelper ping (interface.GetAddress(1)); //destino do ping
    ping.SetAttribute ("Verbose", BooleanValue (false));
    ping.SetAttribute ("Interval", TimeValue (Seconds(1.0)));
    ping.SetAttribute ("Size", UintegerValue (16));

    //Instala e starta
    ApplicationContainer pingApp, sinkApp;
    pingApp = ping.Install(nodes.Get(0));
    pingApp.Start(Seconds(1.0));
    pingApp.Stop(Seconds(1.9));

    NS_LOG_INFO ("Create applications on node 1.");

    //Ping
    V4PingHelper ping1 (interface.GetAddress(0)); //destino do ping1
    ping1.SetAttribute ("Verbose", BooleanValue (false));
    ping1.SetAttribute ("Interval", TimeValue (Seconds(1.0)));
    ping1.SetAttribute ("Size", UintegerValue (16));

    //Instala e starta
    ApplicationContainer ping1App, sink1App;
    ping1App = ping1.Install(nodes.Get(1));
    ping1App.Start(Seconds(1.0));
    ping1App.Stop(Seconds(1.9));

    // TRACING
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("udp-client2client.tr"));
    csma.EnablePcapAll ("udp-client2client.tr", false);

    // --- NETANIM --- //
    NS_LOG_INFO("Set animation.");
    AnimationInterface anim ("udp-client2client-anim.xml");

    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    uint32_t centerX = 30, centerY = 30, x = 0, y = 0;
    for(uint32_t i=0; i<numNodes; ++i){
        x = centerX + 10*i;
        y = centerY + 10*i;
        anim.SetConstantPosition(nodes.Get(i), x, y); 
    }

    // --- EXECUCAO --- //
    NS_LOG_INFO("Run simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    return 0;
}