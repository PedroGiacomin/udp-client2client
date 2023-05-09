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
    uint16_t numNodes = 3;
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
    // Cada client tem numNodes-1 aplicacoes ping, cada uma com um endereco de destino (so nao tem a do proprio endereco)
    // Cada aplicacao eh uma posicao do vetor pingApps
    std::vector<ApplicationContainer> pingApps(numNodes);
    for(uint16_t i = 0; i < numNodes; ++i){
        for(uint16_t j = 0; j < numNodes; ++j){
            if(j != i){
                V4PingHelper pingHelperAux(interface.GetAddress(j));
                pingHelperAux.SetAttribute ("Verbose", BooleanValue (false));
                pingHelperAux.SetAttribute ("Interval", TimeValue (Seconds(numNodes)));
                pingHelperAux.SetAttribute ("Size", UintegerValue (16));
                pingApps[i].Add(pingHelperAux.Install(nodes.Get(i))); // Instala ping(dest: j) no node i
            }
        }
    }

    // Os nodes comecam a pingar com 1 segundo de diferenca
    for(uint16_t i = 0; i < numNodes; ++i){
        uint16_t appStart = 1.0 + 1.0*i;
        pingApps[i].Start(Seconds(appStart));
        pingApps[i].Stop(Seconds(appStart + numNodes * 3 + 1.0)); //Termina apos enviar 3 pacotes
    }
    
    // TRACING
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("tracing/udp-client2client/udp-client2client.tr"));
    csma.EnablePcapAll ("tracing/udp-client2client/udp-client2client.tr", false);

    // --- NETANIM --- //
    NS_LOG_INFO("Set animation.");
    AnimationInterface anim ("anim/udp-client2client-anim.xml");

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